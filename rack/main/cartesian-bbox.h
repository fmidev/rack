/**


    Copyright 2006 - 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#pragma once

#ifndef RACK_CART_BBOX
#define RACK_CART_BBOX



//#include <drain/prog/CommandRegistry.h>
#include <drain/prog/CommandAdapter.h>
//#include "data/Quantity.h"

#include "resources.h"



namespace rack {



class CartesianBBox : public BasicCommand {
    public: //re
	//drain::Rectangle<double> bbox;

	CartesianBBox() : BasicCommand(__FUNCTION__, "Bounding box of the Cartesian product.") {
		RackResources & resources = getResources();
		parameters.reference("llLon", resources.bbox.xLowerLeft = 0.0, "deg");
		parameters.reference("llLat", resources.bbox.yLowerLeft = 0.0, "deg");
		parameters.reference("urLon", resources.bbox.xUpperRight = 0.0, "deg");
		parameters.reference("urLat", resources.bbox.yUpperRight = 0.0, "deg");
	};

	inline
	void exec() const {
		RackResources & resources = getResources();
		resources.composite.setBoundingBoxD(resources.bbox); //llLon, llLat, urLon, urLat);
	};

};
// static RackLetAdapter<CartesianBBox> cBBox("cBBox");

class CartesianBBoxReset : public BasicCommand {
    public: //re
	CartesianBBoxReset() : BasicCommand(__FUNCTION__, "Resets the bounding box (to be set again according to the next radar data).") {};

	inline
	void exec() const {
		RackResources & resources = getResources();
		resources.bbox.set(0,0,0,0);
		resources.composite.setBoundingBoxD(0,0,0,0);
		// options["cSource"].clear();
	};

};
//static RackLetAdapter<CartesianBBoxReset> cBBoxReset("cBBoxReset");

/**
 *   -# "inputOk" (or "0"); prints ovelap/non-overlap through print
 *   -# "exit"    (or "1"); exits, returning value 0 (overlap) or 1 (no overlap)

 */
class CartesianBBoxTest : public SimpleCommand<int> { //
    public: //re
	mutable bool overlap;

	CartesianBBoxTest() : SimpleCommand<int>(__FUNCTION__, "Tests whether the radar range is inside the composite.",
			"mode", 0, "If no overlap, set inputOk=false. If also mode>1, exit with return value <mode>."	) {
	};

	virtual
	inline
	void run(const std::string & params = ""){
		SimpleCommand<int>::run(params);
		value = static_cast<int>(overlap);
	}

	//drain::Rectangle<double> & refBBox;

	void exec() const;

};
//// static RackLetAdapter<CartesianBBoxTest> cBBoxTest("cBBoxTest");


class CartesianBBoxTile : public BasicCommand {
    public: //re
	Rectangle<double> bbox;

	CartesianBBoxTile() : BasicCommand(__FUNCTION__, "Redefines bbox and compositing array size for several radars, applying original projection and resolution. See --cSize, --cBBox, --cProj.") {
		parameters.reference("llLon", bbox.xLowerLeft = 0.0, "deg");
		parameters.reference("llLat", bbox.yLowerLeft = 0.0, "deg");
		parameters.reference("urLon", bbox.xUpperRight = 0.0, "deg");
		parameters.reference("urLat", bbox.yUpperRight = 0.0, "deg");
	}

	void exec() const;

};
//// static RackLetAdapter<CartesianBBoxTile> cBBoxTile("cBBoxTile");




} // rack::



#endif
