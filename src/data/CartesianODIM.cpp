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

#include <drain/Log.h>
#include "CartesianODIM.h"

namespace rack {




void CartesianODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	object = "COMP";

	if (initialize & ODIMPathElem::ROOT){

		// NOTE: these should be in DATASET level
		link("where:projdef", projdef = "");
		link("where:xsize", area.width  = 0L); //xsize = 0L);
		link("where:ysize", area.height = 0L); //ysize = 0L);
		link("where:xscale", xscale = 0.0);
		link("where:yscale", yscale = 0.0);
		//
		/*
		LL_lon = bboxD.lowerLeft.x;
		LL_lat = bboxD.lowerLeft.y;
		UR_lon = bboxD.upperRight.x;
		UR_lat = bboxD.upperRight.y;
		 */
		link("where:UR_lon", bboxD.upperRight.x = 0.0);
		link("where:UR_lat", bboxD.upperRight.y = 0.0);
		link("where:LL_lon", bboxD.lowerLeft.x = 0.0);
		link("where:LL_lat", bboxD.lowerLeft.y = 0.0);
		link("where:UL_lon", UL_lon = 0.0);
		link("where:UL_lat", UL_lat = 0.0);
		link("where:LR_lon", LR_lon = 0.0);
		link("where:LR_lat", LR_lat = 0.0);
		link("how:camethod", camethod = "");

		link("how:nodes", nodes = "");

		link("how:ACCnum", ACCnum = 0);

		// link("how:EPSG", epsg = 0);
		link("where:EPSG", epsg = 0);
	}


	if (initialize & ODIMPathElem::DATASET){
		// link("where:xsize", xsize);
		// link("where:ysize", ysize);
	}


	if (initialize & ODIMPathElem::DATA){
	}

}



void CartesianODIM::setGeometry(size_t cols, size_t rows){

	if ((cols == area.width) && (rows == area.height)){
		return;
	}

	// std::cerr << "CartesianODIM::setGeometry (re)scaling. \n";

	if (area.width > 0)
		xscale *= static_cast<double>(area.width) / static_cast<double>(cols);

	if (area.height > 0)
		yscale *= static_cast<double>(area.height) / static_cast<double>(rows);

	// or getScale
	// xscale = geoFrame.getXScale();
	// yscale = geoFrame.getYScale();

	area.width = cols;
	area.height = rows;

}

void CartesianODIM::updateGeoInfo(const drain::image::GeoFrame & geoFrame){

	area.width = geoFrame.getFrameWidth();
	area.height = geoFrame.getFrameHeight();

	projdef = geoFrame.getProjection();
	epsg = geoFrame.projGeo2Native.getDst().getEPSG();

	bboxD = geoFrame.getBoundingBoxDeg();
	/*
	const drain::Rectangle<double> &bboxD = geoFrame.getBoundingBoxDeg();
	LL_lon = bboxD.lowerLeft.x;
	LL_lat = bboxD.lowerLeft.y;
	UR_lon = bboxD.upperRight.x;
	UR_lat = bboxD.upperRight.y;
	*/

	/// Complete other cornerpoints (non-ODIM)
	double x2,y2;
	geoFrame.pix2LLdeg(0,-1, x2,y2); // (vertically outside)
	UL_lon = x2;
	UL_lat = y2;
	geoFrame.pix2LLdeg(area.width,area.height-1, x2,y2);  // (horizontally outside)
	LR_lon = x2;
	LR_lat = y2;

	xscale = geoFrame.getXScale();
	yscale = geoFrame.getYScale();

}


}  // namespace rack
