/*

MIT License

Copyright (c) 2017 FMI Open Development / Markus Peura, first.last@fmi.fi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
*/
#include "EchoTop2Op.h"
#include "PolarSlidingWindowOp.h"

#include "main/palette-manager.h"

#include "radar/Geometry.h"
#include "radar/PolarSmoother.h"

/* OLD STYLE: CLASS / 255.0 = QIND
template <>
const drain::FlagResolver::dict_t drain::EnumDict<rack::EchoTop2Op::Reliability>::dict = {
		{"UNDEFINED",     rack::EchoTop2Op::Reliability::UNDEFINED},
		{"UNDERSHOOTING", rack::EchoTop2Op::Reliability::UNDERSHOOTING},
		{"OVERSHOOTING",  rack::EchoTop2Op::Reliability::OVERSHOOTING},
		{"INTERPOLATION", rack::EchoTop2Op::Reliability::INTERPOLATION},
};
*/

namespace rack
{

EchoTop2Op::EchoTop2Op(double threshold) :
		PolarProductOp(__FUNCTION__,"Estimates maximum altitude of given reflectivity"),
		threshold(threshold), weights(1.0, 0.8, 0.6, 0.4, 0.2) {

	parameters.link("threshold", this->threshold, "reflectivity limit (dB)");
	parameters.link("reference", this->reference.tuple(), "'dry point' of low reflectivity and high altitude [dBZ:m]");
	parameters.link("undetectValue",    this->undetectReflectivity, "reflectivity replacing 'undetect' [dBZ]"); // if set, reference will be applied also here
#ifndef NDEBUG
	parameters.link("weights", this->weights.tuple(), "weights for INTERPOLATION, INTERP_UNDET, EXTRAP_UP, EXTRAP_DOWN, CLEAR");
	parameters.link("EXTENDED", this->EXTENDED_OUTPUT, "store also DBZ_SLOPE and CLASS");
	// parameters.link("_EXTENDED", this->EXTENDED, "append classified data");
#endif

	dataSelector.setQuantities("^DBZH$");

	this->undetectReflectivity = getQuantityMap().get("DBZH").undetectValue;

	odim.product = "ETOP";
	odim.quantity = "HGHT"; // METRES
	odim.type = "S";
	odim.scaling.scale = 0.0;

	/*
	 The original idea was to "encode" weights to class indices 0...255. But that affects their order in legends.
	const EncodingODIM & odimQ = getQuantityMap().get("QIND")['C'];
	const drain::image::Palette & cp = PaletteManager::getPalette("CLASS-ETOP");
	weights.interpolation = odimQ.scaleForward(cp.getEntryByCode("interpolated").first);
	weights.interpolation_dry  = odimQ.scaleForward(cp.getEntryByCode("interpolated.undetect").first);
	weights.truncated = odimQ.scaleForward(cp.getEntryByCode("extrapolated.dry").first);
	weights.weak          = odimQ.scaleForward(cp.getEntryByCode("weak").first);
	weights.clear         = weights.interpolation_dry; // slightly lower than weights.interpolation
	*/

	// drain::Logger(__FILE__, __LINE__, __FUNCTION__).attention("weights: ", weights);
	// drain::Logger(__FILE__, __LINE__, __FUNCTION__).attention("this: ", this);

};



//void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const;
void EchoTop2Op::computeSingleProduct(const DataSetMap<src_t> & srcSweeps, DataSet<dst_t> & dstProduct) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.warn("Starting...");
	mout.special<LOG_NOTICE>(*this);

	const bool USE_KILOMETRES = ODIM::versionFlagger.isSet(ODIM::KILOMETRES);

	if (USE_KILOMETRES){
		mout.attention("ODIM version ", ODIM::versionFlagger, ", using kilometres (not metres).");
		mout.advice("Use --odim <version> to change. See --help odim");
	}
	else {
		mout.info("ODIM version ", ODIM::versionFlagger, ", using metres (not kilometres).");
	}

	const double odimVersionMetricCoeff = USE_KILOMETRES ? 0.001 : 1.0;

	/// Shared, because input data MAY contain different encoding(s)
	const EncodingODIM & dbzOdim = getQuantityMap().get("DBZH").get('S');
	//dstOdim.updateFromCastableMap(dbzOdim);

	// The actual result
	Data<dst_t> & dstEchoTop = dstProduct.getData(odim.quantity);
	dstEchoTop.odim.updateFromCastableMap(odim);
	deriveDstGeometry(srcSweeps, dstEchoTop.odim);
	determineMetadata(srcSweeps, dstEchoTop.odim);

	mout.debug("ODIM scaling 1: ", dstEchoTop.odim.scaling);


	// applyODIM(dstEchoTop.odim, odim);
	dstEchoTop.setEncoding(odim.type, targetEncoding);

	if ((dstEchoTop.odim.scaling.scale == 0) || (dstEchoTop.odim.scaling.scale == 1.0)){ // ~ unset
		//odim.scaling.setPhysicalRange(0.0, 25.6);
		dstEchoTop.odim.setRange(0.0, 20000.0 * odimVersionMetricCoeff);
		// dstEchoTop.odim.scaling.scale *= odimVersionMetricCoeff;
		mout.attention("ODIM scaling 2: ", dstEchoTop.odim.scaling);
	}

	dstEchoTop.setGeometry(dstEchoTop.odim);

	mout.attention("ODIM scaling 3: ", dstEchoTop.odim.scaling);

	mout.special(dstEchoTop.odim);

	// clumsy
	const drain::image::AreaGeometry & area = dstEchoTop.odim.getGeometry();


	// setGeometry(dstOdim, dstEchoTop);
	// setEncoding(dstOdim, dstEchoTop);
	// dstEchoTop.setGeometry(area);

	const std::type_info & type = dstEchoTop.data.getType();
	typedef drain::typeLimiter<double> Limiter;
	Limiter::value_t limit = drain::Type::call<Limiter>(type);
	//dstEchoTop.data.getScaling().g

	/*
	double d1 = drain::Type::call<drain::typeMin, double>(type);
	double d2 = drain::Type::call<drain::typeMax, double>(type);
	for (int i = 0; i < 100; ++i) {
		double d = i * (d2-d1)/100.0 + d1;
		double m = dstEchoTop.odim.scaleForward(d);
		mout.note("limiter:", i, '\t', d, '\t', m, '\t', dstEchoTop.odim.scaleInverse(m), '\t', limit(dstEchoTop.odim.scaleInverse(m)));
	}
	mout.special("limiter:", limit);
	*/


	PlainData<dst_t> & dstQuality = dstProduct.getQualityData();
	getQuantityMap().setQuantityDefaults(dstQuality, "QIND", "C");
	dstQuality.setGeometry(area);

	/// Quality (confidence) (using storage type encoding)
	MethodWeights<short int> WEIGHTS; // "assumes"
	WEIGHTS.interpolation = dstQuality.odim.scaleInverse(weights.interpolation);
	WEIGHTS.interpolation_dry  = dstQuality.odim.scaleInverse(weights.interpolation_dry);
	WEIGHTS.extrapolation_up   = dstQuality.odim.scaleInverse(weights.extrapolation_up);
	WEIGHTS.extrapolation_down = dstQuality.odim.scaleInverse(weights.extrapolation_down);
	WEIGHTS.clear         = dstQuality.odim.scaleInverse(weights.clear);


	mout.debug("WEIGHTS: ", WEIGHTS);

	/// Class codes (using storage type encoding)
	MethodWeights<unsigned char> CLASS;

// #ifndef NDEBUG

	PlainData<dst_t> & dstClass = dstEchoTop.getQualityData("CLASS-ETOP");
	getQuantityMap().setQuantityDefaults(dstClass, "CLASS", "C");

	PlainData<dst_t> & dstSlope = dstProduct.getData("DBZ-SLOPE");
	// Z(dB) decay per metre (or kilometre until ODIM 2.3?)
	dstSlope.setEncoding(typeid(unsigned short));
	//dstSlope.setPhysicalRange(-0.01, +0.01); // m/dBZ
	dstSlope.setPhysicalRange(-10, +10); // dBZ/km
	Limiter::value_t limitSlope = drain::Type::call<Limiter>(dstSlope.data.getType());

	if (EXTENDED_OUTPUT){
		//dstEchoTop.setGeometry(dstEchoTop.odim);
		dstClass.setGeometry(dstEchoTop.odim);
		//drain::Variable & classLegend = dstClass.getWhat()["legendTEST"];
		//classLegend = "0:Clear,128:OverShooting"; // (TODO: redesign "direct" legend struct, as properties?)
		const drain::image::Palette & cp = PaletteManager::getPalette("CLASS-ETOP");
		CLASS.clear = dstClass.odim.undetect;
		CLASS.interpolation = cp.getEntryByCode("interpolated").first;
		CLASS.interpolation_dry  = cp.getEntryByCode("interpolated.undetect").first; //  cp.getEntryByCode("overshooting").first;
		CLASS.extrapolation_up = cp.getEntryByCode("strong.extrapolated").first; // cp.getEntryByCode("undershooting").first;
		CLASS.extrapolation_down  = cp.getEntryByCode("weak").first;
		CLASS.error = cp.getEntryByCode("error").first; // could be nodata as well


		dstSlope.setGeometry(dstEchoTop.odim);
		dstSlope.getHow()["unit"] = "m/dBZ";
		//dstSlope.getHow()["unit"] = "m/dBZ";
	}
	else {
		dstClass.setExcluded();
		dstSlope.setExcluded();
	}

// #endif

	// MAIN -----------------------------------------------------------------

	double groundDistance;


	struct MeasurementHolder : public Measurement {

		/// Pointer to data array (DBZH)
		const Data<src_t> * dataPtr = nullptr;

		/// Index of the bin at the ground distance D of all the elevation beams considered
		size_t binIndex = 0;

		/*
		/// Height from the ground surface (from radar site, ASL)
		double height = 0;

		/// Reflectivity observed at the current bin.
		double reflectivity = NAN;
		*/

		/// Reliability/confidence of the measurement value, maximal (1.0) when reflectivity is the threshold value.
		// double quality = 0.0;
	};


	// TODO: test overlapping (repeated) elangles, warn and suggest selector (PRF for example)


	/// Pointer to the measurement
	/**
	 *  - lower than the given threshold...
	 *  - ...but minimally lower
	 *  - above a measurement reaching the threshold
	 *  - the highest possible
	 */
	MeasurementHolder *weakMsrm;

	/// Pointer to the measurement
	/**
	 *  - higher than the given threshold
	 *  - ...but minimally higher
	 *  - below the last measurement reaching the threshold
	 *  - the highest possible
	 */
	MeasurementHolder *strongMsrm;

	std::map<double,MeasurementHolder> dbzData; // D bit slow, but for now

	double azm;
	double height;
	double slope; // EXTENDED
	int j2;

	size_t i2;

	// const bool REPLACE_UNDETECT = !::isnan(undetectReflectivity);
	// mout.attention("Replace undetect:", REPLACE_UNDETECT, " dryTop value=", undetectReflectivity, drain::TextStyle::Colour::GREEN, "test", drain::TextStyle::Colour::NO_COLOR);

	const bool USE_INTERPOLATION     = (weights.interpolation     > 0.0);
	const bool USE_INTERPOLATION_DRY = (weights.interpolation_dry > 0.0) && !::isnan(undetectReflectivity);


	// Direct array index of a pixel (i,j)
	size_t address;

	// MAIN
	for (size_t i = 0; i < area.width; ++i){

		groundDistance = dstEchoTop.odim.getBinDistance(i); // These bins are on the ground...
		mout.debug("bin #", i, ", distance=", groundDistance);

		// Initialize: collect sweeps which are inside range.
		dbzData.clear();
		for (const auto & sweep: srcSweeps){
			const Data<src_t> & src =  sweep.second.getData("DBZH");
			int index = src.odim.getBinIndex(groundDistance);
			if (index >= 0){
				i2 = index;
				//if ((i & 15) == 0){
				if (i2 < src.odim.area.width){
					MeasurementHolder & info = dbzData[src.odim.elangle];
					info.dataPtr  = &src;
					info.binIndex = index;
					info.height = Geometry::heightFromEtaGround(src.odim.getElangleR(), groundDistance);
					/*
					if ((i & 15) == 0){
						mout.accept<LOG_DEBUG>(entry.first, " elangle=",src.odim.elangle, " altitude=", info.altitude);
					}
					*/
				}
				else {
					// mout.reject<LOG_NOTICE>(entry.first, " elangle=",src.odim.elangle);
				}
			}
		}

		/// Azimuthal loop: [0,360[ degrees
		for (size_t j = 0; j < area.height; ++j){

			azm = dstEchoTop.odim.getAzimuth(j);

			strongMsrm = nullptr;
			weakMsrm = nullptr;

			// Column loop:
			for (auto & entry: dbzData){

				MeasurementHolder & msr = entry.second;
				const Data<src_t> & srcData = *msr.dataPtr;

				j2 = srcData.odim.getRayIndex(azm);

				/*
				if (i==50){
					mout.reject<LOG_NOTICE>(j, " -> ", j2 , ", AZM=", azm);
				}
				*/

				// Raw byte value
				msr.reflectivity = srcData.data.get<double>(msr.binIndex, j2);

				if (msr.reflectivity == srcData.odim.nodata){
					// Treat as missing data indeed (like missing whole beam) - skip it
					// (This state is possible, if data has been quality controlled or radar range is smaller than image width, and image margin is filled nodata values.)
					continue;
				}

				if (msr.reflectivity == srcData.odim.undetect){ // Measured, but no echo. Treat as dry air.

					if ((strongMsrm != nullptr) && (weakMsrm == nullptr)){
						// Strong echo has been detected below, so store this measurement.
						weakMsrm = &msr;
						/* If applied, this drops "dry-top" class
						if (REPLACE_UNDETECT){
							weakMsrm->reflectivity = undetectReflectivity;
						}
						*/
						// weakMsrm->reflectivity = dbzOdim.undetect; // No effect
					}

					// dstEchoTop.data.put(i,j, ::rand() & 0xffff); // DEBUG
					continue;
				}

				// Ok, info.dBZ is a valid echo (usually precip).
				// Convert it from storage-type scaling to physical value.
				msr.reflectivity = srcData.odim.scaleForward(msr.reflectivity);

				if (msr.reflectivity >= threshold) {

					// A higher sample exceeding the threshold was found. = The highest this far.
					strongMsrm = & msr;
					weakMsrm = nullptr; // reset, "drop" the weaker (and look for a new one, from above bins).

				}
				else {

					/*
					if ((!USE_INTERPOLATION) && (strongMsrm!=nullptr)){
						// KEEP strong, it is prioritized
					}
					*/
					if (true) { //
						// Ensure start of a (new) precipitation column
						if (weakMsrm == nullptr){
							weakMsrm = & msr;
						}
						else if (msr.reflectivity > weakMsrm->reflectivity) {
							// The new measurement is closer to threshold than the current weak one.
							// -> Update.
							// Consider: is this needed? lower measurement was anyway CLOSER (in height).
							weakMsrm = & msr;
						}
					}

				}

			} // End column loop


			/// Main: compute the echo top altitude.

			address = dstEchoTop.data.address(i, j);

			if (weakMsrm != nullptr){

				if (strongMsrm != nullptr){

					if (dbzOdim.isValue(weakMsrm->reflectivity)){

						// INTERPOLATION, as both values are available
						if (weakMsrm->reflectivity < strongMsrm->reflectivity){

							if (USE_INTERPOLATION){
								slope  = getSlope(*weakMsrm, *strongMsrm);
							}
							else {
								slope = getSlope(reference, *strongMsrm);
							}
							height = strongMsrm->height + slope*(threshold - strongMsrm->reflectivity);

							dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height)));
							dstQuality.data.put<int>(address, USE_INTERPOLATION ? WEIGHTS.interpolation : WEIGHTS.extrapolation_up);
//#ifndef NDEBUG
							if (EXTENDED_OUTPUT){
								dstClass.data.put<int>(address, USE_INTERPOLATION ? CLASS.interpolation : CLASS.extrapolation_up);
								dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(1000.0/ slope))); // ALERT div by 0 RISK
							}
// #endif

						}
						else {
							// This should not happen!
							// Ie. the highest bin has higher Z than the lower bin?
							// But the lower bin should has been reseted.
							mout.error("Illegal state in measurement conf: Z+ > Z-  for h+ > h-");
							/*
							dstEchoTop.data.put(address, dstEchoTop.odim.nodata);
							dstQuality.data.put<int>(address, WEIGHTS.error);
// #ifndef NDEBUG
							if (EXTENDED_OUTPUT){
								dstClass.data.put<int>(address, CLASS.error); //
							}
// #endif
							*/
						}
					}
					else {
						// INTERPOLATION_DRY - interpolate between strong and "dry" point.
						// overshooting, "DRY TOP"
						if (USE_INTERPOLATION_DRY){ // && !REFERENCE_ONLY
							weakMsrm->reflectivity = undetectReflectivity;
							slope = getSlope(*weakMsrm, *strongMsrm);
							// slope = getSlope(weakMsrm->height, strongMsrm->height, undetectReflectivity, strongMsrm->reflectivity);
						}
						else {
							slope = getSlope(reference, *strongMsrm);
							// slope = getSlope(reference.height, strongMsrm->height, reference.reflectivity, strongMsrm->reflectivity);
						}
						height = strongMsrm->height + slope*(threshold - strongMsrm->reflectivity);
						dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height)));
						dstQuality.data.put(address, USE_INTERPOLATION_DRY ? WEIGHTS.interpolation_dry : WEIGHTS.extrapolation_up);
// #ifndef NDEBUG
						if (EXTENDED_OUTPUT){
							dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(1000.0/ slope))); // dBZ/km ALERT div by 0 RISK
							dstClass.data.put<int>(address, USE_INTERPOLATION_DRY ? CLASS.interpolation_dry : CLASS.extrapolation_up);
						}
// #endif
					}
				}
				// No strong measurement.
				else if (dbzOdim.isValue(weakMsrm->reflectivity)){
					// Weak measurement only.
					slope  = getSlope(reference, *weakMsrm);
					height = weakMsrm->height + slope*(threshold - weakMsrm->reflectivity);
					dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height))); // ::rand()
					dstQuality.data.put(address, WEIGHTS.extrapolation_down);
// #ifndef NDEBUG
					if (EXTENDED_OUTPUT){
						dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(1000.0/ slope)));
						dstClass.data.put<int>(address, CLASS.extrapolation_down); //192
					}
// #endif
				}
				else { // No strong or weak measurement
					dstEchoTop.data.put(address, dstEchoTop.odim.undetect);
					dstQuality.data.put(address, WEIGHTS.clear); // CONFIRMED CLEAR?
// #ifndef NDEBUG
					if (EXTENDED_OUTPUT){
						dstClass.data.put<int>(address, CLASS.clear);
					}
// #endif
				}

			}
			// no weakMeasurement found
			else {

				if (strongMsrm != nullptr){
					// TRUNCATED / UNDERSHOOTING = the highest bin has Z exceeding the threshold
					slope  = getSlope(reference, *strongMsrm);
					height = strongMsrm->height + slope*(threshold - strongMsrm->reflectivity);

					if (j == 90){
						// mout.note(i,',', j, '\t', height, '\t', dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height), '\t', limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height)));
					}
					// dstEchoTop.data.put(address, ::rand());
					dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height))); // strong
					dstQuality.data.put(address, WEIGHTS.extrapolation_up);
// #ifndef NDEBUG
					if (EXTENDED_OUTPUT){
						dstClass.data.put<int>(address, CLASS.extrapolation_up);
						dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(1000.0 / slope))); // ALERT div by 0 RISK
					}
// #endif
				}
				else {
					// No echo at all was found in this column above ground bin (i,j).
					dstEchoTop.data.put(address, dstEchoTop.odim.undetect);
					dstQuality.data.put(address, WEIGHTS.clear);
// #ifndef NDEBUG
					if (EXTENDED_OUTPUT){
						dstClass.data.put<int>(address, CLASS.clear);
						//dstSlope.data.put(address, limitSlope(dstSlope.odim.undetect));
						dstSlope.data.put(address, dstSlope.odim.undetect);
					}
// #endif
				}


			}

		}

	}

// #ifndef NDEBUG
	if (false){
		Data<dst_t> & dstEchoTopSmooth = dstProduct.getData(odim.quantity+"_SMOOTH");
		// qm.setQuantityDefaults(fuzzyCell2, "PROB");

		dstEchoTopSmooth.odim.importCastableMap(dstEchoTop.odim);
		// mout.special(dstEchoTopSmooth);
		dstEchoTopSmooth.copyEncoding(dstEchoTop);
		dstEchoTopSmooth.copyGeometry(dstEchoTop);

		mout.attention(dstEchoTopSmooth);

		PolarSlidingAvgOp smoother;
		/*
		smoother.conf.widthM  = 5*1000.0; // smoothAzm;
		smoother.conf.heightD = 100.0; // smoothRad;
		*/
		/*
		RadarWindowConfig test;
		smoother.conf.setPixelConf(test, dstEchoTop.odim);
		mout.warn("smoother.conf" , smoother.conf);
		mout.warn("    test conf" ,          test);
		*/

		//avgOp.setPixelConf()
		smoother.odim.importCastableMap(dstEchoTop.odim);
		smoother.odim.area.set(dstEchoTop.data.getGeometry());
		smoother.conf.updatePixelSize(dstEchoTop.odim);

		const PlainData<src_t> & pd = dstEchoTop;

		mout.attention("Next: processPlainData");
		mout.attention(pd);
		mout.attention(pd.odim);
		mout.attention(pd.odim.getGeometry());
		//mout.attention(dstEchoTop.odim.getGeometry());
		//mout.attention(dstEchoTop.odim);

		mout.attention(drain::TypeName<PlainData<dst_t> >::str());

		//smoother.processPlainData(dstEchoTop, dstEchoTopSmooth);
		smoother.processPlainData(pd, dstEchoTopSmooth);

		// Data<dst_t> & dstEchoTopSmooth = dstProduct.getData(odim.quantity+"_SMOOTH");
		// PolarSmoother::filter(dstEchoTop.odim, dstEchoTop.data, dstEchoTopSmooth.data, 100.0);
	}
// #endif

	/*
	for (const auto & entry: srcSweeps){
		const Data<src_t> & srcData = entry.second.getData("DBZH");
		smoother.processData(srcData, dstGradient);  // not in-place..
		return;
	}
	*/

	mout.accept<LOG_NOTICE>(" threshold=", threshold);

	dstProduct.getWhere()["rscale"] = dstEchoTop.odim.rscale;
	dstProduct.getWhere()["rscale10"] = dstEchoTop.odim.rscale+10;
	dstProduct.getWhat()["comment"] = "EchoTop2-test";
	//dstProduct.updateTree3(dstEchoTop.odim);

	mout.special<LOG_NOTICE>("First Data: ", dstProduct.getFirstData());
	Data<dst_t> & dst =  dstProduct.getFirstData();
	dstProduct.getHow()["from-qty"] = dst.odim.quantity;
	for (const auto & entry: srcSweeps){
		const Data<src_t> & src =  entry.second.getData("DBZH");
		if (!src.data.isEmpty()){
			dst.odim.angles.push_back(src.odim.elangle);
			// dstProduct.getHow()["angles"] << src.odim.elangle;
		}
	}

}


}


// Rack
