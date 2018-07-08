/*

    Copyright 2010-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#include "EchoTopOp.h"
// #include <drain/image/File.h>

#include <drain/util/Fuzzy.h>

#include "radar/Constants.h"

namespace rack
{

using namespace drain::image;


void EchoTopOp::processData(const Data<PolarSrc> & sweep, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug(2) << "Start" << mout.endl;
	mout.debug(3) << (const drain::image::Accumulator &) accumulator << mout.endl;

	//if (sweep.data.isEmpty())
	//	mout.warn() << "data is empty " << mout.endl;
	//else
	//	mout.debug(5) << "data:\n" << sweep << mout.endl;
	const PlainData<PolarSrc> & srcQuality = sweep.getQualityData();

	const bool USE_QUALITY = ! srcQuality.data.isEmpty();

	const QuantityMap & qm = getQuantityMap();
	const Quantity & HGHT = qm.get("HGHT");
	const double undetectWeight = HGHT.hasUndetectValue ? DataCoder::undetectQualityCoeff : 0.0;


	mout.info() << "Using quality data: " << (USE_QUALITY?"YES":"NO") << mout.endl;


	// Elevation angle
	const double eta = sweep.odim.elangle * drain::DEG2RAD;

	//const double altitudeFinal = aboveSeaLevel ? (altitude - sweep.odim.height) : altitude;

	// A fuzzy beam power model, with +/- 0.1 degree beam "width".
	//drain::FuzzyPeak<double,double> beamPower(0.0, 0.1*DEG2RAD, 1.0);

	/// Ground angle
	double beta;

	// Bin distance along the beam.
	double binDistance;

	// Source x coordinate?
	int iSweep;

	// Source y coordinate?
	int jSweep;

	/// Measurement, encoded
	double x;

	/// Measurement, decoded
	double dbz;

	/// Beam weight
	//double weight=1.0;

	/// Measurement weight (quality)
	double w;

	double wUndetect;


	// Quality based weights
	float wHeight;

	// Quantity (dBZ)
	//const double dbzRef = -50.0;
	//const double hRef = 15000.0;

	// bin altitude (metres)
	double h,hTop;

	drain::FuzzyBell<double>        dbzQuality(minDBZ, 10.0); // scaled between 0...1
	drain::FuzzyStepsoid<double> heightQuality(500.0, 250.0); // scaled between 0...1

	const double M2KM = 0.001;

	/// Direct pixel address in the accumulation arrey.
	size_t address;

	for (size_t i = 0; i < accumulator.getWidth(); ++i) {

		// Ground angle
		beta = accumulator.odim.getGroundAngle(i); // (static_cast<double>(i)+0.5) * accumulator.rscale / EARTH_RADIUS_43;

		binDistance = Geometry::beamFromEtaBeta(eta, beta);
		if (binDistance < sweep.odim.rstart)
			continue;

		iSweep = sweep.odim.getBinIndex(binDistance); // static_cast<int>(binDistance/sweep.odim.rscale + 0.5);
		if (iSweep >= sweep.odim.nbins)
			continue;

		h = Geometry::heightFromEtaBeta(eta, beta);
		//mout.warn() << "h=" << h << mout.endl;
		wHeight = heightQuality(h);
		wUndetect = 0.10 + 0.40*(1.0-wHeight);
		//if ((binDistance >= sweep.odim.rstart) && (iSweep < sweep.odim.nbins)){

		for (size_t j = 0; j < accumulator.getHeight(); ++j) {

			jSweep = (j * sweep.odim.nrays) / accumulator.getHeight();

			x = sweep.data.get<double>(iSweep,jSweep);

			if (x != sweep.odim.nodata){

				if (x != sweep.odim.undetect) {
					dbz = sweep.odim.scaleForward(x);
					hTop = M2KM *  (hRef + (h-hRef)/(dbz-dbzRef)*(minDBZ - dbzRef));
					// w = weight;
					if (hTop < 0.0){
						//mout.warn() << "h=" << hTop << mout.endl;
						hTop = 0.0;
						w = wUndetect;
					}
					else
						w = wHeight * dbzQuality(dbz) ;
				}
				else {
					hTop = HGHT.undetectValue;
					w = undetectWeight; // DataCoder::undetectQualityCoeff;  //0.0; //wUndetect;
					//hTop = i;
				}

				//if (i == j) if (hTop != 0.0)
				//	std::cerr << " h=" << h <<", dBZ=" << dbz <<":  height=" << hTop << ", w=" << w << std::endl;


				if (USE_QUALITY)
					w = w * srcQuality.odim.scaleForward(srcQuality.data.get<double>(iSweep,jSweep));

				address = accumulator.data.address(i,j);
				//accumulator.add(address, hTop, 255.0 * w);
				accumulator.add(address, hTop, w);

			}
			else {
				//dstData.put(i, j, odimOut.nodata );
				//dstQuality.put(i, j, odimOutQ.nodata );
			}


		}


	}

}



}




