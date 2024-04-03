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

EchoTop2Op::EchoTop2Op(double threshold) : PolarProductOp(__FUNCTION__,"Estimates maximum altitude of given reflectivity"), threshold(threshold)
	{

	parameters.link("threshold", this->threshold, "reflectivity limit (dB)");
	parameters.link("reference", this->reference.tuple(), "'dry point' of low reflectivity and high altitude [dBZ:m]");
	parameters.link("dryTop",    this->dryTopDBZ, "reflectivity replacing 'undetect' [dBZ]"); // if set, reference will be applied also here
#ifndef NDEBUG
	parameters.link("_EXTENDED", this->EXTENDED, "append classified data");
	// parameters.link("_EXTENDED", this->EXTENDED, "append classified data");
#endif

	dataSelector.setQuantities("^DBZH$");

	odim.product = "ETOP";
	odim.quantity = "HGHT";
	odim.type = "S";

};



//void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const;
void EchoTop2Op::computeSingleProduct(const DataSetMap<src_t> & srcSweeps, DataSet<dst_t> & dstProduct) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.warn("Starting...");

	/// ODIM uses kilometres...s

	//static const double M2KM = 0.001;
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

	dstEchoTop.setEncoding(odim.type, targetEncoding);
	dstEchoTop.setGeometry(dstEchoTop.odim);

	mout.attention(dstEchoTop.odim);

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
	// setGeometry(dstOdim, dstEchoTop);

#ifndef NDEBUG

	PlainData<dst_t> & dstClass = dstEchoTop.getQualityData("CLASS");
	getQuantityMap().setQuantityDefaults(dstClass, "CLASS", "C");
	dstClass.setGeometry(area);

	PlainData<dst_t> & dstSlope = dstProduct.getData("DBZH_SLOPE");
	// Z(dB) decay per metre (kilometre ODIM 2.2)
	dstSlope.setEncoding(typeid(unsigned short));
	dstSlope.setPhysicalRange(-500, +500); // m/dBZ
	Limiter::value_t limitSlope = drain::Type::call<Limiter>(dstSlope.data.getType());

	if (EXTENDED){
		dstClass.setGeometry(area);
		dstSlope.setGeometry(area);
		dstSlope.getHow()["unit"] = "m/dBZ";
	}
	else {
		dstClass.setExcluded();
		dstSlope.setExcluded();
	}

#endif



	double groundDistance;

	struct rangeBinInfo {
		const Data<src_t> * dataPtr = nullptr;
		size_t binIndex = 0;
		double altitude = 0;
		double dBZ = NAN;
	};

	// TODO: test overlapping (repeated) elagles, warn and suggest selector (PRF for example)

	std::map<double,rangeBinInfo> dbzData; // a bit slow?, but for now...

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



	// Direct array index of a pixel (i,j)
	size_t address;


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
					info.altitude = Geometry::heightFromEtaGround(src.odim.getElangleR(), groundDistance);
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


			/// Finally, compute the echo top altitude.

			address = dstEchoTop.data.address(i, j);

			if (outerInfo != nullptr){

				if (innerInfo != nullptr){

					if (dbzOdim.isValue(outerInfo->dBZ)){
						// interpolate!
						if (outerInfo->dBZ < innerInfo->dBZ){
							slope  = (outerInfo->altitude - innerInfo->altitude) / (outerInfo->dBZ - innerInfo->dBZ);
							//height = innerInfo->altitude + (threshold - innerInfo->dBZ)/(outerInfo->dBZ - innerInfo->dBZ) * (outerInfo->altitude - innerInfo->altitude);
							height = innerInfo->altitude + slope*(threshold - innerInfo->dBZ);
							// height = dstEchoTop.odim.scaleInverse(height);
							// dstEchoTop.data.put(address, limit(height));
							dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(height)));
							dstQuality.data.put<int>(address, INTERPOLATION); // TODO: quality index
#ifndef NDEBUG
							if (EXTENDED){
								dstClass.data.put<int>(address, INTERPOLATION);
								dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(slope)));
							}
#endif

						}
						else {
							// the highest bin has higher Z than the lower bin? This should not happen.
							dstEchoTop.data.put(address, dstEchoTop.odim.nodata);
							dstQuality.data.put<int>(address, ERROR);
#ifndef NDEBUG
							if (EXTENDED){
								dstClass.data.put<int>(address, ERROR);
							}
#endif
						}
					}
					else {
						dstEchoTop.data.put(address, dstEchoTop.odim.nodata);
						dstQuality.data.put(address,(int)OVERSHOOTING);
#ifndef NDEBUG
						if (EXTENDED){
							dstClass.data.put<int>(address, OVERSHOOTING);
						}
#endif
					}
				}
				else if (dbzOdim.isValue(outerInfo->dBZ)){
					// Dry top
					dstEchoTop.data.put(address, 0);
					dstQuality.data.put(address, (int)WEAK);
#ifndef NDEBUG
					if (EXTENDED){
						dstClass.data.put<int>(address, WEAK);
					}
#endif
				}
				else {
					dstEchoTop.data.put(address, dstEchoTop.odim.undetect);
					dstQuality.data.put(address, (int)CLEAR); // CONFIRMED CLEAR?
#ifndef NDEBUG
					if (EXTENDED){
						dstClass.data.put<int>(address, CLEAR);
					}
#endif
				}

			}
			else {

				if (innerInfo == nullptr){
					// No echo at all was found in this column above ground bin (i,j).
					dstEchoTop.data.put(address, dstEchoTop.odim.undetect);
					dstQuality.data.put(address, (int)CLEAR);
#ifndef NDEBUG
					if (EXTENDED){
						dstClass.data.put<int>(address, CLEAR);
						dstSlope.data.put(address, limitSlope(dstSlope.odim.undetect));
					}
#endif
				}
				else {
					// UNDERSHOOTING = the highest bin has Z exceeding the threshold
					slope  = (reference.height - innerInfo->altitude)/(reference.reflectivity - innerInfo->dBZ);
					height = reference.height  + (threshold - innerInfo->dBZ) * slope;
					dstEchoTop.data.put(address, limit(dstEchoTop.odim.scaleInverse(odimVersionMetricCoeff * height))); // strong
					dstQuality.data.put(address, (int)UNDERSHOOTING);
#ifndef NDEBUG
					if (EXTENDED){
						dstClass.data.put<int>(address, UNDERSHOOTING);
						dstSlope.data.put(address, limitSlope(dstSlope.odim.scaleInverse(slope)));
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

	mout.accept<LOG_NOTICE>(*this, " threshold=", threshold);

	dstProduct.getWhere()["rscale"] = dstEchoTop.odim.rscale;
	dstProduct.getWhere()["rscale2"] = dstEchoTop.odim.rscale+10;
	dstProduct.getWhat()["comment"] = "EchoTop2-test";
	//dstProduct.updateTree3(dstEchoTop.odim);

	mout.special<LOG_NOTICE>(*this, "First Data: ", 	dstProduct.getFirstData());


}


}


// Rack