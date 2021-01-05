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
 
// New design (2020)

#include "drain/util/Log.h"

#include "CommandUtils.h"

namespace drain {


typename Script2::entry_t & Script2::add(const std::string & cmd, const std::string & params){
	this->push_back(typename list_t::value_type(cmd, params));
	return back();
}



void Script2::entryToStream(const typename Script2::entry_t & entry, std::ostream & ostr) const {
	if (entry.second.empty())
		ostr << entry.first << '\n';
	else
		ostr << entry.first << '=' << entry.second << '\n';
}




Command & Program::add(Command & cmd){ // const std::string & params = ""){
	push_back(& cmd);
	if (!cmd.contextIsSet() && contextIsSet())
		cmd.setContext(getBaseContext());
	return cmd;
}

/*
void Program::append(const CommandBank & commandBank, const Script2 & script){
	for (Script2::const_iterator it = script.begin(); it!=script.end(); ++it) {
		BasicCommand & cmd = commands.clone(it->first);
		cmd.setParameters(it->second);
		cmd.setContext(getBaseContext());
		add(cmd);
	}
}
*/


void Program::run() const {

	Logger mout(__FILE__, __FUNCTION__);
	for (const_iterator it = this->begin(); it != this->end(); ++it) {
		Command & cmd = *(*it);
		mout.warn() << "  executing " << cmd.getName() << '(' << cmd.getParameters() << ')' << mout.endl;
		mout.note() << "  context: "  << cmd.getBaseContext().getId() << mout.endl;
		cmd.exec();
	}

}

void Program::entryToStream(const list_t::value_type & entry, std::ostream & ostr) const {
	if (entry->getParameters().empty())
		ostr << entry->getName();
	else {
		// entry->getParameter("s")
		// ostr << entry->getName() << '(' << entry->getParameters().getValues() << ')';
		ostr << entry->getName() << '(';
		entry->getParameters().valuesToJSON(ostr);
		ostr << ')';
	}

}



}

// Rack
