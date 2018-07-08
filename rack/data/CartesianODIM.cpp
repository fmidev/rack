/*


    Copyright 2011-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

   Created on: Sep 30, 2010
*/

#include <drain/util/Log.h>

#include "CartesianODIM.h"

namespace rack {




void CartesianODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	object = "COMP";

	if (initialize & ROOT){
		//reference("what:object", );
		reference("where:projdef", projdef = "");
		reference("where:xsize", xsize = 0L);
		reference("where:ysize", ysize = 0L);
		reference("where:xscale", xscale = 0.0);
		reference("where:yscale", yscale = 0.0);
		//
		reference("where:UR_lon", UR_lon = 0.0);
		reference("where:UR_lat", UR_lat = 0.0);
		reference("where:UL_lon", UL_lon = 0.0);
		reference("where:UL_lat", UL_lat = 0.0);
		reference("where:LR_lon", LR_lon = 0.0);
		reference("where:LR_lat", LR_lat = 0.0);
		reference("where:LL_lon", LL_lon = 0.0);
		reference("where:LL_lat", LL_lat = 0.0);
		reference("how:camethod", camethod = "");
		reference("how:nodes", nodes = "");
	}


	if (initialize & DATASET){
	}


	if (initialize & DATA){
	}

}



void CartesianODIM::setGeometry(size_t cols, size_t rows){

	if (xsize)
		xscale = xscale * static_cast<double>(xsize) / static_cast<double>(cols);
	if (ysize)
		yscale = yscale * static_cast<double>(ysize) / static_cast<double>(rows);

	xsize = cols;
	ysize = rows;

	/*
	// Determining horizontal and vertical scale in meters.
	// More reliably computed from degrees than from the metric BoundingBoxM, because the latter can actually be in radians!
	// Uses a 2 x 2 pix box in the centre of the grid.
	const int im = xsize/2;
	const int jm = ysize/2;
	drain::Rectangle<double> box;
	pix2deg(im-1, jm-1, box.xLowerLeft,  box.yLowerLeft);
	pix2deg(im+1, jm+1, box.xUpperRight, box.yUpperRight);
	//mout.warn() << box << mout.endl;
	odim.xscale = (box.xUpperRight-box.xLowerLeft )/2.0 * DEG2RAD * 2.0 * rack::Geometry::EARTH_RADIUS;
	odim.yscale = (box.yLowerLeft -box.yUpperRight)/2.0 * DEG2RAD * 2.0 * rack::Geometry::EARTH_RADIUS * cos(DEG2RAD*(box.yLowerLeft+box.yUpperRight)/2.0);
	*/
}



}  // namespace rack


