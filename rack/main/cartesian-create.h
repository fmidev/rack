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

		RackResources & resources = getResources();

		addCmd.exec();
		//size_t n = std::min(resources.composite.getFrameWidth(), resources.composite.getFrameHeight());
		//for (size_t i = 0; i < n; i+=2) {
		//	std::cerr << i << '\t' << resources.composite.data.get<double>(i,i) << '\t' << resources.composite.weight.get<double>(i,i) << '\n';
		//}
		extractCmd.exec();

		resources.cartesianHi5["what"].data.attributes["source"] = (*resources.currentPolarHi5)["what"].data.attributes["source"];
		//const DataSet<PolarSrc> polarSrc((*resources.currentPolarHi5)["dataset1"]);
		//DataSet<CartesianDst> cartDst((*resources.currentPolarHi5)["dataset1"]);
		//cartDst.get  polarSrc.getFirstData().odim.source;
	}

private:

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

// Rack
