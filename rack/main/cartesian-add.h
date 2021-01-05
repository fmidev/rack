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

#ifndef RACK_CART_ADD
#define RACK_CART_ADD



//#include "drain/prog/CommandRegistry.h"
#include "drain/prog/CommandAdapter.h"
//#include "data/Quantity.h"

#include "resources.h"



namespace rack {


/// Creates a single-radar Cartesian data set (2D data of both quantity and quality).
/**
 *   Accumulates data to a temporary array ("subcomposite"= and extracts that to a Cartesian product (HDF5).
 *
 *   If a composite has been defined, uses it as a reference of projection, resolution and cropping to geographical bounding box.
 *
 */
class CompositeAdd : public drain::BasicCommand {  // NEW 2017/06

public:

	CompositeAdd() : drain::BasicCommand(__FUNCTION__, "Adds the current product to the composite."), weight(1.0) {};

	void exec() const;


protected:

	double applyTimeDecay(double w, const ODIM & odim) const;

	CompositeAdd(const std::string & name, const std::string & description) : drain::BasicCommand(name, description), weight(1.0){};

	void addPolar() const;
	void addCartesian() const;
	double weight;
};




/**
 *  --cAddWeighted (Creates reference => accepts command line argument).
 */
class CartesianAddWeighted : public CompositeAdd {

public:

	CartesianAddWeighted() : CompositeAdd(__FUNCTION__, "Adds the current product to the composite applying weight.") {
		parameters.link("weight", this->weight = weight, "0...1");
	};

};



} // rack::



#endif

// Rack
