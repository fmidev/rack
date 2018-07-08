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

	CartesianSpread() : BasicCommand(__FUNCTION__, "Set Spread of the compositing array."){ // Does not allocate memory."){
		parameters.reference("horz", horz = 10, "pixels");
		parameters.reference("vert", vert = 0,  "pixels");
		parameters.reference("loops",  loops = 0, "N");
	};

	void exec() const;
};





} // rack::



#endif
