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

#ifndef RACK_CART_GRID
#define RACK_CART_GRID



//#include <drain/prog/CommandRegistry.h>
#include <drain/prog/CommandAdapter.h>
//#include "data/Quantity.h"

#include "resources.h"



namespace rack {


class CartesianGrid : public BasicCommand {
    public: //re
	int lonSpacing;
	int latSpacing;
	double width;
	double intensity;


	CartesianGrid() : BasicCommand("CartesianGrid","Draw lat-lon grid onto a Cartesian product.") {
		parameters.reference("lonSpacing", lonSpacing = 1, "deg");
		parameters.reference("latSpacing", latSpacing = 1, "deg");
		parameters.reference("width", width = 1.0, "pix");  // , "deg");
		parameters.reference("intensity", intensity = 0.5, "");
	};

	void exec() const;

};
//// static RackLetAdapter<CartesianGrid> cGrid("cGrid");


} // rack::



#endif
