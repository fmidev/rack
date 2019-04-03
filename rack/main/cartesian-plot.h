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

#ifndef RACK_CART_PLOT
#define RACK_CART_PLOT



//#include <drain/prog/CommandRegistry.h>
#include <drain/prog/CommandAdapter.h>
//#include "data/Quantity.h"

#include "resources.h"



namespace rack {

class CartesianQuantity : public BasicCommand {  //SimpleCommand<std::string> {

public:

	CartesianQuantity() : BasicCommand(__FUNCTION__, "Set quantity of the composite.") {
		parameters.reference("quantity", getResources().composite.odim.quantity);
	}
	/*
	inline
	CartesianQuantity() : SimpleCommand<std::string>(__FUNCTION__, "Set quantity of the composite.", "quantity"){
	};

	inline
	void exec() const {
		getResources().composite.odim.quantity = value;
	};
	*/

};

class CartesianPlot : public BasicCommand {

public:

	double lon;
	double lat;
	double x;
	double w;

	inline
	CartesianPlot() : BasicCommand(__FUNCTION__, "Add a single data point."){
		parameters.reference("lon", lon = 0.0, "longitude");
		parameters.reference("lat", lat = 0.0, "latitude");
		parameters.reference("x", x = 0.0, "value");
		parameters.reference("w", w = 1.0, "weight");
	};

	inline
	void exec() const {
		getResources().composite.allocate();      // TODO conditional
		getResources().composite.addUnprojected(lon, lat, x, w);
	};

};



class CartesianPlotFile : public SimpleCommand<std::string> {

public:

	CartesianPlotFile() : SimpleCommand<>(__FUNCTION__, "Plot file containing rows '<lat> <lon> <value> [weight] (skipped...)'.",
			"file", "", "filename"){
	};

	void exec() const;
};


class CartesianSpread : public BasicCommand {

public:

	double horz;
	double vert;
	int loops;
	//double decay;
	// int height;

	CartesianSpread() : BasicCommand(__FUNCTION__, "Set Spread of the compositing array. OBSOLETE. Use --iDistanceTransformFill(Exp) instead"){ // Does not allocate memory."){
		parameters.reference("horz", horz = 10, "pixels");
		parameters.reference("vert", vert = 0,  "pixels");
		parameters.reference("loops",  loops = 0, "N");
	};

	void exec() const;
};





} // rack::



#endif

// Rack
