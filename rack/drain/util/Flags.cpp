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
void Flags::assign(const std::string & params){

	drain::Logger mout(__FUNCTION__, __FILE__);

	value = 0;

	typedef std::list<std::string> keylist;
	keylist keys;

	if (separator)
		drain::StringTools::split(params, keys, this->separator);
	else
		keys.push_front(params);

	for (keylist::const_iterator it = keys.begin(); it !=keys.end(); ++it) {

		// mout.warn() << " '" << *it << "'" << mout.endl;

		dict_t::const_iterator dit = dictionaryRef.findByKey(*it);

		if (dit != dictionaryRef.end()){
			// Alphabetic key found
			set(dit->second);
			//value = value | dit->second;
		}
		else {
			// Numeric value
			if (*it == "0"){
				//std::cout << "resetting...\n";
				//value = 0;
				reset();
				continue;
			}
			else {
				value_t v;
				std::stringstream sstr(*it);
				sstr >> v;
				if (v == 0){
					throw std::runtime_error(*it + ": key not found in Flags");
				}
				set(v);
				//value = value | v;
				// Nice to know
				//dict_t::second_type::const_iterator vit = dictionary.second.find(v);
				dict_t::const_iterator vit = dictionaryRef.findByValue(v);
				if (vit != dictionaryRef.end()){
					std::cout << "(assigned key '" << vit->second << "')\n"; // or vit->first?
				}
			}
		}
	}

	//return *this;
}

/// List keys in their numeric order.
std::ostream & Flags::keysToStream(std::ostream & ostr, char separator) const {

	//if (!separator)separator = dictionary.separator;

	/* note: instead of shifting bits of this->value, studies dictionary which can contain
		- combined values
		- repeated values (aliases)
	*/
	char sep = 0;
	for (dict_t::const_iterator it = dictionaryRef.begin(); it != dictionaryRef.end(); ++it){
		if ((it->second > 0) && ((it->second & value)) == it->second){ // fully covered in value
			if (sep)
				ostr << sep;
			else
				sep = separator ? separator : this->separator; // dictionary.separator;
			ostr << it->first;
		}
		else {

		}
	}

	return ostr;
}

std::ostream & Flags::toStream(std::ostream & ostr, char separator) const {

	if (!separator)
		separator = this->separator;

	char sep = 0;
	//for (dict_t::second_type::const_iterator it = dictionary.second.begin(); it != dictionary.second.end(); ++it){
	for (dict_t::const_iterator it = dictionaryRef.begin(); it != dictionaryRef.end(); ++it){
		if (sep)
			ostr << sep;
		else
			sep = separator;
		ostr << it->second; // << '='  << it->first;
	}

	return ostr;

}




} // drain::
