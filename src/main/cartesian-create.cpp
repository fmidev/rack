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





#include "cartesian-create.h"

namespace rack {

void CartesianCreate::exec() const {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	Composite & composite = ctx.getComposite(RackContext::PRIVATE);

	mout.attention<LOG_DEBUG>("composite*: ", &composite, " accArray: ", composite.accArray);

	//composite.reset();
	composite.accArray.reset(); // Not frame, otherwise --cSize spoiled
	composite.dataSelector.reset();
	composite.odim.clear();
	composite.nodeMap.clear();

	// composite.odim.source.clear();
	// mout.attention("Composite counter N=", composite.odim.ACCnum, ")");

	if (composite.odim.ACCnum > 0){ // .counter
		// mout.experimental("Clearing previous composite? N=", composite.counter, ")");
		// mout.experimental("Clearing previous composite? N=", composite.odim.ACCnum, ")");
		mout.hint<LOG_DEBUG>("Clearing previous composite. If that was not meant, use --cAdd to add, instead.");
		composite.accArray.clear();
		// clear metadata?
	}

	// mout.attention("start add: ", composite.getTargetEncoding(), " odim.quantity=", composite.odim.quantity, " sel:", composite.dataSelector);
	composite.odim.quantity = "";

	add(composite, RackContext::POLAR|RackContext::CURRENT, true);

	if (ctx.statusFlags.value > 0){
		mout.warn("errors (", ctx.statusFlags, "), skipping extraction");
		return;
	}

	const drain::StringMatcher qualityMatcher("QIND"); // coming op: other

	//mout.accept<LOG_NOTICE>("EPSG_A: ", composite.odim.epsg);
	if (qualityMatcher.test(composite.odim.quantity)){
		mout.note("Quality [", composite.odim.quantity, "] as input: extracting data only");
		extract(composite, "d");
	}
	else {
		extract(composite, "dw");
	}

	// mout.accept<LOG_NOTICE>("EPSG: ", composite.odim.epsg);
	// mout.attention("extract dw");


	// When are these needed? Upon one-liner DBZH, VRAD singles?
	composite.dataSelector.setQuantities(""); // why quantity only?
	// mout.experimental("quantity ["," clearance removed");
	composite.odim.quantity.clear();

	// mout.attention("angles: ", drain::sprinter(composite.odim.angles, "<>"));

	// better without...
	// ctx.cartesianHi5[ODIMPathElem::WHAT].data.attributes["source2"] = (*ctx.currentPolarHi5)["what"].data.attributes["source"];
}



void CompositeCreateTile::exec() const {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	//Composite & composite = getComposite();
	Composite & composite = ctx.composite;

	if (!composite.geometryIsSet()){
		mout.error("Composite geometry undefined, cannot create tile");
	}

	if (! composite.bboxIsSet()){
		mout.error("Bounding box undefined, cannot create tile");
	}

	if (! composite.projectionIsSet()){ // or use first input (bbox reset)
		mout.error("Projection undefined, cannot create tile");
	}

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

	if (ctx.statusFlags){ // drain::Status::INPUT_ERROR
		// mout.warn("errors: ", ctx.statusFlags);
		mout.error("errors: ", ctx.statusFlags, " – quitting");
		return;
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


void CartesianCreateLookup::exec() const {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	Composite & composite = ctx.getComposite(RackContext::PRIVATE);

	mout.debug("composite*: ", &composite, "accArray: ", composite.accArray);

	if (!ctx.polarInputHi5.empty())
		mout.warn("polar input not empty");

	composite.createBinIndex(ctx.polarInputHi5);


}


void CartesianReset::exec() const {

	RackContext & ctx = getContext<RackContext>();

	ctx.composite.reset();
	ctx.composite.setTargetEncoding("");
	ctx.composite.odim.source.clear();
	ctx.composite.nodeMap.clear();
	ctx.composite.odim.clear(); // 2022/12
	ctx.composite.legend.clear();
	ctx.unsetCurrentImages();

	// Consider including in reset:
	// ctx.composite.metadataMap.clear();
}


} // rack::


