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

#ifndef RACK_CART_PLOT
#define RACK_CART_PLOT



//#include "drain/prog/CommandRegistry.h"
#include <drain/prog/CommandInstaller.h>
#include "resources.h"



namespace rack {

/*
class CartesianQuantity : public drain::BasicCommand {

public:

	CartesianQuantity() : drain::BasicCommand(__FUNCTION__, "Set quantity of the composite.") {
		parameters.link("quantity", getResources().composite.odim.quantity);
		// Consider resources.composite.dataSelector.quantity ?
	}

	inline
	void exec() const {

		drain::Logger mout(__FUNCTION__, __FILE__);
		RackResources & resources = getResources();
		//RackContext & ctx = getContext();
		RackResources::baseCtx().targetEncoding = "quantity=" + resources.composite.odim.quantity;
		mout.deprecating() << "use --encoding " << RackResources::baseCtx().targetEncoding << " instead" << mout.endl;
		// = "quantity=" + resources.composite.odim.quantity;

	};

};
*/

class CartesianPlot : public drain::BasicCommand {

public:

	double lon;
	double lat;
	double x;
	double w;

	inline
	CartesianPlot() : drain::BasicCommand(__FUNCTION__, "Add a single data point."){
		parameters.link("lon", lon = 0.0, "longitude");
		parameters.link("lat", lat = 0.0, "latitude");
		parameters.link("x", x = 0.0, "value");
		parameters.link("w", w = 1.0, "weight");
	};

	inline
	void exec() const {
		// consider CompositingModule::initComposite();
		RackContext & ctx = getContext<RackContext>();
		ctx.composite.allocate();      // TODO conditional
		ctx.composite.addUnprojected(lon, lat, x, w);
	};

};



class CartesianPlotFile : public drain::SimpleCommand<std::string> {

public:

	CartesianPlotFile() : drain::SimpleCommand<>(__FUNCTION__, "Plot file containing rows '<lat> <lon> <value> [weight] (skipped...)'.",
			"file", "", "filename"){
	};

	void exec() const;
};


class CartesianSpread : public drain::BasicCommand {

public:

	double horz;
	double vert;
	int loops;

	CartesianSpread() : drain::BasicCommand(__FUNCTION__, "Set Spread of the compositing array. OBSOLETE. Use --iDistanceTransformFill(Exp) instead"){ // Does not allocate memory."){
		parameters.link("horz", horz = 10, "pixels");
		parameters.link("vert", vert = 0,  "pixels");
		parameters.link("loops",  loops = 0, "N");
	};

	void exec() const;
};





} // rack::



#endif

// Rack
