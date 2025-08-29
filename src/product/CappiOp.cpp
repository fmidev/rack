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
#include "CappiOp.h"
// #include <drain/image/File.h>

#include <drain/util/Fuzzy.h>
#include <drain/util/Geo.h>
#include <drain/image/FilePng.h>

//#include "data/Conversion.h"
#include "radar/Constants.h"


namespace rack
{

using namespace drain::image;


// r = radians
/*
double CappiOp::beamPowerGauss(double r) const {
	// e^x = ½ => x = ln(1/2) = - ln(2)
	// e^(-x²) = ½ => -x = ln(1/sqrt(2)) = -ln(sqrt2) =
	//x = ln(sqrt2)
	const double coeff = log(sqrt(2.0));
	double b = drain::DEG2RAD*beam.width;
	return exp(- (r*r) * coeff/(b*b));

	//return exp(- (d*d) * coeff/(width*width));

}
*/
CappiOp::CappiOp(double altitude, bool aboveSeaLevel, double beamWidth, double weightMin, std::string accumulationMethod) :
	CumulativeProductOp(__FUNCTION__, "Constant-altitude planar position indicator", accumulationMethod)
{

	parameters.link("altitude", this->altitude = altitude, "metres");
	parameters.link("aboveSeaLevel", this->aboveSeaLevel = aboveSeaLevel);
	parameters.link("beamWidth", this->beam.width = beamWidth, "deg"); //"virtual beam width");
	parameters.link("weightMin", this->weightMin = weightMin, "-0.1|0...1");
	parameters.link("accumulationMethod", this->accumulationMethod = accumulationMethod, "string");
	parameters.link("height", this->COMPUTE_HGHT = false, "true|false");
	//parameters.link("weightExponent", this->weightExponent = weightExponent, "scalar");

	//dataSelector.setQuantities("^DBZH$");
	dataSelector.setQuantities("DBZH:DBZHC:DBZ:DBZV");
	// NOTE: caused problems with some BALTRAD radars
	//dataSelector.setPrf(DataSelector::Prf::SINGLE);
	dataSelector.setPrf(DataSelector::Prf::ANY);

	odim.product  = "PCAPPI";
	odim.type = "";
	odim.quantity = "DBZH"; // <- NOTE: only a default, for palette automagics?

};

CappiOp::CappiOp(const CappiOp &op) : CumulativeProductOp(op), altitude(op.altitude), beam(op.beam), weightMin(op.weightMin) {
	// parameters.copyStruct(op.getParameters(), op, *this);
};


void CappiOp::processData(const Data<PolarSrc> & sweep, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	// mout.debug3("start");
	mout.debug3((const drain::image::Accumulator &) accumulator );

	if (sweep.data.isEmpty()){
		mout.fail("data is empty " );
		return;
	}

	/* For pseudoCappi
	drain::Range<double> elangleRange;
	elangleRange.set(+90.0, -90.0);
	for (double e : accumulator.odim.angles){
		elangleRange.min = std::min(elangleRange.min, e);
		elangleRange.max = std::min(elangleRange.max, e);
	}
	*/

	const PlainData<PolarSrc> & sweepQuality = sweep.getQualityData();

	const bool USE_QUALITY = ! sweepQuality.data.isEmpty();

	mout.debug("Elangle: " ,  sweep.odim.elangle );

	mout.debug("Using quality data: " , USE_QUALITY );


	double altitudeFinal;

	if (aboveSeaLevel){
		altitudeFinal = altitude - sweep.odim.height; // radar site height
		if (altitudeFinal < 0.0){
			mout.note("requested altitude (", altitude, "m ASL) below radar, using site height (", sweep.odim.height, "m ASL).");
			altitudeFinal = 0.0;
		}
	}
	else {
		altitudeFinal = altitude;
		if (altitudeFinal < 0.0){
			mout.warn("requested altitude (", altitude, ") less than 0m, using 0m.");
			altitudeFinal = 0.0;
		}
	}

	mout.debug("Freezing level: " , sweep.odim.freeze );

	// In this context decoding only, ie form bytevalues to physical values.
	DataCoder coder(sweep.odim, sweepQuality.odim);

	// Optional! Used only if height=true (odim.quantity=HGHT)
	// DataCoder heightCoder(odim, sweepQuality.odim);
	// mout.attention("heightCoder: ", accumulator.odim);
	//mout.attention("heightCoder: ", accumulator.odim);
	// DataCoder heightCoder(accumulator.odim, sweepQuality.odim);
	mout.experimental("hey");

	if (!COMPUTE_HGHT){
		mout.debug("decoder: ", (const drain::BeanLike &)(coder));
	}
	else {
		mout.experimental("hey");
		//mout.experimental("height [HGHT] decoder: ", (const drain::BeanLike &)(heightCoder));
	}

	// Elevation angle
	const double eta = sweep.odim.getElangleR();

	/// Ground angle
	double beta;

	/// Elevation angle of a bin.
	double etaBin;

	// Bin distance along the beam.
	double binDistance;

	// Source value coordinate?
	size_t iSweep;
	int t; // test

	// Source y coordinate?
	size_t jSweep;

	/// Measurement, encoded
	double value;

	/// Beam weight
	double beamWeight; // 0.0...1.0;

	/// Prevent bell curve falling under a limit
	const double beamWeightMin = (this->weightMin < 0.0) ? -this->weightMin : 0.0;

	// double finalWeight
	// A fuzzy beam power model, with +/- 0.1 degree beam "width".
	// drain::FuzzyBell<double> beamPower(0.0, beamWidth*drain::DEG2RAD, 1.0);

	/// Measurement weight (quality)
	double w;

	/// Direct pixel address in the accumulation arrey.
	size_t address;

	// std::cerr << "coeff: "<< -log(sqrt(2.0)) << '\n';


	for (size_t i = 0; i < accumulator.accArray.getWidth(); ++i) {

		// Ground angle
		beta = accumulator.odim.getGroundAngle(i); // (static_cast<double>(i)+0.5) * accumulator.rscale / EARTH_RADIUS_43;

		// Virtual elevation angle of the bin
		etaBin = Geometry::etaFromBetaH(beta, altitudeFinal);

		//beamWeight = 1.0;
		// if ((i&16) == 0)
		//	beamWeight = beamPower( etaBin - eta );
		//else
		beamWeight = beam.getBeamPowerRad(etaBin - eta);
		//beamWeight = beamPowerGauss(etaBin - eta);

		//if ((!pseudo) && (beamWeight < weightMin))
		if (beamWeight < weightMin)
			continue;
		// Prevent bell curve falling neglible
		beamWeight = std::max(beamWeightMin, beamWeight);

		binDistance = Geometry::beamFromEtaBeta(eta, beta);
		if (binDistance < sweep.odim.rstart)
			continue;

		t = sweep.odim.getBinIndex(binDistance);
		if (t < 0) // unneeded test? Was tested equivalently above already?
			continue;

		iSweep = static_cast<size_t>(t);
		if (iSweep >= sweep.odim.area.width)
			continue;

		//std::cerr << "cappi " << sweep.odim.elangle << '\t' << (etaBin*drain::RAD2DEG) << '\t' << '>' << ' ' << beamWeight << '\n';

		// TODO: derive iStart and iEnd instead.
		//if ((binDistance >= sweep.odim.rstart) && (iSweep < sweep.odim.geometry.width)){

		if (!COMPUTE_HGHT){


			for (size_t j = 0; j < accumulator.accArray.getHeight(); ++j) {

				jSweep = (j * sweep.odim.area.height) / accumulator.accArray.getHeight();

				value = sweep.data.get<double>(iSweep, jSweep);

				// if (i==j) std::cerr << "cappi w0=" << weight << "(" << value << ") => ";

				if (USE_QUALITY){
					w = sweepQuality.data.get<double>(iSweep,jSweep);
					if (!coder.decode(value, w))
						continue;
					w = beamWeight * w;
				}
				else {
					if (value == sweep.odim.undetect){
						w = beamWeight * DataCoder::undetectQualityCoeff; //converter.undetectQualityCoeff;
					}
					else {
						w = beamWeight;
					}
					if (!coder.decode(value))
						continue;

				}

				address = accumulator.accArray.data.address(i,j);
				accumulator.add(address, value, w);
				//accumulator.add(address, value, 1.0);

			}

		}
		else { // EXPERIMENTAL

			// double d = accumulator.odim.getBinDistance(i);
			// double height = Geometry::heightFromEtaGround(sweep.odim.getElangleR(), d);
			double height = Geometry::heightFromEtaBeta(sweep.odim.getElangleR(), beta);

			mout.experimental<LOG_DEBUG>("Computed HGHT:", height);

			for (size_t j = 0; j < accumulator.accArray.getHeight(); ++j) {

				jSweep = (j * sweep.odim.area.height) / accumulator.accArray.getHeight();

				value = sweep.data.get<double>(iSweep, jSweep);

				// if (i==j) std::cerr << "cappi w0=" << weight << "(" << value << ") => ";
				if (value == sweep.odim.nodata){
					continue;
				}

				if (USE_QUALITY){
					w = sweepQuality.data.get<double>(iSweep,jSweep);
					// if (!heightCoder.decode(value, w))
					//	continue;
					w = beamWeight * w;
				}
				else {
					if (value == sweep.odim.undetect){
						w = beamWeight * DataCoder::undetectQualityCoeff; //converter.undetectQualityCoeff;
					}
					else {
						w = beamWeight;
					}
					//if (!heightCoder.decode(value))
					//	continue;

				}

				address = accumulator.accArray.data.address(i,j);
				//accumulator.add(address, value, w);
				accumulator.add(address, height, w);
			}
		}
	}
	/*
	std::stringstream filename;
	filename << "sweep-" << sweep.odim.elangle << ".png";
	drain::image::FilePng::write(sweep.data, filename.str());
	drain::image::FilePng::write(accumulator.data, "acc.png");
	*/
}

}

// Rack
