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

#ifndef RACK_CART_GRID
#define RACK_CART_GRID



//#include "drain/prog/CommandRegistry.h"
#include "drain/prog/CommandAdapter.h"
//#include "data/Quantity.h"

#include "resources.h"



namespace rack {


class CartesianGrid : public drain::BasicCommand {

public:

	int lonSpacing;
	int latSpacing;
	double width;
	double intensity;

	CartesianGrid() : drain::BasicCommand("CartesianGrid","Draw lat-lon grid onto a Cartesian product.") {
		parameters.link("lonSpacing", lonSpacing = 1, "deg");
		parameters.link("latSpacing", latSpacing = 1, "deg");
		parameters.link("width", width = 1.0, "pix");
		parameters.link("intensity", intensity = 0.5, "");
	};

	void exec() const;

};


} // rack::



#endif

// Rack
