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

#include "data/ODIM.h"

// // using namespace std;

namespace rack {


class PolarSector : public drain::BeanLike {
    public: //re 
	PolarSector() : drain::BeanLike(__FUNCTION__) {
		parameters.reference("azm1", azm1 = 0.0, "deg");
		parameters.reference("azm2", azm2 = 0.0, "deg");
		parameters.reference("range1", range1 = 0, "km");
		parameters.reference("range2", range2 = 0, "km");
		parameters.reference("ray1", ray1 = 0, "index");
		parameters.reference("ray2", ray2 = 0, "index");
		parameters.reference("bin1", bin1 = 0, "index");
		parameters.reference("bin2", bin2 = 0, "index");
	}

	/// Start azimuth [deg]
	double azm1;

	/// End azimuth [deg]
	double azm2;

	/// Start distance [km]
	int range1;  // km
	/// End distance [km]
	int range2;  // km

	/// Start beam [index]
	int ray1;
	/// Start beam [index]
	int ray2;

	/// Start bin [index]
	int bin1;
	/// End bin [index]
	int bin2;

	void reset();

	/// Sets range such that range1 < range2.
	inline
	void setRange(int range, int range2 = 0){
		if (range2 < range){
			this->range1 = range2;
			this->range2 = range;
		}
		else {
			this->range1 = range;
			this->range2 = range2;
		}
	};

	/// Sets azimuth(s)
	inline
	void setAzimuth(double azm, double azm2 = std::numeric_limits<double>::max()){

		this->azm1 = azm;

		if (azm2 != std::numeric_limits<double>::max())
			this->azm2 = azm2;
		else
			this->azm2 = azm;

	};

	inline
	int getSafeRay(const PolarODIM & odim, int j){
		j = j%odim.nrays;
		if (j < 0)
			j += odim.nrays;
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
