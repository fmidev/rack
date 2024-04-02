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
 
#include <drain/Log.h>
#include "Command.h"

namespace drain {


void BasicCommand::setParameters(const std::string & args){ //, char assignmentSymbol) {

	const char assignmentSymbol = '=';

	if (args.empty() && !parameters.empty()){
		Logger mout(__FUNCTION__, getName());

		//mout.info() << " empty argument" << mout.endl;
		ReferenceMap::iterator it = parameters.begin();

		if (parameters.size() > 1){
			mout.info() << "resetting 1st parameter (only): " << it->second << mout.endl;
			//mout.note() <<  "parameters: " << parameters << mout.endl;
		}

		if (it->second.isString()){
			it->second.clear();
		}
		else if (it->second.getType() == typeid(bool)){
			mout.note() << it->first << ": empty assignment, interpreting as 'false'" << mout.endl;
			it->second = false;
		}
		else{
			mout.warn() << it->first << ": empty assignment of non-string" << mout.endl;
			//throw std::runtime_error(getName()+":"+it->first + ": empty assigment '' for non-string");
		}
	}
	else if (parameters.separator){
		parameters.setValues(args, assignmentSymbol); //
	}
	else {
		//Logger mout(__FILE__, __FUNCTION__);
		//mout.warn() << "Trying to set values for " << getName() << " params:" << parameters << mout.endl;
		parameters.setValues(args, assignmentSymbol);
		//mout.warn() << "Done (" << args <<  ')' <<  mout.endl;
	}

	this->update();
}

//BasicCommand::BasicCommand(const std::string & name, const std::string & description) : Command(), section(1), name(name), description(description) {

BasicCommand::BasicCommand(const std::string & name, const std::string & description) : Command(), name(name), description(description) {

	if (name.find(' ') != std::string::npos){
		Logger mout(__FILE__, __FUNCTION__);
		mout.error() << "BasicCommand(): name contains space(s): " << name << " descr=" << description << mout.endl;
		// std::cerr << "BasicCommand(): name contains space(s): " << name << " descr=" << description << std::endl;
		// exit(1);
	}

};


}  // drain::
