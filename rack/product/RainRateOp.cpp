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

#include <data/Data.h>
#include <data/PolarODIM.h>
#include <data/QuantityMap.h>
#include <image/Image.h>
#include <image/ImageLike.h>
#include <product/RainRateOp.h>
#include <radar/Geometry.h>
#include <util/Fuzzy.h>
#include <util/Log.h>
#include <cmath>
#include <string>



namespace rack
{

using namespace drain::image;

// These are contorolled directly from command line. See rack.cpp.
PrecipitationZ      RainRateOp::precipZrain(200.0, 1.60); // Marshall-Palmer
PrecipitationZ      RainRateOp::precipZsnow(223.0, 1.53); // Leinonen
PrecipitationKDP    RainRateOp::precipKDP;
PrecipitationKDPZDR RainRateOp::precipKDPZDR;
PrecipitationZZDR   RainRateOp::precipZZDR;

FreezingLevel RainRateOp::freezingLevel;

void RainRateOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	drain::Logger mout(name, __FUNCTION__);

	mout.debug(1) << "Start." << mout.endl;

	setGeometry(srcData.odim, dstData);

	PlainData<PolarDst> & dstQuality = dstData.getQualityData();
	setGeometry(srcData.odim, dstQuality);
	//const QuantityMap & qm = getQuantityMap();
	getQuantityMap().setQuantityDefaults(dstQuality, "QIND");
	//dstQuality.data.setGeometry(dstData.data.getGeometry());
	//@ dstQuality.updateTree();

	precipZrain.initParameters();
	precipZsnow.initParameters();

	const double maxQuality = dstQuality.odim.scaleInverse(1.0);
	const double minQuality = dstQuality.odim.scaleInverse(0.5);
	//const double undetectQuality = 0.75 * maxQuality;

	//srcData.odim.toOStr(std::cout, ':', '{','}','\n');
	/*
	mout.warn() << "---------------------\n" << srcData.data << "\n" << mout.endl;
	mout.warn() << "---------------------\n" << srcData.odim << "\n" << mout.endl;
	mout.warn() << dstData << mout.endl;
	mout.warn() << dstQuality << mout.endl;
	*/
	//mout.warn() << EncodingODIM(dstData.odim) << mout.endl;

	//mout.warn() << "\n::: " << srcData.odim << "\n::: " << mout.endl;

	// Quantity (reflectivity) in normal and db scale.
	//double z;
	double dbz;


	// bin altitudes
	double height = 0.0;

	// Distance to the bin along the beam (in metres).
	double beam;

	// Rain rate
	// double rRain, rSnow;

	// Probability of freezing
	double pFreeze = 0.0;
	// Complement of pFreeze.
	double pLiquid = 1.0;

	drain::FuzzyStepsoid<double> freezingProbability(0, 1000.0 * freezingLevel.thickness);
	drain::FuzzyBell<double>     freezingLayer(0, 1000.0 * freezingLevel.thickness);

	drain::FuzzyBell<double> rainQuality(2.0, 10.0, maxQuality); // QPE based on DBZH
	drain::FuzzyBell<double> snowQuality(0.0, 5.0,  maxQuality); // QPE based on DBZH

	/// Check if elevation scan or a ground-level product.
	const bool SCAN = (srcData.odim.product == "SCAN");
	if (!SCAN){
		mout.error() << "input data not of type SCAN " << mout.endl;
	}

	const double elangleR = srcData.odim.getElangleR();

	double rain; //, rainQ;
	double snow; //, snowQ;

	double rateEnc;
	double quality;

	const double dstMin = dstData.data.getEncoding().getTypeMin<double>();
	const double dstMax = dstData.data.getEncoding().getTypeMax<double>();
	//mout.warn() << dstMin << '-' << dstMax << mout.endl;

	const bool USE_METADATA_FLEVEL = std::isnan(freezingLevel.height);

	//const double finalHeight = 1000.0 * (USE_METADATA_FLEVEL ? srcData.odim.freeze : freezingLevel.height);
	const double relativeHeight = 1000.0 * (USE_METADATA_FLEVEL ? srcData.odim.freeze : freezingLevel.height) - srcData.odim.height;

	if (USE_METADATA_FLEVEL){
		mout.note() << "Using freezing level set in metadata: " << srcData.odim.freeze << " (km)" << mout.endl;
	}
	mout.note() << "Site-relative freezing level height: " << relativeHeight << " (m)" << mout.endl;

	// mout.warn() << srcData << mout.endl;
	// mout.note() << srcData.odim << mout.endl;
	// mout.warn() << dstData << mout.endl;

	for (int i = 0; i < srcData.odim.nbins; ++i) {


		// if (SCAN){
		beam = srcData.odim.getBinDistance(i);
		height = Geometry::heightFromEtaBeam(elangleR, beam) - relativeHeight;
		pFreeze = freezingProbability(height);
		pLiquid = 1.0 - pFreeze;
		//quality = 1.0 - 0.5*freezingLayer(height);
		quality = sqrt(pFreeze*pFreeze + pLiquid*pLiquid);

		if ((i&31) == 0)
			mout.debug(10) << i << '\t' << beam << "m,\t h=" << height << "m,\t p=" << pFreeze <<"m, q=" << quality << mout.endl;

		// TODO: use str height information (HEIGHT in PseudoCAPPI)


		for (int j = 0; j < srcData.odim.nrays; ++j) {

			dbz = srcData.data.get<double>(i,j);
			if (dbz != srcData.odim.nodata){
				if (dbz != srcData.odim.undetect){
					dbz = srcData.odim.scaleForward(dbz);
					rain = RainRateOp::precipZrain.rainRate(dbz); //, pFreeze);
					snow = RainRateOp::precipZsnow.rainRate(dbz); //, pFreeze);
					//rainQ = rainQuality(rain);
					//snowQ = snowQuality(snow);

					rateEnc = dstData.odim.scaleInverse(pLiquid*rain + pFreeze*snow);
					if (rateEnc < dstMin){
						dstData.data.put(i,j, dstData.odim.undetect );  // undetect?
						dstQuality.data.put(i,j, quality/5.0);
					}
					else if (rateEnc > dstMax){
						dstData.data.put(i,j, dstData.odim.nodata );  // often no-data?
						dstQuality.data.put(i,j, quality/5.0);
					}
					else {
						dstData.data.put(i,j, rateEnc );
						dstQuality.data.put(i,j, quality);
					}
					dstQuality.data.put(i,j, quality*maxQuality);
					//dstQuality.data.putScaled(i, j);
					//dstQuality.data.put(i,j, 250.0);
				}
				else {
					dstData.data.put(i,j, dstData.odim.undetect);
					dstQuality.data.put(i,j, minQuality);
				}
			}
			else
				dstData.data.put(i,j, dstData.odim.nodata);
		}
	}

	//mout.warn() << "Finished." << mout.endl;

}


}

// Rack
