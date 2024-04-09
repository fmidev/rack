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

//using namespace drain::image;
template <>
const drain::FlaggerDict drain::EnumDict<rack::EchoTop2Op::Reliability>::dict = {
		{"UNDEFINED",     rack::EchoTop2Op::Reliability::UNDEFINED},
		{"UNDERSHOOTING", rack::EchoTop2Op::Reliability::UNDERSHOOTING},
		{"OVERSHOOTING",  rack::EchoTop2Op::Reliability::OVERSHOOTING},
		{"INTERPOLATION", rack::EchoTop2Op::Reliability::INTERPOLATION},
};

namespace rack
{

EchoTop2Op::EchoTop2Op(double threshold) :
		PolarProductOp(__FUNCTION__,"Estimates maximum altitude of given reflectivity"),
		threshold(threshold), weights(1.0, 0.8, 0.6, 0.4, 0.2) {

	parameters.link("threshold", this->threshold, "reflectivity limit (dB)");
	parameters.link("reference", this->reference.tuple(), "'dry point' of low reflectivity and high altitude [dBZ:m]");
	parameters.link("dryTop",    this->dryTopDBZ, "reflectivity replacing 'undetect' [dBZ]"); // if set, reference will be applied also here
#ifndef NDEBUG
	parameters.link("weights", this->weights.tuple(), "weights for INTERPOLATION, OVERSHOOTING, UNDERSHOOTING, WEAK, NOECHO");
	parameters.link("EXTENDED", this->EXTENDED, "append classified data");

	// parameters.link("_EXTENDED", this->EXTENDED, "append classified data");
#endif

	dataSelector.setQuantities("^DBZH$");

	odim.product = "ETOP";
	odim.quantity = "HGHT";
	odim.type = "S";
	odim.scaling.scale = 0.0;

	/*
	 The original idea was to "encode" weights to class indices 0...255. But that affects their order in legends.
	const EncodingODIM & odimQ = getQuantityMap().get("QIND")['C'];
	const drain::image::Palette & cp = PaletteManager::getPalette("CLASS-ETOP");
	weights.interpolation = odimQ.scaleForward(cp.getEntryByCode("interpolated").first);
	weights.overShooting  = odimQ.scaleForward(cp.getEntryByCode("interpolated.undetect").first);
	weights.underShooting = odimQ.scaleForward(cp.getEntryByCode("extrapolated.dry").first);
	weights.weak          = odimQ.scaleForward(cp.getEntryByCode("weak").first);
	weights.clear         = weights.overShooting; // slightly lower than weights.interpolation
	*/

	//drain::Logger(__FILE__, __LINE__, __FUNCTION__).attention("weights: ", weights);
	// drain::Logger(__FILE__, __LINE__, __FUNCTION__).attention("this: ", this);

};



//void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const;
void EchoTop2Op::computeSingleProduct(const DataSetMap<src_t> & srcSweeps, DataSet<dst_t> & dstProduct) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.warn("Starting...");
	mout.special<LOG_NOTICE>(*this);

	const bool USE_KILOMETRES = ODIM::versionFlagger.isSet(ODIM::KILOMETRES);

	if (USE_KILOMETRES){
		mout.advice("Use --odim <version> to change. See --help odim");
		mout.attention("ODIM version ", ODIM::versionFlagger, ", using kilometres (not metres).");
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

	mout.attention("ODIM scaling: ", dstEchoTop.odim.scaling);


	// applyODIM(dstEchoTop.odim, odim);
	dstEchoTop.setEncoding(odim.type, targetEncoding);

	if ((dstEchoTop.odim.scaling.scale == 0) || (dstEchoTop.odim.scaling.scale == 1.0)){ // ~ unset
		//odim.scaling.setPhysicalRange(0.0, 25.6);
		dstEchoTop.odim.setRange(0.0, 20000.0 * odimVersionMetricCoeff);
		// dstEchoTop.odim.scaling.scale *= odimVersionMetricCoeff;
		mout.attention("ODIM scaling: ", dstEchoTop.odim.scaling);
	}


	dstEchoTop.setGeometry(dstEchoTop.odim);

	mout.attention("ODIM scaling: ", dstEchoTop.odim.scaling);

	mout.special(dstEchoTop.odim);

	// clumsy
	const drain::image::AreaGeometry & area = dstEchoTop.odim.getGeometry();


	// setGeometry(dstOdim, dstEchoTop);
	// setEncoding(dstOdim, dstEchoTop);
	// dstEchoTop.setGeometry(area);

	typedef drain::typeLimiter<double> Limiter;
	Limiter::value_t limit = drain::Type::call<Limiter>(dstEchoTop.data.getType());
	mout.attention("limiter:", limit);

	PlainData<dst_t> & dstQuality = dstProduct.getQualityData(); // dstEchoTop.getQualityData();
	getQuantityMap().setQuantityDefaults(dstQuality, "QIND", "C");
	dstQuality.setGeometry(area);
	MethodWeights<short int> WEIGHTS; // "assumes"
	WEIGHTS.interpolation = dstQuality.odim.scaleInverse(weights.interpolation);
	WEIGHTS.overShooting  = dstQuality.odim.scaleInverse(weights.overShooting);
	WEIGHTS.underShooting = dstQuality.odim.scaleInverse(weights.underShooting);
	WEIGHTS.weak          = dstQuality.odim.scaleInverse(weights.weak);
	WEIGHTS.clear         = dstQuality.odim.scaleInverse(weights.clear);


	mout.attention("WEIGHTS: ", WEIGHTS);

	MethodWeights<unsigned char> CLASS;

#ifndef NDEBUG

	PlainData<dst_t> & dstClass = dstEchoTop.getQualityData("CLASS-ETOP");
	getQuantityMap().setQuantityDefaults(dstClass, "CLASS", "C");

	PlainData<dst_t> & dstSlope = dstProduct.getData("DBZ-SLOPE");
	// Z(dB) decay per metre (or kilometre until ODIM 2.3?)
	dstSlope.setEncoding(typeid(unsigned short));
	//dstSlope.setPhysicalRange(-0.01, +0.01); // m/dBZ
	dstSlope.setPhysicalRange(-10, +10); // dBZ/km
	Limiter::value_t limitSlope = drain::Type::call<Limiter>(dstSlope.data.getType());

	if (EXTENDED){
		//dstEchoTop.setGeometry(dstEchoTop.odim);
		dstClass.setGeometry(dstEchoTop.odim);
		//drain::Variable & classLegend = dstClass.getWhat()["legendTEST"];
		//classLegend = "0:Clear,128:OverShooting"; // (TODO: redesign "direct" legend struct, as properties?)
		const drain::image::Palette & cp = PaletteManager::getPalette("CLASS-ETOP");
		CLASS.clear = dstClass.odim.undetect;
		CLASS.interpolation = cp.getEntryByCode("interpolated").first;
		CLASS.overShooting  = cp.getEntryByCode("interpolated.undetect").first; //  cp.getEntryByCode("overshooting").first;
		CLASS.underShooting = cp.getEntryByCode("extrapolated.dry").first; // cp.getEntryByCode("undershooting").first;
		CLASS.weak  = cp.getEntryByCode("weak").first;
		CLASS.error = cp.getEntryByCode("error").first; // could be nodata as well


		dstSlope.setGeometry(dstEchoTop.odim);
		dstSlope.getHow()["unit"] = "m/dBZ";
		//dstSlope.getHow()["unit"] = "m/dBZ";
	}
	else {
		dstClass.setExcluded();
		dstSlope.setExcluded();
	}

#endif



	double groundDistance;

	struct rangeBinInfo {

		/// Pointer to data array (DBZH)
		const Data<src_t> * dataPtr = nullptr;

		/// Index of the bin at the ground distance D of all the elevation beams considered
		size_t binIndex = 0;

		/// Height from the ground surface (from radar site, ASL)
		double height = 0;

		/// Reflectivity observed at the current bin. (Needed?)
		double dBZ = NAN;
	};

	// TODO: test overlapping (repeated) elagles, warn and suggest selector (PRF for example)

	std::map<double,rangeBinInfo> dbzData; // D bit slow, but for now

	/// Pointer to the measurement
	/**
	 *  - lower than the given threshold...
	 *  - ...but minimally lower
	 *  - above a measurement reaching the threshold
	 *  - the highest possible
	 */
	rangeBinInfo *outerInfo;

	/// Pointer to the measurement
	/**
	 *  - higher than the given threshold
	 *  - ...but minimally higher
	 *  - below the last measurement reaching the threshold
	 *  - the highest possible
	 */
	rangeBinInfo *innerInfo;

	double azm;
	double height;
	double slope; // EXTENDED
	int j2;

	size_t i2;

	const bool REPLACE_UNDETECT = !::isnan(dryTopDBZ);

	mout.attention("Replace undetect:", REPLACE_UNDETECT, " dryTop value=", dryTopDBZ);

	// Direct array index of a pixel (i,j)
	size_t address;

	// MAIN
	for (size_t i = 0; i < area.width; ++i){

		groundDistance = dstEchoTop.odim.getBinDistance(i); // These bins are on the ground...
		mout.debug("bin #", i, ", distance=", groundDistance);

		// Initialize: collect sweeps which are inside range.
		dbzData.clear();
		for (const auto & entry: srcSweeps){
			const Data<src_t> & src =  entry.second.getData("DBZH");
			int index = src.odim.getBinIndex(groundDistance);
			if (index >= 0){
				i2 = index;
				//if ((i & 15) == 0){
				if (i2 < src.odim.area.width){
					rangeBinInfo & info = dbzData[src.odim.elangle];
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

		/// Actual azimuthal loop: [0,360[ degrees
		for (size_t j = 0; j < area.height; ++j){

			azm = dstEchoTop.odim.getAzimuth(j);

			innerInfo = nullptr;
			outerInfo = nullptr;

			// Start column loop
			for (auto & entry: dbzData){

				rangeBinInfo & info = entry.second;
				const Data<src_t> & srcData = *info.dataPtr;

				j2 = srcData.odim.getRayIndex(azm);

				/*
				if (i==50){
					mout.reject<LOG_NOTICE>(j, " -> ", j2 , ", AZM=", azm);
				}
				*/

				// Raw byte value
				info.dBZ = srcData.data.get<double>(info.binIndex, j2);

				if (!srcData.odim.isValue(info.dBZ)){

					if ((innerInfo != nullptr) && (outerInfo == nullptr)){
						// Store this empty measurent, as it is above a high-dbz measurement
						outerInfo = &info;
						outerInfo->dBZ = dbzOdim.undetect;
					}

					// dstEchoTop.data.put(i,j, ::rand() & 0xffff); // DEBUG
					continue;
				}

				// Scale to physical-scale Z
				info.dBZ = srcData.odim.scaleForward(info.dBZ);

				if (info.dBZ < threshold){
					// Ensure start of a (new) precipitation column
					if (outerInfo == nullptr){
						outerInfo = &info;
					}
					else if (info.dBZ > outerInfo->dBZ) {
						// Adjust if a measurement even closer to threshold was found
						outerInfo = &info;
					}
				}
				else { // info.dBZ >= threshold
					// Adjust if a measurement closer to threshold was found
					innerInfo = &info;
					outerInfo = nullptr; // reset
				}

			} // End column loop


			/// Main: compute the echo top altitude.

			address = dstEchoTop.data.address(i, j);

			if (outerInfo != nullptr){

				if (innerInfo != nullptr){

					if (dbzOdim.isValue(outerInfo->dBZ)){

						// INTERPOLATE, as both values are available
						if (outerInfo->dBZ < innerInfo->dBZ){
							slope  = getSlope(outerInfo->height, innerInfo->height, outerInfo->dBZ, innerInfo->dBZ);
							//slope  = (outerInfo->altitude - innerInfo->altitude) / (outerInfo->dBZ - innerInfo->dBZ);
							height = innerInfo->height + slope*(threshold - innerInfo->dBZ);

							dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height)));
							dstQuality.data.put<int>(address, WEIGHTS.interpolation); // TODO: quality index
#ifndef NDEBUG
							if (EXTENDED){
								dstClass.data.put<int>(address, CLASS.interpolation);
								dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(1000.0/ slope))); // ALERT div by 0 RISK
							}
#endif

						}
						else {
							// the highest bin has higher Z than the lower bin? This should not happen.
							dstEchoTop.data.put(address, dstEchoTop.odim.nodata);
							dstQuality.data.put<int>(address, WEIGHTS.error);
#ifndef NDEBUG
							if (EXTENDED){
								dstClass.data.put<int>(address, CLASS.error); //
							}
#endif
						}
					}
					else {
						// OVERSHOOTING = "DRY TOP" - interpolate between strong and "dry" point.
						if (REPLACE_UNDETECT){
							slope  = getSlope(outerInfo->height, innerInfo->height, dryTopDBZ, innerInfo->dBZ);
						}
						else {
							slope  = getSlope(reference.height, innerInfo->height, reference.reflectivity, innerInfo->dBZ);
						}
						height = innerInfo->height + slope*(threshold - innerInfo->dBZ);
						dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height)));
						// dstEchoTop.data.put(address, dstEchoTop.odim.nodata);
						dstQuality.data.put(address, WEIGHTS.overShooting);
#ifndef NDEBUG
						if (EXTENDED){
							dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(1000.0/ slope))); // dBZ/km ALERT div by 0 RISK
							dstClass.data.put<int>(address, CLASS.overShooting);
						}
#endif
					}
				}
				else if (dbzOdim.isValue(outerInfo->dBZ)){
					// Weak
					dstEchoTop.data.put(address, 2.0*odimVersionMetricCoeff);
					dstQuality.data.put(address, WEIGHTS.weak);
#ifndef NDEBUG
					if (EXTENDED){
						dstClass.data.put<int>(address, CLASS.weak); //192
					}
#endif
				}
				else {
					dstEchoTop.data.put(address, dstEchoTop.odim.undetect);
					dstQuality.data.put(address, WEIGHTS.clear); // CONFIRMED CLEAR?
#ifndef NDEBUG
					if (EXTENDED){
						dstClass.data.put<int>(address, CLASS.clear);
					}
#endif
				}

			}
			else {

				if (innerInfo != nullptr){
					// UNDERSHOOTING = the highest bin has Z exceeding the threshold
					slope  = getSlope(reference.height, innerInfo->height, reference.reflectivity, innerInfo->dBZ);
					// slope  = (reference.height - innerInfo->altitude)/(reference.reflectivity - innerInfo->dBZ);
					height = innerInfo->height + slope*(threshold - innerInfo->dBZ);
					dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height))); // strong
					dstQuality.data.put(address, WEIGHTS.underShooting);
#ifndef NDEBUG
					if (EXTENDED){
						dstClass.data.put<int>(address, CLASS.underShooting);
						dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(1000.0 / slope))); // ALERT div by 0 RISK
					}
#endif
				}
				else {
					// No echo at all was found in this column above ground bin (i,j).
					dstEchoTop.data.put(address, dstEchoTop.odim.undetect);
					dstQuality.data.put(address, WEIGHTS.clear);
#ifndef NDEBUG
					if (EXTENDED){
						dstClass.data.put<int>(address, CLASS.clear);
						//dstSlope.data.put(address, limitSlope(dstSlope.odim.undetect));
						dstSlope.data.put(address, dstSlope.odim.undetect);
					}
#endif
				}


				// Todo: use far point
				// dstEchoTop.data.put(address, dstEchoTop.odim.scaleInverse(upperInfo->altitude) ); // weak
			}


			//dbz = dstEchoTop.data.get<double>(i,j);
			/*
			for (const auto & entry: srcSweeps){
				dst = std::max(dbz, entry.second.getData("DBZH").data.get<double>(i,j));
			}
			*/
		}

	}

#ifndef NDEBUG
	{
		PolarSlidingAvgOp smoother;

	}


#endif

	PolarSlidingAvgOp smoother;
	/*
	for (const auto & entry: srcSweeps){
		const Data<src_t> & srcData = entry.second.getData("DBZH");
		smoother.processData(srcData, dstGradient);  // not in-place..
		return;
	}
	*/

	mout.accept<LOG_NOTICE>(" threshold=", threshold);

	dstProduct.getWhere()["rscale"] = dstEchoTop.odim.rscale;
	dstProduct.getWhere()["rscale2"] = dstEchoTop.odim.rscale+10;
	dstProduct.getWhat()["comment"] = "EchoTop2-test";
	//dstProduct.updateTree3(dstEchoTop.odim);

	mout.special<LOG_NOTICE>("First Data: ", dstProduct.getFirstData());


}


}


// Rack
