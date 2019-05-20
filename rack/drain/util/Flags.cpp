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
//#include "Path.h"


#include <stdexcept>
#include <iostream>
#include "Flags.h"
#include "String.h"
#include "Log.h"

namespace drain {


/// Set flags
Flags & Flags::operator =(const std::string & params){

	drain::Logger mout("Flags", __FUNCTION__);

	typedef std::list<std::string> keylist;
	keylist keys;

	if (separator)
		drain::StringTools::split(params, keys, this->separator);
	else
		keys.push_front(params);

	for (keylist::const_iterator it = keys.begin(); it !=keys.end(); ++it) {

		// mout.warn() << " '" << *it << "'" << mout.endl;

		dict_t::first_type::const_iterator dit = dictionary.first.find(*it);

		if (dit != dictionary.first.end()){
			// Alphabetic key found
			value = value | dit->second;
		}
		else {
			// Numeric value
			if (*it == "0"){
				std::cout << "resetting...\n";
				value = 0;
				continue;
			}
			else {
				value_t v;
				std::stringstream sstr(*it);
				sstr >> v;
				if (v == 0){
					throw std::runtime_error(*it + ": key not found in Flags");
				}
				value = value | v;
				// Nice to know
				dict_t::second_type::const_iterator vit = dictionary.second.find(v);
				if (vit != dictionary.second.end()){
					std::cout << "(assigned key '" << vit->second << "')\n";
				}
			}
		}
	}

	return *this;
}

std::ostream & Flags::valueKeysToStream(std::ostream & ostr, char separator) const {

	if (!separator)
		separator = dictionary.separator;

	char sep = 0;
	for (dict_t::first_type::const_iterator it = dictionary.first.begin(); it != dictionary.first.end(); ++it){
		if ((it->second > 0) && ((it->second & value)) == it->second){ // fully covered in value
			if (sep)
				ostr << sep;
			else
				sep = dictionary.separator;
			ostr << it->first;
		}
		else {

		}
	}

	return ostr;
}

/// List keys in their numeric order.
std::ostream & Flags::keysToStream(std::ostream & ostr, char separator) const {

	if (!separator)
		separator = this->separator;

	char sep = 0;
	for (dict_t::second_type::const_iterator it = dictionary.second.begin(); it != dictionary.second.end(); ++it){
		if (sep)
			ostr << sep;
		else
			sep = separator;
		ostr << it->second; // << '='  << it->first;
	}

	return ostr;

}




} // drain::
