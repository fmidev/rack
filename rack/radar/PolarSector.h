/*

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

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
#ifndef RADAR_POLAR_SECTOR_H
#define RADAR_POLAR_SECTOR_H


#include <sstream>

#include <drain/util/BeanLike.h>

#include "data/PolarODIM.h"

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
