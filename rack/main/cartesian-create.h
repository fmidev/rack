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

#ifndef RACK_CART_CREATE
#define RACK_CART_CREATE



//#include <drain/prog/CommandRegistry.h>
#include <drain/prog/CommandAdapter.h>
//#include "data/Quantity.h"

#include "resources.h"

#include "cartesian-add.h"
#include "cartesian-extract.h"

namespace rack {



/// Creates a single-radar Cartesian data set (2D data of both quantity and quality).
/**
 *   Accumulates data to a temporary array ("subcomposite"= and extracts that to a Cartesian product (HDF5).
 *
 *   If a composite has been defined, uses it as a reference of projection, resolution and cropping to geographical bounding box.
 *
 */
class CartesianCreate : public BasicCommand {

public:

	CartesianCreate(const CompositeAdd & addCmd, const CartesianExtract & extractCmd) : BasicCommand(__FUNCTION__,
			"Maps the current polar product to a Cartesian product."), addCmd(addCmd), extractCmd(extractCmd)
	{
	}

	inline
	void exec() const {
		addCmd.exec();
		extractCmd.exec();
	}

private:

	void execOLD() const;

	const CompositeAdd & addCmd;
	const CartesianExtract & extractCmd;
};


class CartesianCreateTile : public BasicCommand {

public:

	CartesianCreateTile(const CompositeAdd & addCmd, const CartesianExtract & extractCmd) : BasicCommand(__FUNCTION__,
			"Maps the current polar product to a tile to be used in compositing."), addCmd(addCmd), extractCmd(extractCmd)
	{
	}

	inline
	void exec() const {
		getResources().composite.setCropping(true);
		addCmd.exec();
		extractCmd.exec();
		getResources().composite.setCropping(false);
	}

private:


	const CompositeAdd & addCmd;
	const CartesianExtract & extractCmd;
};


class CartesianRange : public SimpleCommand<int> {

public:

	CartesianRange() : SimpleCommand<int>(__FUNCTION__, "Force range for single-radar cartesian products (0=use-metadata)", "range", 0, "km"){
	}

	inline
	void exec() const {
		getResources().composite.defaultRange = value;
	}
};

} // rack::



#endif
