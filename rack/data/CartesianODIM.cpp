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

#include <drain/util/Log.h>

#include "CartesianODIM.h"

namespace rack {




void CartesianODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	object = "COMP";

	if (initialize & ODIMPathElem::ROOT){
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


	if (initialize & ODIMPathElem::DATASET){
	}


	if (initialize & ODIMPathElem::DATA){
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



// Rack
