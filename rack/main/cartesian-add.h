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

#ifndef RACK_CART_ADD
#define RACK_CART_ADD



//#include <drain/prog/CommandRegistry.h>
#include <drain/prog/CommandAdapter.h>
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
class CompositeAdd : public BasicCommand {  // NEW 2017/06

public:

	CompositeAdd() : BasicCommand(__FUNCTION__, "Adds the current product to the composite."), weight(1.0) {};

	void exec() const;

protected:

	CompositeAdd(const std::string & name, const std::string & description) : BasicCommand(name, description), weight(1.0){};

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
		parameters.reference("weight", this->weight = weight, "0...1");
	};

};



} // rack::



#endif
