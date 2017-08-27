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

//#include "data/Conversion.h"
#include "radar/Constants.h"


namespace rack
{

using namespace drain::image;


void CappiOp::processData(const Data<PolarSrc> & sweep, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const {

	drain::MonitorSource mout(name, __FUNCTION__);
	mout.debug(2) << "start" << mout.endl;
	mout.debug(3) << (const drain::image::Accumulator &) accumulator << mout.endl;

	if (sweep.data.isEmpty())
		mout.warn() << "data is empty " << mout.endl;

	const PlainData<PolarSrc> & sweepQuality = sweep.getQualityData();

	const bool USE_QUALITY = ! sweepQuality.data.isEmpty();

	mout.info() << "Using quality data: " << (USE_QUALITY?"YES":"NO") << mout.endl;


	double altitudeFinal;

	if (aboveSeaLevel){
		altitudeFinal = altitude - sweep.odim.height; // radar site height
		if (altitudeFinal < 0.0){
			mout.note() << "requested altitude ("<< altitude << "m ASL) below radar height (" << sweep.odim.height << "m ASL), using the latter." << mout.endl;
			altitudeFinal = 0.0;
		}
	}
	else {
		altitudeFinal = altitude;
		if (altitudeFinal < 0.0){
			mout.warn() << "requested altitude ("<< altitude << ") below 0m, using 0m." << mout.endl;
			altitudeFinal = 0.0;
		}
	}


	mout.info() << "Freezing level: " << sweep.odim.freeze << mout.endl;

	// In this context decoding only.
	DataCoder coder(sweep.odim, sweepQuality.odim);
	//converter.undetectQualityCoeff = CumulativeProductOp::relativeUndetectWeight;
	//converter.undetectValue = -100;

	//converter.undetectQualityCoeff = 0.5;
	//converter.init();

	// Elevation angle
	const double eta = sweep.odim.elangle*DEG2RAD;

	/// Ground angle
	double beta;

	/// Elevation angle of a bin.
	double etaBin;

	// Bin distance along the beam.
	double binDistance;

	// Source value coordinate?
	int iSweep;

	// Source y coordinate?
	int jSweep;

	/// Measurement, encoded
	double value;

	/// Measurement, decoded
	//double dbz;

	/// Beam weight
	double beamWeight; // 0.0...1.0;

	// A fuzzy beam power model, with +/- 0.1 degree beam "width".
	drain::FuzzyBell<double> beamPower(0.0, 0.2*DEG2RAD, 1.0);

	/// Measurement weight (quality)
	double w;

	/// Direct pixel address in the accumulation arrey.
	size_t address;



	for (size_t i = 0; i < accumulator.getWidth(); ++i) {

		// Ground angle
		beta = accumulator.odim.getGroundAngle(i); // (static_cast<double>(i)+0.5) * accumulator.rscale / EARTH_RADIUS_43;

		// Virtual elevation angle of the bin
		etaBin = Geometry::etaFromBetaH(beta, altitudeFinal);

		beamWeight = beamPower( etaBin - eta );
		if (beamWeight < weightMin)
			continue;

		binDistance = Geometry::beamFromEtaBeta(eta, beta);
		if (binDistance < sweep.odim.rstart)
			continue;

		iSweep = sweep.odim.getBinIndex(binDistance);
		if (iSweep >= sweep.odim.nbins)
			continue;

		// TODO: derive iStart and iEnd instead.

		//if ((binDistance >= sweep.odim.rstart) && (iSweep < sweep.odim.nbins)){

		for (size_t j = 0; j < accumulator.getHeight(); ++j) {

			jSweep = (j * sweep.odim.nrays) / accumulator.getHeight(); // todo ODIM.getSweep()

			value = sweep.data.get<double>(iSweep,jSweep);

			// if (i==j) std::cerr << "cappi w0=" << weight << "(" << value << ") => ";

			if (USE_QUALITY){
				w = sweepQuality.data.get<double>(iSweep,jSweep);
				if (!coder.decode(value, w))
					continue;
				w = beamWeight * w;
			}
			else {
				// w = (value==sweep.odim.undetect) ? 0.5 : 1.0; // or default?
				// if (!converter.decode(value, w))
				if (value == sweep.odim.undetect){
					w = beamWeight * DataCoder::undetectQualityCoeff; //converter.undetectQualityCoeff;
					// --> value = coder.undetectValue
					//if (j == 195){
					//	std::cerr << "beam" << j << ",\t w=" << w << " ud=" << sweep.odim.undetect << '\n';
						//w = 0.01;
					//}
					//value = sweep.odim.undetect;
				}
				else {
					w = beamWeight;
				}
				if (!coder.decode(value))
					continue;

				// if (j == 195) std::cerr << "\t val" << value << '\n';
			}

			address = accumulator.data.address(i,j);
			accumulator.add(address, value, w);

		}
		//else
			//mout.warn() << "skipping range b=" << binDistance << " i1=" << iSweep << mout.endl;

	}


}

}

// Rack
