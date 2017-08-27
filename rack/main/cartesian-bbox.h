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

// Rack
