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



//#include "drain/prog/CommandRegistry.h"
#include "drain/prog/CommandAdapter.h"
//#include "data/Quantity.h"

#include "resources.h"



namespace rack {



class CartesianBBox : public drain::BasicCommand {

public:

	CartesianBBox() : drain::BasicCommand(__FUNCTION__, "Bounding box of the Cartesian product.") {
		RackResources & resources = getResources();
		parameters.link("llLon", resources.bbox.lowerLeft.x = 0.0, "deg");
		parameters.link("llLat", resources.bbox.lowerLeft.y = 0.0, "deg");
		parameters.link("urLon", resources.bbox.upperRight.x = 0.0, "deg");
		parameters.link("urLat", resources.bbox.upperRight.y = 0.0, "deg");
	};

	void exec() const;

private:

	/// Checks if a coordinate looks like metric, that is, beyond [-90,+90] or [-180,+180]
	/*
	static inline
	bool isMetric(double x, double limit){
		return (x < -limit) || (x > limit);
	}
	*/

};


class CartesianBBoxReset : public drain::BasicCommand {

public:

	CartesianBBoxReset() : drain::BasicCommand(__FUNCTION__, "Resets the bounding box (to be set again according to the next radar data).") {};

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
class CartesianBBoxTest : public drain::SimpleCommand<int> { //

public:

	mutable bool overlap;

	CartesianBBoxTest() : drain::SimpleCommand<int>(__FUNCTION__, "Tests whether the radar range is inside the composite.",
			"mode", 0, "If no overlap, set inputOk=false. If also mode>1, exit with return value <mode>."	) {
	};

	virtual
	inline
	void run(const std::string & params = ""){
		SimpleCommand<int>::run(params);
		value = static_cast<int>(overlap);
	}


	void exec() const;

};


class CartesianBBoxTile : public drain::BasicCommand {

public:

	drain::Rectangle<double> bbox;

	CartesianBBoxTile() : drain::BasicCommand(__FUNCTION__, "Redefines bbox and compositing array size for several radars, applying original projection and resolution. See --cSize, --cBBox, --cProj.") {
		parameters.link("llLon", bbox.lowerLeft.x = 0.0, "deg");
		parameters.link("llLat", bbox.lowerLeft.y = 0.0, "deg");
		parameters.link("urLon", bbox.upperRight.x = 0.0, "deg");
		parameters.link("urLat", bbox.upperRight.y = 0.0, "deg");
	}

	void exec() const;

};


} // rack::



#endif

// Rack
