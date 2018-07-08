/*

    Copyright 2016 - 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack program for C++.

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

#ifndef RADAR_POLAR_SMOOTHER_H
#define RADAR_POLAR_SMOOTHER_H


#include <sstream>

#include <drain/util/BeanLike.h>
#include <drain/image/Image.h>

#include "data/PolarODIM.h"

// // using namespace std;

namespace rack {


class PolarSmoother { //: public drain::BeanLike {

public:

	static
	void filter(const PolarODIM & odimSrc, const drain::image::Image & src, drain::image::Image & dst, double radiusMetres);

};


}  // rack::


#endif /*GEOMETRY_H_*/
