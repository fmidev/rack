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
 
/*
 * RackLet.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: mpe
 */

//#include <fstream>

#include "drain/util/Log.h"

#include "CommandUtils.h"

namespace drain {


typename ScriptTxt::entry_t & ScriptTxt::add(const std::string & cmd, const std::string & params){
	this->push_back(typename list_t::value_type(cmd, params));
	return back();
}



void ScriptTxt::entryToStream(const typename ScriptTxt::entry_t & entry, std::ostream & ostr) const {
	ostr << entry.first << '=' << entry.second << '\n';
}




BasicCommand & Program::add(BasicCommand & cmd){ // const std::string & params = ""){
	push_back(& cmd);
	return cmd;
}


void Program::run() const {

	Logger mout(__FILE__, __FUNCTION__);
	for (const_iterator it = this->begin(); it != this->end(); ++it) {
		BasicCommand & cmd = *(*it);
		mout.warn() << "  executing " << cmd.getName() << '(' << cmd.getParameters() << ')' << mout.endl;
		cmd.exec();
	}

}

void Program::entryToStream(const list_t::value_type & entry, std::ostream & ostr) const{
	ostr << entry->getName() << '(' << entry->getParameters().getValues() << ')';
}


// -----------------------



} /* namespace drain */

// Rack