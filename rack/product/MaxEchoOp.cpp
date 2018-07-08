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
#include "MaxEchoOp.h"
// #include <drain/image/File.h>

#include <drain/util/Fuzzy.h>

//#include "data/Conversion.h"
#include "radar/Constants.h"


namespace rack
{

using namespace drain::image;

//processSweep(data, cumulator);
void MaxEchoOp::processData(const Data<PolarSrc> & sweep, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const {
	// void MaxEchoOp::filter(const HI5TREE &src, const std::map<double,std::string> & srcPaths, HI5TREE &dst) const {

	drain::Logger mout(drain::getLog(), getName(), __FUNCTION__);
	mout.debug(2) << "Starting MaxEchoOp (" << name << ") " << mout.endl;
	mout.debug(3) << (const drain::image::Accumulator &) accumulator << mout.endl;

	const PlainData<PolarSrc> & srcQuality = sweep.getQualityData();
	const bool WEIGHTED = !srcQuality.data.isEmpty();

	DataCoder converter(sweep.odim, srcQuality.odim);


	mout.info() << "Using quality data: " << (WEIGHTED?"YES":"NO") << mout.endl;


	// Elevation angle
	const double eta = sweep.odim.getElangleR();

	// A fuzzy beam power model.
	//drain::FuzzyPeak<double,double> beamPower(0.0, 0.25*DEG2RAD, 1.0);
	drain::FuzzyBell<double> altitudeQuality(altitude, devAltitude, 1.0);

	/// Ground angle
	double beta;

	/// Elevation angle of a bin.
	//double etaBin;
	double altitudeBin;

	// Bin distance along the beam.
	double binDistance;

	// Source value coordinate?
	int iSweep;

	// Source y coordinate?
	int jSweep;

	/// Measurement, first encoded and then decoded
	double value;

	/// Measurement, decoded
	// double dbz;

	/// Beam weight
	double weight=1.0;

	/// Measurement weight (quality)
	double w;

	/// Direct pixel address in the accumulation arrey.
	size_t address;

	for (size_t i = 0; i < accumulator.getWidth(); ++i) {

		// Ground angle
		beta = accumulator.odim.getGroundAngle(i);

		altitudeBin = Geometry::heightFromEtaBeta(eta, beta);
		weight = altitudeQuality(altitudeBin);
		//if ()
		//weight = 0.7654321;
		//weight = weight*weight;

		binDistance = Geometry::beamFromEtaBeta(eta, beta);
		iSweep = static_cast<int>(binDistance/sweep.odim.rscale + 0.5);

		// TODO: derive iStart and iEnd instead.

		if ((binDistance >= sweep.odim.rstart) && (iSweep < sweep.odim.nbins)){

			for (size_t j = 0; j < accumulator.getHeight(); ++j) {

				jSweep = (j * sweep.odim.nrays) / accumulator.getHeight();

				value = sweep.data.get<double>(iSweep,jSweep);

				//if (i==j) std::cerr << " MaxEcho(" << value << ")\t";

				if (converter.decode(value)){
					address = accumulator.data.address(i,j);
					if (WEIGHTED){
						w = weight * srcQuality.odim.scaleForward(srcQuality.data.get<double>(iSweep,jSweep));
						accumulator.add(address, value, w);
					}
					else {
						//accumulator.add(address, value, weight);
						accumulator.add(address, value, weight);
						w = weight; // debug
					}

				}

			}

		}
		//else
			//mout.warn() << "skipping range b=" << binDistance << " i1=" << iSweep << mout.endl;

	}

	//_mout.writeImage(11, dstData, "max");
}



}




