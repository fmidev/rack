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

#include <drain/Log.h>
#include "data/Quantity.h"
#include "data/QuantityMap.h"
//#include <drain/util/Log.h>
//#include <drain/util/Variable.h>
#include <drain/image/Image.h>
#include <product/RadarProductOp.h>
#include "product/PseudoRhiOp.h"
#include "radar/Geometry.h"
#include <algorithm>
#include <cmath>
#include <map>
//#include <stdexcept>
#include <utility>



namespace rack {

void PseudoRhiOp::setPolarSelector(const PolarSelector & ps){
	// polarSelector = ps;

	if (odim.range.empty()){
		drain::Logger mout(__FILE__, __FUNCTION__);
		//mout.experimental("using shared polarSelector: azm:", ps.azimuth.range);

		mout.experimental<LOG_NOTICE>("using shared polarSelector");

		if (ps.radius.range.empty()){
			mout.warn("(also) shared polarSelector radius empty");
			odim.range.set(0.0, 250000.0);
		}
		else {
			odim.range.set(ps.radius.range.tuple());
		}
		mout.note("setting range: ", odim.range);

		if (!ps.azimuth.range.empty()){
			mout.warn("polarSelector azimuth is a range: ", ps.azimuth.range, " using start");
		}
		odim.az_angle = ps.azimuth.range.min;

	}

	/*
	if (!(ps.azimuth.range.empty() && ps.radius.range.empty())){
		drain::Logger mout(__FILE__, __FUNCTION__);
		odim.range.set(ps.radius.range);
		odim.az_angle = 0.5*(ps.azimuth.range.min + ps.azimuth.range.max);
		mout.revised<LOG_WARNING>("Using polarSelector values: azm=", odim.az_angle, "[m], range: ", odim.range, "[m]");
		// mout.info(this->getName(), ": no support for polar selector");
		mout.attention(DRAIN_LOG(odim.range), " ", DRAIN_LOG(odim.az_angle));
	}
	*/
}

void PseudoRhiOp::setGeometry(const PolarODIM & srcODIM, PlainData<RhiDst> & dstData) const {

	dstData.odim.area.width = odim.area.width;
	dstData.odim.area.height = odim.area.height;
	if (dstData.odim.area.width == 0){
		dstData.odim.area.width = 200;
	}
	if (dstData.odim.area.height == 0){
		dstData.odim.area.height = 100;
	}
	dstData.odim.altitudeRange = odim.altitudeRange;
	//dstData.odim.secondheight = odim.secondheight;
	//dstData.odim.firstRange  = odim.firstRange;
	/*
	if (this->polarSelector.radius.range.empty()){
		dstData.odim.range     = odim.range;
		dstData.odim.az_angle  = odim.az_angle;
	}
	else {
		drain::Logger mout(__FILE__, __FUNCTION__);
		dstData.odim.range.set(this->polarSelector.radius.range);
		dstData.odim.az_angle = 0.5*(this->polarSelector.azimuth.range.min + this->polarSelector.azimuth.range.max);
		mout.revised<LOG_WARNING>("Using polarSelector values: azm=", dstData.odim.az_angle, "[m], range: ", dstData.odim.range, "[m]");
	}
	*/
	dstData.odim.range     = odim.range;
	dstData.odim.az_angle  = odim.az_angle;
	dstData.odim.xscale    = (odim.range.max - odim.range.min)/static_cast<double>(odim.area.width);
	dstData.odim.yscale    = (odim.altitudeRange.max - odim.altitudeRange.min)/static_cast<double>(odim.area.height);

	if (!dstData.odim.type.empty()){
		dstData.data.setType(dstData.odim.type.at(0));
	}
	dstData.data.setGeometry(odim.area.width, odim.area.height);
	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.attention(DRAIN_LOG(dstData.odim));
}

void PseudoRhiOp::computeSingleProduct(const DataSetMap<PolarSrc> & src, DataSet<RhiDst> & dstProduct) const {
	//void PseudoRhiOp::processVolume(const Hi5Tree & src, Hi5Tree &dst) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	//mout.warn(parameters  );
	//mout.warn(odim  );

	if (src.empty()){
		mout.warn("Empty input data, skipping. Selector=" , dataSelector );
		return;
	}

	const DataSet<PolarSrc> & srcDataSet = src.begin()->second;
	const Data<PolarSrc> & srcData = srcDataSet.getFirstData();
	const std::string & quantity = srcData.odim.quantity;

	if (srcDataSet.size() > 1){
		mout.note("Multiple data: several quantities. Using quantity=" , quantity );
	}
	else {
		mout.debug("Using quantity=" , quantity );
	}

	Data<RhiDst> & dstData = dstProduct.getData(quantity);
	dstData.odim.quantity = quantity;
	//mout.warn(dstData.odim );

	// setEncoding(srcData.odim, dstData);
	initDst(srcData.odim, dstData);

	// mout.warn(odim         );
	mout.warn(dstData.odim );

	if ((odim.area.width==0) || (odim.area.height==0)){
		mout.warn("empty image: " ,  DRAIN_LOG(dstData.data.getGeometry()) );
		return;
	}

	if (dstData.data.isEmpty()){
		mout.warn("empty image: " ,  DRAIN_LOG(dstData.data) );
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
	// TODO: ODIM VERSION check
	//const double rangeResolution = 1000.0 * dstData.odim.xscale; //(odim.range - odim.firstRange) / static_cast<double>(odim.geometry.width);
	const double rangeResolution  = dstData.odim.xscale; //(odim.range - odim.firstRange) / static_cast<double>(odim.geometry.width);
	const double heightResolution = dstData.odim.yscale; // (odim.secondheight - odim.firstheight) / static_cast<double>(odim.geometry.height);

	const double beamWidth2 = beamWidth*beamWidth*(M_PI/180.0)*(M_PI/180.0);

	// mout.warn(" rangeResolution:  " ,  rangeResolution  );
	// mout.warn(" heightResolution: " ,  heightResolution );


	// "Ground angle" = distance / earthRadius43
	double beta;

	// Angle between radar beam and ground line.
	// double gamma; // check name

	// Altitude of location (i,j)
	double h;

	/// Virtual elevation at the point (i,k)
	double etaPixel;

	// QUANTITY
	double x;

	/*
	// QUANTITY
	double xLower = 0.0, xUpper = 0.0;

	// WEIGHT
	double weightLower = 0.0, weightUpper = 0.0; //, weight;

	// Use the beam (scope ok, data available)
	bool USE_UPPER = false;
	bool USE_LOWER = false;

	bool UNDETECT_UPPER = false;
	bool UNDETECT_LOWER = false;
	*/


	//const double weightThreshold = 0.1;

	struct beamInfo {
		double eta;      //
		double x=0;      // measurement value
		double weight=0;
		bool USE = false;
		bool UNDETECT = false;
	};

	beamInfo upper;
	beamInfo lower;

	/*
	std::map<std::string,double>
	const bool REPLACE_UNDETECT_VALUE = false;
	double undetect_replace =
	 */
	// QuantityMap & qmap = getQuantityMap();
	const Quantity & quantityInfo = getQuantityMap().get(dstData.odim.quantity);
	mout.debug("Using quality info:" );
	mout.debug2(quantityInfo );

	const bool USE_TRUE_UNDETECT = !quantityInfo.hasUndetectValue();
	const double undetectValue = quantityInfo.undetectValue;
	mout.note(DRAIN_LOG(quantityInfo.hasUndetectValue()));

	/// bin index
	int bin;

	/// Azimuthal bin index (j coordinate)
	int azm;

	/*
	if (this->polarSelector.radius.range.empty()){
		azm = odim.az_angle;
	}
	else {

		// warn if non-empty azimuthal range?
		azm = 0.5 * (this->polarSelector.azimuth.range.min + this->polarSelector.azimuth.range.max);
	}
	*/

	// Vice versa?
	const int azmInverse = (static_cast<int>(dstData.odim.az_angle) + 180) % 360;
	const int azmForward =  static_cast<int>(dstData.odim.az_angle)        % 360;

	mout.warn(DRAIN_LOG(azmInverse), DRAIN_LOG(azmForward));

	//const drain::Frame2D<int> srcArea(srcData.odim.area);
	double eta;
	double weight; // max(upper,lower)

	/// MAIN LOOPS
	//  Traverse range (horz dimension)
	for (unsigned int i = 0; i < odim.area.width; ++i) {

		// TODO: check ODIM VERSION
		//beta   = (1000.0*odim.range.min + static_cast<double>(i) * rangeResolution) / Geometry::EARTH_RADIUS_43;
		// Ground angle
		//beta   = (odim.range.min + static_cast<double>(i) * rangeResolution) / Geometry::EARTH_RADIUS_43;
		beta   = (odim.range.min + static_cast<double>(i) * rangeResolution) / Geometry::EARTH_RADIUS_43;

		if (beta < 0.0){
			azm = azmInverse;
			beta = -beta;
		}
		else {
			azm = azmForward;
		}

		// lower.USE = false;
		// double etaLower = -M_PI;
		// double etaUpper = -M_PI;
		// lower.eta = -M_PI;
		// upper.eta = -M_PI;

		for (unsigned int j = odim.area.height-1; j>0; --j) {
				//const unsigned int j = j;

				// Height in metres.
				h   = odim.altitudeRange.min + static_cast<double>(odim.area.height-j-1) * heightResolution;

				// "Virtual elevation angle" of the pixel.
				etaPixel = Geometry::etaFromBetaH(beta, h);

				// beam.reset()?
				upper.USE    = false;
				upper.eta    = +M_PI;
				// upper.weight = 0.0;
				lower.USE    = false;
				lower.eta    = -M_PI;
				// lower.weight = 0.0;

				// Exhaustive search - logarithmic sorted search cannot be used as
				for (const auto & entry: src){

					// mout.accept<LOG_WARNING>(entry.first, ": ", entry.second.getFirstData().odim.elangle);
					// dstData.odim.angles.insert(dstData.odim.angles.end(), entry.second.getFirstData().odim.elangle);

					const DataSet<PolarSrc>   & srcDataSet = entry.second;
					const Data<PolarSrc>      & srcData = srcDataSet.getFirstData(); // todo 1) what if differs from quantity  2) control quantity?
					const drain::Frame2D<int> srcArea(srcData.odim.area);

					//const double eta = srcData.odim.elangle*drain::DEG2RAD;
					eta = srcData.odim.elangle*drain::DEG2RAD;
					// USE: srcData.odim.getBinIndex()
					bin = static_cast<int>(Geometry::beamFromEtaBeta(eta, beta) / srcData.odim.rscale - srcData.odim.rstart);

					if ((bin >= 0) && (bin < srcArea.width)){

						x = srcData.data.get<double>(bin, (azm*srcArea.height)/360);

						if (x != srcData.odim.nodata){

							// Is it lower than the pixel in focus...
							if (eta < etaPixel){
								/// ... but still as high as or higher than previous
								if (eta >= lower.eta){
									lower.eta = eta;
									lower.USE = true;
									if (x == srcData.odim.undetect) {
										lower.x = undetectValue;
										lower.UNDETECT = USE_TRUE_UNDETECT;
									}
									else {
										lower.x = srcData.odim.scaleForward(x);
										lower.UNDETECT = false;
									}
								}
							}
							/// Ok, it is higher or as high as the pixel in focus
							else {
								/// ... but still as low as or lower than previous
								if (eta < upper.eta){
									upper.eta = eta;
									upper.USE = true;
									if (x == srcData.odim.undetect) {
										upper.x = undetectValue;
										upper.UNDETECT = USE_TRUE_UNDETECT;
									}
									else {
										upper.x = srcData.odim.scaleForward(x);
										upper.UNDETECT = false;
									}
								}

							}
						} // if (x != srcData.odim.nodata)

					} // if ((bin >= 0) && (bin < srcArea.width)){
				}


				// if (i == 100) std::cerr << "Height:" << h << " elangle:" << etaPixel << " etaPixel:" << etaPixel << "\n lower.eta:" << lower.eta << "\n upper.eta:" << upper.eta << '\n';


				// Beam proximity test.
				if (lower.USE){
					lower.weight = relativeBeamPower(etaPixel - lower.eta, beamWidth2);
				}

				// Beam proximity test.
				if (upper.USE){
					upper.weight = relativeBeamPower(etaPixel - upper.eta, beamWidth2);
				}

				// Between two elevations
				if (lower.USE && upper.USE){

					// New: rescale
					// Residual of power of one beam at the other beam..
					double coeff = relativeBeamPower(upper.eta - lower.eta, beamWidth2);
					lower.weight = (lower.weight - coeff) / (1.0 - coeff);
					upper.weight = (upper.weight - coeff) / (1.0 - coeff);

					if (lower.UNDETECT && upper.UNDETECT){
						dstData.data.put(i, j, odim.undetect );
					}
					else {
						if (lower.UNDETECT){
							// lower.weight *= 0.5;
							x = upper.x;
						}
						else if (upper.UNDETECT){
							// upper.weight *= 0.5;
							x = lower.x;
						}
						else {
							x =  (lower.weight*lower.x + upper.weight*upper.x) / (lower.weight + upper.weight);
						}
						//if (x > undetectValue)
						// dstData.data.put(i, odim.area.height-1-k, dstData.odim.scaleInverse(x) );
					}
					// dstQuality.data.put(i, odim.area.height-1-k, Q_MAX *  std::max(lower.weight, upper.weight));

					weight = std::max(lower.weight, upper.weight);
					if (weight > weightThreshold.max){
						dstData.data.put(i, j, dstData.odim.scaleInverse(x) );
						dstQuality.data.put(i, j, Q_MAX * weight );
					}
					else if (weight > weightThreshold.min){
						dstData.data.put(i, j, dstData.odim.undetect);
						dstQuality.data.put(i, j, Q_MAX * weight );
					}
					else{
						dstData.data.put(i, j, dstData.odim.nodata);
						dstQuality.data.put(i, j, 0 );
					}

				}
				else if (upper.USE){
					//if (upper.x > undetectValue)
					if (upper.UNDETECT){
						x = odim.undetect;
						// dstData.data.put(i, j, odim.undetect );
					}
					else {
						x = dstData.odim.scaleInverse(upper.x);
						// dstData.data.put(i, j, dstData.odim.scaleInverse(upper.x) );
					}

					// dstQuality.data.put(i, odim.area.height-1-k, Q_MAX * upper.weight );

					if (upper.weight > weightThreshold.max){
						dstData.data.put(i, j, x);
						dstQuality.data.put(i, j, Q_MAX * upper.weight );
					}
					/*
				else if (upper.weight > weightThreshold.min){
					dstData.data.put(i, j, dstData.odim.undetect);
					dstQuality.data.put(i, j, Q_MAX * upper.weight );
				} */
					else{
						dstData.data.put(i, j, dstData.odim.nodata);
						dstQuality.data.put(i, j, 0 );
					}

				}
				else if (lower.USE){
					if (lower.UNDETECT){
						dstData.data.put(i, j, odim.undetect);
					}
					else {
						dstData.data.put(i, j, dstData.odim.scaleInverse(lower.x) );
					}
					// dstQuality.data.put(i, j, Q_MAX * lower.weight );

					if (lower.weight > weightThreshold.max){
						dstQuality.data.put(i, j, Q_MAX * lower.weight );
					}
					/*
				else if (lower.weight > weightThreshold.min){
					dstData.data.put(i, j, dstData.odim.undetect);
					dstQuality.data.put(i, j, Q_MAX * lower.weight );
				}
					 */
					else{
						dstData.data.put(i, j, dstData.odim.nodata);
						dstQuality.data.put(i, j, 0 );
					}

				}
				else { // mark as no-data
					dstData.data.put(i, j, odim.nodata);
					dstQuality.data.put(i, j, 0);
				}

				//if (lower.USE)					dstData.data.put(i, odim.geometry.height-1-k, 192);

				// mout.debug2("elev=" , it->first , "\t : " , it->second );

		}  // for j


	} // for i

	dstData.odim.angles.clear();
	dstData.odim.angles.reserve(src.size());
	std::set<double> angles;
	for (const auto & entry: src){
		mout.accept<LOG_WARNING>(entry.first, ": ", entry.second.getFirstData().odim.elangle);
		dstData.odim.angles.insert(dstData.odim.angles.end(), entry.second.getFirstData().odim.elangle);
		angles.insert(entry.second.getFirstData().odim.elangle);
	}

	if (angles.size() < dstData.odim.angles.size()){
		mout.warn("Input data contains repeated angles:",  drain::sprinter(dstData.odim.angles));
	}

	// dstData.odim.angles = {1,2,3};
	//@? dstProduct.updateTree(odim);

}


}  // namespace rack

/// Traverse the sweeps to derive upper and lower beams.
/*
while (true){

	const DataSet<PolarSrc> & srcDataSet = it->second;
	const Data<PolarSrc>      & srcData = srcDataSet.getFirstData(); // todo 1) what if differs from quantity  2) control quantity?
	const drain::Frame2D<int> srcArea(srcData.odim.area);

	const double eta = srcData.odim.elangle*drain::DEG2RAD; //(M_PI/180.0);

	bin = static_cast<int>(Geometry::beamFromEtaBeta(eta, beta) / srcData.odim.rscale - srcData.odim.rstart);


	if ((bin >= 0) && (bin < srcArea.width)){

		x = srcData.data.get<double>(bin, (azm*srcArea.height)/360);

		/// Use this value, if it is valid

		if (x != srcData.odim.nodata){
			/// Update lower beam value (possibly several times)
			if (eta < etaPixel){
				lower.USE = true;
				lower.eta = eta;
				if (x == srcData.odim.undetect) {
					lower.x = undetectValue;
					lower.UNDETECT = USE_TRUE_UNDETECT;
				}
				else {
					lower.x = srcData.odim.scaleForward(x);
					lower.UNDETECT = false;
				}
				//itStart = it; // unneeded?
			}
			/// Update upper beam value (once), update next itStart.
			else {
				upper.USE = true;
				upper.eta = eta;
				//upper.x = srcData.odim.scaleForward(x);
				if (x == srcData.odim.undetect){
					upper.x = undetectValue;
					upper.UNDETECT = USE_TRUE_UNDETECT;
				}
				else {
					upper.x = srcData.odim.scaleForward(x);
					upper.UNDETECT = false;
				}
				// Upper beam (the next upper beam) has been found -> exit search
				itStart = it;
				break;
			}
		}
	}

	++it;
	if (it == src.end()){
		upper.eta = M_PI;
		// consider:  upper.UNDETECT = USE_TRUE_UNDETECT;
		//
		break;
	}

} // while true }


} // if (etaPixel > etaMax) }

*/
// Rack
