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


#ifndef ANDRE_PRODUCTS
#define ANDRE_PRODUCTS

//#include "commands.h"
/*
#include "productAdapter.h"
#include "andre/AndreOp.h"
#include "andre/DetectorOp.h"
*/

// Utils
//#include "andre/QualityCombinerOp.h"
//#include "andre/ClutterOp.h"

namespace rack {


struct AnDReSection : public drain::CommandSection {

	inline	AnDReSection(): CommandSection("andre"){
		//hello(__FUNCTION__);
		drain::CommandBank::trimWords().insert("Op");
		drain::CommandBank::trimWords().insert("AnDRe");
	};

};

class AnDReModule : drain::CommandModule<'a',AnDReSection> { //: public drain::CommandSection { //: public drain::CommandGroup {

public:

	AnDReModule(drain::CommandBank & cmdBank = drain::getCommandBank());
	// const std::string & section = "andre", const std::string & prefix = "a"); // : drain::CommandGroup(section, prefix); // { //,
	// clutterMapRead(clutter.productOp) {};
	// AnDReModule(CommandRegistry & registry);

};


} // rack

#endif

// Rack
