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


#ifndef RACK_CART_CREATE
#define RACK_CART_CREATE



#include <drain/prog/CommandInstaller.h>
#include "resources.h"

#include "composite.h"

namespace rack {



/// Creates a single-radar Cartesian data set (2D data of both quantity and quality).
/**
 *   Accumulates data to a temporary array ("subcomposite"= and extracts that to a Cartesian product (HDF5).
 *
 *   If a composite has been defined, uses it as a reference of projection, resolution and cropping to geographical bounding box.
 *
 */
class CartesianCreate : public Compositor {

public:


	CartesianCreate() : Compositor(__FUNCTION__, "Maps the current polar product to a Cartesian product."){
	}


	inline
	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		Composite & composite = ctx.getComposite(RackContext::PRIVATE);

		mout.debug("composite*: ", &composite, "accArray: ", composite.accArray);

		if (composite.counter > 0){
			mout.experimental("Clearing previous composite? N=", composite.counter, ")");
			mout.note("Use --cAdd to add, instead");
			composite.accArray.clear();
		}

		// mout.attention("start add: ", composite.getTargetEncoding(), " odim.quantity=", composite.odim.quantity, " sel:", composite.dataSelector);
		composite.odim.quantity = "";

		add(composite, RackContext::POLAR|RackContext::CURRENT, true);

		if (ctx.statusFlags.value > 0){
			mout.warn("errors (", ctx.statusFlags, "), skipping extraction");
			return;
		}

		QuantityMatcher qualityMatcher("QIND"); // coming op: other

		if (qualityMatcher.test(composite.odim.quantity)){
			mout.note("Quality [", composite.odim.quantity, "] as input: extracting data only");
			extract(composite, "d");
		}
		else {
			extract(composite, "dw");
		}
		// mout.attention("extract dw");


		// When are these needed? Upon one-liner DBZH, VRAD singles?
		composite.dataSelector.setQuantities(""); // why quantity only?
		// mout.experimental("quantity ["," clearance removed");
		composite.odim.quantity.clear();

		// mout.attention("angles: ", drain::sprinter(composite.odim.angles, "<>"));

		// better without...
		// ctx.cartesianHi5[ODIMPathElem::WHAT].data.attributes["source2"] = (*ctx.currentPolarHi5)["what"].data.attributes["source"];
	}

};


class CompositeCreateTile : public Compositor {

public:

	inline
	CompositeCreateTile() : Compositor(__FUNCTION__, "Maps the current polar product to a tile to be used in compositing."){
	}

	inline
	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		//Composite & composite = getComposite();
		Composite & composite = ctx.composite;

		if (!composite.geometryIsSet())
			mout.error("Composite geometry undefined, cannot create tile");

		if (! composite.bboxIsSet())
			mout.error("Bounding box undefined, cannot create tile");

		if (! composite.projectionIsSet()) // or use first input (bbox reset)
			mout.error("Projection undefined, cannot create tile");

		if ((composite.odim.ACCnum > 0) || (!composite.odim.quantity.empty())){
			mout.debug("Clearing previous composite...");
			// Consider: composite.clear() ?
			composite.accArray.clear();
			composite.odim.quantity.clear();
			composite.odim.ACCnum = 0;
			composite.odim.scaling.set(0,0);
			composite.odim.type.clear(); // ? risky
			mout.info("Cleared previous composite");
		}


		composite.setCropping(true);
		//add(composite, RackContext::POLAR|RackContext::CURRENT);
		add(composite, RackContext::POLAR|RackContext::CURRENT, true); // updateSelector
		extract(composite, "dw");

		// "Debugging"
		if (!composite.isCropping()){
			mout.warn("Composite cropping switched off during op");
			mout.error("? Programming error in parallel comp design");
		}

		composite.setCropping(false);

	}

private:


	// const CompositeAdd & addCmd;
	// const CompositeExtract & extractCmd;
};


class CartesianRange : public drain::BasicCommand { //SimpleCommand<double> {

public:

	inline
	CartesianRange() : drain::BasicCommand(__FUNCTION__, "Force a range for single-radar cartesian products (0=use-metadata)."){
		parameters.link("range", PolarODIM::defaultRange, "km");
	};

	inline
	CartesianRange(const CartesianRange & cmd) : drain::BasicCommand(__FUNCTION__, cmd.getDescription()) {
		parameters.link("range", PolarODIM::defaultRange, "km");
	};

};

/// Creates a single-radar Cartesian data set (2D data of both quantity and quality).
/**
 *   Accumulates data to a temporary array ("subcomposite"= and extracts that to a Cartesian product (HDF5).
 *
 *   If a composite has been defined, uses it as a reference of projection, resolution and cropping to geographical bounding box.
 *
 */
class CartesianCreateLookup : public drain::BasicCommand {

public:


	CartesianCreateLookup() : drain::BasicCommand(__FUNCTION__, "Creates lookup objects"){
	}


	inline
	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		Composite & composite = ctx.getComposite(RackContext::PRIVATE);

		mout.debug("composite*: ", &composite, "accArray: ", composite.accArray);

		if (!ctx.polarInputHi5.empty())
			mout.warn("polar input not empty");

		composite.createBinIndex(ctx.polarInputHi5);


	}

};

/// Creates a single-radar Cartesian data set (2D data of both quantity and quality).
/**
 *   Accumulates data to a temporary array ("subcomposite"= and extracts that to a Cartesian product (HDF5).
 *
 *   If a composite has been defined, uses it as a reference of projection, resolution and cropping to geographical bounding box.
 *
 */
class CartesianReset : public drain::BasicCommand {

public:

	CartesianReset() : drain::BasicCommand(__FUNCTION__, "Clears the current Cartesian product."){
	}

	inline
	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		ctx.composite.reset();
		// Consider including in reset:
		ctx.composite.setTargetEncoding("");
		ctx.composite.odim.source.clear();
		ctx.composite.nodeMap.clear();
		ctx.composite.metadataMap.clear();
		// ctx.composite.projR2M.clear() !
		// ctx.projStr.clear();

		ctx.composite.odim.clear(); // 2022/12
		//ctx.composite.odim.type.clear();
		//ctx.composite.odim.scaling.scale = 0.0;

		ctx.unsetCurrentImages();

	}

};

} // rack::



#endif

// Rack
