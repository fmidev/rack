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

#ifndef RACK_CART_EXTRACT
#define RACK_CART_EXTRACT


#include "resources.h"
#include "composite.h"


namespace rack {

class CartesianExtract : public Compositor {

public:

	CartesianExtract() : Compositor(__FUNCTION__,"Extract data that has been composited on the accumulation array"){
		getParameters().link("channels", channels="dw", "Layers: data,count,weight,std.deviation"); // Accumulation layers to be extracted
		getParameters().link("bbox", bbox.tuple(), "Optional cropping"); // Accumulation layers to be extracted
	};

	CartesianExtract(const CartesianExtract & cmd) : Compositor(__FUNCTION__, cmd.getDescription()){
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	virtual
	void exec() const;

protected:

	// TODO: replace with MultiFlagEnum etc. D=DATA
	std::string channels;
	drain::Rectangle<double> bbox;

};


/// Creates a single-radar Cartesian data set (2D data of both quantity and quality).
/**
 *   Accumulates data to a temporary array ("subcomposite"= and extracts that to a Cartesian product (HDF5).
 *
 *   If a composite has been defined, uses it as a reference of projection, resolution and cropping to geographical bounding box.
 *
 */
class CartesianSun : public drain::SimpleCommand<std::string> { //drain::BasicCommand {

public:

	CartesianSun() : drain::SimpleCommand<std::string>(__FUNCTION__, "Cartesian sun shine field.", "timestamp", "200507271845")
	{
		//getParameters().link("timestamp", timestamp="200527071845");
		//getParameters().link("quantity", odim.quantity="SUNSHINE");
	}

	/*
	CartesianSun() :  drain::BasicCommand(__FUNCTION__,
			"Sunshine to a Cartesian product.") //, extractCmd(extractCmd)
	{
		getParameters().link("timestamp", timestamp="200527071845");
		//getParameters().link("quantity", odim.quantity="SUNSHINE");
	}
	*/

	void exec() const;

	std::string timestamp;

};



} // rack::



#endif

// Rack
