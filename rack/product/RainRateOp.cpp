/*

    Copyright 2013-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack library for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */
#include <drain/image/File.h>

#include <drain/util/Fuzzy.h>


//#include "andre/AndreOp.h"
#include "radar/Geometry.h"
#include "radar/Precipitation.h"

#include "RainRateOp.h"



namespace rack
{

using namespace drain::image;

// These are contorolled directly from command line. See rack.cpp.
PrecipitationZ RainRateOp::precipZrain(200.0, 1.60);
PrecipitationZ RainRateOp::precipZsnow(223.0, 1.53); // Leinonen
PrecipitationKDP RainRateOp::precipKDP;
PrecipitationKDPZDR RainRateOp::precipKDPZDR;
PrecipitationZZDR RainRateOp::precipZZDR;

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
	drain::FuzzyBell<double> snowQuality(0.0, 5.0, maxQuality); // QPE based on DBZH

	/// Check if elevation scan or a ground-level product.
	const bool SCAN = (srcData.odim.product == "SCAN");
	if (!SCAN){
		mout.error() << "input data not of type SCAN " << mout.endl;
	}

	const double elangleR = srcData.odim.getElangleR();

	double rain, rainQ;
	double snow, snowQ;

	double rateEnc;
	double quality;

	const double dstMin = dstData.data.getMin<double>();
	const double dstMax = dstData.data.getMax<double>();
	//mout.warn() << dstMin << '-' << dstMax << mout.endl;

	const bool USE_METADATA_FLEVEL = isnan(freezingLevel.height);

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
		quality = 1.0 - 0.5*freezingLayer(height);

		// if ((i&15) == 0) std::cout << beam << "m,\t h=" << height << "m,\t p=" << pFreeze <<"m, q=" << quality << std::endl;

		// TODO: use other height information (HEIGHT in PseudoCAPPI)

		// if (i >= dstData.data.getWidth())
		//	mout.error() << "overflow i=" << i << mout.endl;

		for (int j = 0; j < srcData.odim.nrays; ++j) {
			//if (j >= dstData.data.getHeight())
			//	mout.error() << "overflow j=" << j << mout.endl;


			dbz = srcData.data.get<double>(i,j);
			if (dbz != srcData.odim.nodata){
				if (dbz != srcData.odim.undetect){
					dbz = srcData.odim.scaleForward(dbz);
					rain = RainRateOp::precipZrain.rainRate(dbz); //, pFreeze);
					snow = RainRateOp::precipZsnow.rainRate(dbz); //, pFreeze);
					rainQ = rainQuality(rain);
					snowQ = snowQuality(snow);

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
					//dstQuality.data.put(i,j, 250.0);
				}
				else {
					dstData.data.put(i,j, dstData.odim.undetect);
					dstQuality.data.put(i,j, 0);
				}
			}
			else
				dstData.data.put(i,j, dstData.odim.nodata);
		}
	}

	//mout.warn() << "Finished." << mout.endl;

}


}
