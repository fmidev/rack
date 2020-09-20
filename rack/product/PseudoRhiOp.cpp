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

#include <data/Quantity.h>
#include <data/QuantityMap.h>
//#include "drain/util/Log.h"
//#include "drain/util/Variable.h"
#include "drain/image/Image.h"
#include <product/ProductOp.h>
#include <product/PseudoRhiOp.h>
#include <radar/Geometry.h>
#include "drain/util/Log.h"
#include <algorithm>
#include <cmath>
#include <map>
//#include <stdexcept>
#include <utility>



namespace rack {



void PseudoRhiOp::setGeometry(const PolarODIM & srcODIM, PlainData<RhiDst> & dstData) const {

	dstData.odim.geometry.width = odim.geometry.width;
	dstData.odim.geometry.height = odim.geometry.height;
	if (dstData.odim.geometry.width == 0){
		dstData.odim.geometry.width = 200;
	}
	if (dstData.odim.geometry.height == 0){
		dstData.odim.geometry.height = 100;
	}
	dstData.odim.minheight = odim.minheight;
	dstData.odim.maxheight = odim.maxheight;
	dstData.odim.minRange  = odim.minRange;
	dstData.odim.range     = odim.range;
	dstData.odim.xscale    = (odim.range - odim.minRange)/static_cast<double>(odim.geometry.width);
	dstData.odim.yscale    = (odim.maxheight - odim.minheight)/static_cast<double>(odim.geometry.height);

	if (!dstData.odim.type.empty()){
		dstData.data.setType(dstData.odim.type.at(0));
	}
	dstData.data.setGeometry(odim.geometry.width, odim.geometry.height);

}

void PseudoRhiOp::processDataSets(const DataSetMap<PolarSrc> & src, DataSet<RhiDst> & dstProduct) const {
//void PseudoRhiOp::processVolume(const Hi5Tree & src, Hi5Tree &dst) const {

	drain::Logger mout(__FUNCTION__, __FILE__);
	//mout.warn() << parameters  << mout.endl;
	//mout.warn() << odim  << mout.endl;

	if (src.empty()){
		mout.warn() << "Empty input data, skipping. Selector=" << dataSelector << mout.endl;
		return;
	}

	const DataSet<PolarSrc> & srcDataSet = src.begin()->second;
	const Data<PolarSrc> & srcData = srcDataSet.getFirstData();
	const std::string & quantity = srcData.odim.quantity;

	if (srcDataSet.size() > 1){
		mout.note() << "Multiple data: several quantities. Using quantity=" << quantity << mout.endl;
	}
	else {
		mout.debug() << "Using quantity=" << quantity << mout.endl;
	}

	Data<RhiDst> & dstData = dstProduct.getData(quantity);
	dstData.odim.quantity = quantity;
	//mout.warn() << dstData.odim << mout.endl;

	initDst(srcData.odim, dstData);

	// mout.warn() << odim         << mout.endl;
	// mout.warn() << dstData.odim << mout.endl;

	if ((odim.geometry.width==0) || (odim.geometry.height==0)){
		mout.warn() << "empty image: " <<  dstData.data.getGeometry() << mout.endl;
		return;
	}

	//@ dstData.updateTree(); // TODO later quantity?

	PlainData<RhiDst> & dstQuality = dstData.getQualityData("QIND");
	dstQuality.odim.quantity = "QIND";
	initDst(srcData.odim, dstQuality);
	//dstQuality.setQuantityDefaults("QIND");
	//dstQuality.data.setGeometry(odim.geometry.width, odim.geometry.height);
	//quantityMap.setQuantityDefaults(dstQuality, "QIND");
	//dstQuality.data.setGeometry(dstData.data.getGeometry());
	//@ dstQuality.updateTree();
	const double Q_MAX = dstQuality.odim.scaleInverse(1.0);  // 255

	/// Todo: range (kms) already in dst odim?
	const double rangeResolution = 1000.0 * dstData.odim.xscale; //(odim.range - odim.minRange) / static_cast<double>(odim.geometry.width);
	const double heightResolution = dstData.odim.yscale; // (odim.maxheight - odim.minheight) / static_cast<double>(odim.geometry.height);

	const double beamWidth2 = beamWidth*beamWidth*(M_PI/180.0)*(M_PI/180.0);

	// mout.warn() << " rangeResolution:  " <<  rangeResolution  << mout.endl;
	// mout.warn() << " heightResolution: " <<  heightResolution << mout.endl;


	// "Ground angle" = distance / earthRadius43
	double beta;

	// True, if "behind" the radar, ie. negative beta and range.
	bool REVERSE;

	//
	bool UNDETECT_UPPER = false;
	bool UNDETECT_LOWER = false;

	// Altitude of location (i,j)
	double h;

	/// Virtual elevation at the point (i,k)
	double etaPixel;

	// Use the beam (scope ok, data available)
	bool USE_UPPER = false;
	bool USE_LOWER = false;

	// QUANTITY
	double x, xLower = 0.0, xUpper = 0.0;

	// WEIGHT
	double weightLower = 0.0, weightUpper = 0.0; //, weight;

	//const double weightThreshold = 0.1;

	/*
	std::map<std::string,double>
	const bool REPLACE_UNDETECT_VALUE = false;
	double undetect_replace =
	*/
	// QuantityMap & qmap = getQuantityMap();
	const Quantity & quantityInfo = getQuantityMap().get(dstData.odim.quantity);
	mout.debug()  << "Using quality info:" << mout.endl;
	mout.debug(1) << quantityInfo << mout.endl;

	const bool SKIP_UNDETECT = !quantityInfo.hasUndetectValue();
	const double undetectValue = quantityInfo.undetectValue;
	mout.debug() << "Has zero:" << (int)quantityInfo.hasUndetectValue() << mout.endl;

	/// bin index
	int bin;

	/// Azimuthal bin index (j coordinate)
	int azm; // = static_cast<int>(odim.az_angle) % 360; // TODO!

	/// MAIN LOOPS
	//  Traverse range (horz dimension)
	for (int i = 0; i < odim.geometry.width; ++i) {

		beta   = (1000.0*odim.minRange + static_cast<double>(i) * rangeResolution) / Geometry::EARTH_RADIUS_43;

		REVERSE = (beta < 0.0);

		if (REVERSE){
			beta = -beta;
			azm = (static_cast<int>(odim.az_angle) + 180) % 360;
		}
		else {
			azm = static_cast<int>(odim.az_angle) % 360;
		}

		USE_LOWER = false;
		//double etaMax = -M_PI;
		double etaLower = -M_PI;
		double etaUpper = -M_PI;
		//std::map<double,const Image &>::const_iterator itStart = images.begin();
		DataSetMap<PolarSrc>::const_iterator itStart = src.begin();

		//  Traverse altitude (vert dimension)
		for (int k = 0; k < odim.geometry.height; ++k) {

			h   = odim.minheight + static_cast<double>(k) * heightResolution;
			etaPixel = Geometry::etaFromBetaH(beta, h);

			if (etaPixel > etaUpper){

				/// Who knows, maybe there is no upper beam
				USE_UPPER = false;

				DataSetMap<PolarSrc>::const_iterator it = itStart; // ??

				/// Traverse the sweeps, derive upper and lower beams.
				while (true){

					// Virtual elevation angle
					const double eta0 = it->first*(M_PI/180.0);
					const DataSet<PolarSrc> & srcDataSet = it->second;
					const Data<PolarSrc>      & srcData = srcDataSet.getFirstData(); // todo 1) what if differs from quantity  2) control quantity?

					bin = static_cast<int>(Geometry::beamFromEtaBeta(eta0, beta) / srcData.odim.rscale - srcData.odim.rstart);

					if ((bin >= 0) && (bin < srcData.odim.geometry.width)){

						x = srcData.data.get<double>(bin, (azm*srcData.odim.geometry.height)/360);

						/// Use this value, if it is valid
						//if ((x != srcData.odim.undetect) && (x != srcData.odim.nodata)){
						if (x != srcData.odim.nodata){
							/// Update lower beam value (possibly several times)
							if (eta0 < etaPixel){
								USE_LOWER = true;
								etaLower = eta0;
								if (x != srcData.odim.undetect) {
									xLower = srcData.odim.scaleForward(x);
									UNDETECT_LOWER = false;
								}
								else {
									xLower = undetectValue;
									UNDETECT_LOWER = SKIP_UNDETECT;
								}
								//itStart = it; // unneeded?
							}
							/// Update upper beam value (once), update next itStart.
							else {
								USE_UPPER = true;
								etaUpper = eta0;
								//xUpper = srcData.odim.scaleForward(x);
								if (x != srcData.odim.undetect){
									xUpper = srcData.odim.scaleForward(x);
									UNDETECT_UPPER = false;
								}
								else {
									xUpper = undetectValue;
									UNDETECT_UPPER = SKIP_UNDETECT;
								}
								itStart = it;
								break;
							}
						}
					}

					++it;
					if (it == src.end()){ // images.end()
						etaUpper = M_PI;
						break;
					}
				} // while true }


			} // if (etaPixel > etaMax) }

			// if (i == 100) std::cerr << "Height:" << h << " elangle:" << etaPixel << " etaPixel:" << etaPixel << "\n etaLower:" << etaLower << "\n etaUpper:" << etaUpper << '\n';


			// Beam proximity test.
			if (USE_LOWER){
				weightLower = relativeBeamPower(etaPixel - etaLower, beamWidth2);
				if (weightLower < weightThreshold)
					//USE_LOWER = false;
					weightLower = 0.0;
			}

			// Beam proximity test.
			if (USE_UPPER){
				weightUpper = relativeBeamPower(etaPixel - etaUpper, beamWidth2);
				if (weightUpper < weightThreshold)
					//USE_UPPER = false;
					weightUpper = 0.0;
			}


			if (USE_LOWER && USE_UPPER){
				// x =  weightLower + weightUpper;
				// if ((x > 0.0) && !UNDETECT_LOWER && !UNDETECT_UPPER){
				//	x =  (weightLower*xLower + weightUpper*xUpper) / x;
				if (UNDETECT_LOWER && UNDETECT_UPPER){
					dstData.data.put(i, odim.geometry.height-1-k, odim.undetect );
				}
				else {
					if (UNDETECT_LOWER)
						weightLower = 0.0;
					else if (UNDETECT_UPPER)
						weightUpper = 0.0;
					x =  (weightLower*xLower + weightUpper*xUpper) / (weightLower + weightUpper);
					//if (x > undetectValue)
					dstData.data.put(i, odim.geometry.height-1-k, dstData.odim.scaleInverse(x) );
				}
				dstQuality.data.put(i, odim.geometry.height-1-k, Q_MAX * std::max(weightLower, weightUpper) );
			}
			else if (USE_UPPER){
				//if (xUpper > undetectValue)
				if (!UNDETECT_UPPER)
					dstData.data.put(i, odim.geometry.height-1-k, dstData.odim.scaleInverse(xUpper) );
				else
					dstData.data.put(i, odim.geometry.height-1-k, odim.undetect );
				dstQuality.data.put(i, odim.geometry.height-1-k, Q_MAX * weightUpper );
			}
			else if (USE_LOWER){
				if (! UNDETECT_LOWER)
				//if (xLower > undetectValue)
					dstData.data.put(i, odim.geometry.height-1-k, dstData.odim.scaleInverse(xLower) );
				else
					dstData.data.put(i, odim.geometry.height-1-k, odim.undetect );
				dstQuality.data.put(i, odim.geometry.height-1-k, Q_MAX * weightLower );
			}
			else { // mark as no-data
				dstData.data.put(i, odim.geometry.height-1 - k, odim.nodata);
				dstQuality.data.put(i, odim.geometry.height-1 - k, 0);
			}

			//if (USE_LOWER)					dstData.data.put(i, odim.geometry.height-1-k, 192);

			// mout.debug(1) << "elev=" << it->first << "\t : " << it->second << mout.endl;

		}  // for k


	} // for i

	//@? dstProduct.updateTree(odim);

}


}  // namespace rack

// Rack
