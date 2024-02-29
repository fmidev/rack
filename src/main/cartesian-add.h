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
class CompositeAdd : public Compositor { //drain::BasicCommand {  // NEW 2017/06

public:

	CompositeAdd() : Compositor(__FUNCTION__, "Adds the current product to the composite."){}; //, weight(1.0) {};

	virtual inline
	void exec() const {
		// Accept Cartesian and polar
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		Composite & composite = ctx.getComposite(RackContext::SHARED);

		mout.debug("starting add(), thread #", ctx.getName());


		add(composite, RackContext::CURRENT, true);

		mout.debug("ended add(), thread #", ctx.getName());

		// composite.updateInputSelector(ctx.select);
		// CHECK ctx.select.clear();
	}


protected:

	//double applyTimeDecay(double w, const ODIM & odim) const;

};




/**
 *  --cAddWeighted (Creates reference => accepts command line argument).
 */
class CompositeAddWeighted : public Compositor {

public:

	CompositeAddWeighted() : Compositor(__FUNCTION__, "Adds the current product to the composite applying weight.") {
		parameters.link("weight", this->weight = weight, "0...1");
	};

	CompositeAddWeighted(const CompositeAddWeighted & cmd) : Compositor(cmd){
		//parameters.link("weight", this->weight = weight, "0...1");
		parameters.copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK);
	}

	virtual inline
	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		Composite & composite = ctx.getComposite(RackContext::SHARED);
		add(composite, RackContext::CURRENT, true);
		// RackContext & ctx = getContext<RackContext>();
		// Composite & composite = getComposite();
		//add(composite, RackContext::CURRENT);
		// composite.updateInputSelector(ctx.select);
		// CHECK ctx.select.clear();
	}
};



} // rack::



#endif

// Rack
