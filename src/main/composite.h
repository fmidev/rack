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

// SINCE 2017/06/30,
/*  cCreate = cAdd + cExtract dw
 *  cAdd adds current h5 (polar or cartesian).
 */

#ifndef RACK_COMPOSITING
#define RACK_COMPOSITING


//#include "data/Quantity.h"
#include "data/ODIM.h"

//#include <drain/prog/Command.h>
#include <drain/prog/CommandInstaller.h>


#include "resources.h"



namespace rack {

// Combines program resources and compositing functions.

class Compositor : public drain::BasicCommand {


protected:

	inline
	Compositor(const std::string & name, const std::string & description) : drain::BasicCommand(name, description), weight(1.0) {
	}

	inline
	Compositor(const Compositor & cmd) : drain::BasicCommand(cmd), weight(cmd.weight) {
	}


protected:

	static
	void prepareBBox(const Composite & composite, const drain::BBox & cropGeo, drain::Rectangle<int> & cropImage);

	double applyTimeDecay(Composite & composite,double w, const ODIM & odim) const;

	// Filter is applied by getH5(), so OR function of: RackContext::CARTESIAN, RackContext::POLAR, RackContext::LATEST

	// drain::Flags::ivalue_t
	//void add(Composite & composite, Hdf5Context::h5_role::ivalue_t inputFilter, bool updateSelector = true) const;
	void add(Composite & composite, int inputFilter, bool updateSelector = true) const;
	//void add(drain::Flags::value_t inputFilter, bool updateSelector = true) const;

	void addPolar(Composite & composite, const Hi5Tree & src) const;
	void addCartesian(Composite & composite, const Hi5Tree & src) const;
	//void extract(Composite & composite, const std::string & channels, const drain::Rectangle<double> & bbox = {0,0,0,0}) const;
	// void extract(Composite & composite, const std::string & channels, const std::string & crop = "") const;
	void extract(Composite & composite, const drain::image::Accumulator::FieldList & channels, const std::string & crop = "") const;

	double weight;

	// Composite & getCompositeOLD() const;

};



}



#endif

// Rack
