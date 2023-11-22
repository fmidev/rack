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

#ifndef RACK_PRODUCTS
#define RACK_PRODUCTS


#include "drain/prog/CommandInstaller.h"

//#include "resources.h"


namespace rack {

//template <class OP>
//class ProductCommand;


struct ProductSection : public drain::CommandSection {

	ProductSection(): CommandSection("products"){
		// hello(__FUNCTION__); std::cerr << __FUNCTION__ << std::endl;
		drain::CommandBank::trimWords().insert("Op");
	};

};


/*! Connects meteorological products to program core.
 *
 */
class ProductModule : public drain::CommandModule<'p',ProductSection> { //: public drain::CommandSection {

public:

	ProductModule(drain::CommandBank & bank = drain::getCommandBank());


protected:

	template <class OP>
	drain::Command & install(char alias = 0);

	/*

	template <class OP>
	drain::Command & install(char alias = 0){  // TODO: EMBED "install2"
		static const OP op;
		std::string name = op.getName(); // OP()
		drain::CommandBank::deriveCmdName(name, getPrefix());
		drain::Command  & cmd = cmdBank.add<ProductCommand<OP> >(name);
		cmd.section = getSection().index;
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.special(name , "\n -> " , op.getOutputQuantity() , "\t test:" , op.getOutputQuantity("TEST") );
		CmdOutputQuantity::quantityMap[name] =  op.getOutputQuantity();
		return cmd;
	}
	*/

};




} // Rack

#endif
