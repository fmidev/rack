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
#ifndef RADAR_POLAR_SECTOR_H
#define RADAR_POLAR_SECTOR_H


#include <sstream>

#include <drain/util/BeanLike.h>
#include <drain/util/Range.h>

#include "data/PolarODIM.h"

// // using namespace std;

namespace rack {


class PolarSector : public drain::BeanLike {
    public: //re 
	PolarSector() : drain::BeanLike(__FUNCTION__) {
		parameters.reference("azm",   azm.vect,   "deg");
		parameters.reference("range", range.vect, "km");
		parameters.reference("ray",   ray.vect,   "index");
		parameters.reference("bin",   bin.vect,   "index");
	}

	/// Start azimuth [deg]
	drain::Range<double> azm;
	// double azm1;

	/// End azimuth [deg]
	//  double azm2;

	/// Start distance [km]
	drain::Range<int> range;
	//  int range1;  // km
	/// End distance [km]
	//  int range2;  // km

	/// Start beam [index]
	drain::Range<int> ray;
	// int ray1;
	/// Start beam [index]
	// int ray2;

	/// Start bin [index]
	drain::Range<int> bin;
	//  int bin1;
	/// End bin [index]
	//  int bin2;

	void reset();

	/// Sets range such that range1 < range2.
	inline
	void setRange(int range, int range2 = 0){
		if (range2 < range){
			this->range.set(range2, range);
			// this->range1 = range2;
			// this->range2 = range;
		}
		else {
			this->range.set(range, range2);
			// this->range1 = range;
			// this->range2 = range2;
		}
	};

	/// Sets azimuth(s)
	inline
	void setAzimuth(double azm, double azm2 = std::numeric_limits<double>::max()){

		if (azm2 != std::numeric_limits<double>::max())
			this->azm.set(azm, azm2);
		else
			this->azm.set(azm, azm);

	};

	inline
	int getSafeRay(const PolarODIM & odim, int j){
		j = j % odim.geometry.height;
		if (j < 0)
			j += odim.geometry.height;
		return j;
	}

	/// Given scaling (ODIM nbins, rscale, nrays), convert azimuths and ranges to ray and bin indices.
	void adjustIndices(const PolarODIM & odim);

	/// Given scaling (ODIM nbins, rscale, nrays), convert current azm and range data to ray and bin indices.
	void deriveWindow(const PolarODIM & srcOdim, int & ray1, int & bin1, int & ray2, int & bin2) const;

};


}  // rack::


#endif /*GEOMETRY_H_*/

// Rack
