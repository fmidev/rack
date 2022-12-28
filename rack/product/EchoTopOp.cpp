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
#include "EchoTopOp.h"
// #include "drain/image/File.h"

#include "drain/util/Fuzzy.h"

#include "radar/Constants.h"

namespace rack
{

using namespace drain::image;


void EchoTopOp::processData(const Data<PolarSrc> & sweep, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const {

	drain::Logger mout(__FUNCTION__, __FILE__);
	mout.debug3() << "Start" << mout.endl;
	mout .debug3() << (const drain::image::Accumulator &) accumulator << mout.endl;

	//if (sweep.data.isEmpty())
	//	mout.warn() << "data is empty " << mout.endl;
	//else
	//	mout.debug(5) << "data:\n" << sweep << mout.endl;
	const PlainData<PolarSrc> & srcQuality = sweep.getQualityData();

	const bool USE_QUALITY = ! srcQuality.data.isEmpty();

	const QuantityMap & qm = getQuantityMap();
	const Quantity & HGHT = qm.get("HGHT");
	const double undetectWeight = HGHT.hasUndetectValue() ? DataCoder::undetectQualityCoeff : 0.0;


	mout.info() << "Using quality data: " << (USE_QUALITY?"YES":"NO") << mout.endl;


	/// Derivative
	/*
	drain::image::Image & src = sweep.data;
	drain::image::Image deriv(typeid(unsigned short), sweep.data.getGeometry());
	deriv.initialize(typeid(unsigned short), sweep.data.getGeometry());

	for (size_t i=0; i != deriv.getWidth(); ++i){
		for (size_t j=0; j != deriv.getHeight(); ++j){
			double a = sweep.odim.getAzimuth(i);

		}
	}
	*/


	// Elevation angle
	const double eta = sweep.odim.elangle * drain::DEG2RAD;



	/// Ground angle
	double beta;

	// Bin distance along the beam.
	double binDistance;

	// Source x coordinate?
	// int i2;
	size_t iSweep;


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

	for (size_t i = 0; i < accumulator.accArray.getWidth(); ++i) {

		// Ground angle
		beta = accumulator.odim.getGroundAngle(i); // (static_cast<double>(i)+0.5) * accumulator.rscale / EARTH_RADIUS_43;

		binDistance = Geometry::beamFromEtaBeta(eta, beta);
		if (binDistance < sweep.odim.rstart)
			continue;

		iSweep = sweep.odim.getBinIndex(binDistance); // static_cast<int>(binDistance/sweep.odim.rscale + 0.5);

		if (iSweep >= sweep.odim.area.width)
			continue;

		h = Geometry::heightFromEtaBeta(eta, beta);
		//mout.warn() << "h=" << h << mout.endl;
		wHeight = heightQuality(h);
		wUndetect = 0.10 + 0.40*(1.0-wHeight);
		//if ((binDistance >= sweep.odim.rstart) && (iSweep < sweep.odim.geometry.width)){

		for (size_t j = 0; j < accumulator.accArray.getHeight(); ++j) {

			jSweep = (j * sweep.odim.area.height) / accumulator.accArray.getHeight();

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

				address = accumulator.accArray.data.address(i,j);
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


// Rack
