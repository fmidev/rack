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


#include "CommandBankUtils.h"

namespace drain {



/// Global program command registry.
CommandBank & getCommandBank(){
	static CommandBank commandBank;
	return commandBank;
}


std::set<std::string> & CommandBankUtils::prunes(){
	static std::set<std::string> s;
	if (s.empty()){
		s.insert("Cmd");
		s.insert("Command");
	}
	return s;
}

void CommandBankUtils::simplifyName(std::string & name, const std::set<std::string> & prune, char prefix){

	Logger mout(__FILE__, __FUNCTION__);

	std::stringstream sstr;

	if (prefix){
		sstr << prefix;
	}
	size_t i = 0;

	bool lowerCase = (prefix == 0);

	while (i < name.size()) {

		mout.debug() << ' ' << i << '\t' << name.at(i) << mout.endl;
		size_t len = 0;

		for (std::set<std::string>::const_iterator it=prune.begin(); it!=prune.end(); ++it){
			// std::cerr << " ..." << *it;
			len = it->length();
			if (name.compare(i, len, *it) == 0){
				// std::cerr << "*";
				break;
			}
			len = 0;
		}

		if (len > 0){
			i += len;
		}
		else {
			if (lowerCase){
				char c = name.at(i);
				if ((c>='A') && (c<='Z'))
					c = ('a' + (c-'A'));
				sstr << c;
				lowerCase = false;
			}
			else {
				sstr << name.at(i);
			}
			++i;
		}
		//mout.debug()
		//std::cerr << '\n';
	}

	name = sstr.str();
};


} /* namespace drain */

// Rack
