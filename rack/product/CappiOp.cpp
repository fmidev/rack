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
#include "CappiOp.h"
// #include <drain/image/File.h>

#include <drain/util/Fuzzy.h>
#include <drain/util/Geo.h>

//#include "data/Conversion.h"
#include "radar/Constants.h"


namespace rack
{

using namespace drain::image;


void CappiOp::processData(const Data<PolarSrc> & sweep, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const {

	drain::Logger mout(name, __FUNCTION__);
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

	// In this context decoding only, ie form bytevalues to physical values.
	DataCoder coder(sweep.odim, sweepQuality.odim);
	mout.info() << "decoder: " << coder << mout.endl;

	// Elevation angle
	const double eta = sweep.odim.getElangleR();

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
	drain::FuzzyBell<double> beamPower(0.0, 0.2*drain::DEG2RAD, 1.0);

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

			jSweep = (j * sweep.odim.nrays) / accumulator.getHeight();

			value = sweep.data.get<double>(iSweep,jSweep);

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

			address = accumulator.data.address(i,j);
			accumulator.add(address, value, w);

		}
		//else
			//mout.warn() << "skipping range b=" << binDistance << " i1=" << iSweep << mout.endl;

	}


}

}
