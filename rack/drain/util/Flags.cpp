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



FlagResolver::value_t FlagResolver::getFreeBit(const dict_t & dict){

	drain::Logger mout(__FUNCTION__, __FILE__);

	value_t full = 0;
	for (dict_t::const_iterator it = dict.begin(); it != dict.end(); ++it){
		full = full | it->second;
	}

	mout.debug2() << "sum:" << full << mout.endl;

	value_t i = 1;
	while (full>0){
		mout.debug2() << "checking bit:" << i << " vs.\t" << full << mout.endl;
		full = (full>>1);
		i = (i<<1);
	}

	mout.debug() << "released: " << i << mout.endl;

	return i;

}


FlagResolver::value_t FlagResolver::add(dict_t &dict, const dict_t::key_t & key, value_t i){

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (dict.hasKey(key)){
		mout.info() << key << " already in dict: " << dict << mout.endl;
		return dict.getValue(key);
	}

	if (i==0)
		i = getFreeBit(dict);

	if (i>0){
		dict.add(key, i);
	}
	else {
		mout.warn() << key << " could not get a valid (non-zero) bit flag for dict: " << dict << mout.endl;
	}

	return i;
};


FlagResolver::value_t FlagResolver::getValue(const dict_t & dict, const key_t & args, char separator){

	drain::Logger mout(__FUNCTION__, __FILE__);

	value_t v = ::atoi(args.c_str()); // Debugging option: numeric bit flag
	if (v > 0)
		return v;

	typedef std::list<std::string> keylist;
	keylist keys;

	if (!separator)
		separator = dict.separator;

	if (separator)
		drain::StringTools::split(args, keys, separator);
	else
		keys.push_front(args); // single entry, including whatever punctuation...

	//drain::StringTools::split(args, keys, dict.separator);

	for (keylist::const_iterator it = keys.begin(); it !=keys.end(); ++it) {

		if (*it == "0"){
			continue;
		}
		// mout.warn() << " '" << *it << "'" << mout.endl;

		dict_t::const_iterator dit = dict.findByKey(*it);

		if (dit != dict.end()){ // String key match,
			// Numeric value for an alphabetic key was found
			v = (v | dit->second);
		}
		else {
			// Numeric value
			value_t x;
			std::stringstream sstr(*it);
			sstr >> x;
			if (x == 0){
				throw std::runtime_error(*it + ": key not found in Flags");
			}
			v = v | x;
			// Nice to know
			/*
			dict_t::const_iterator vit = dictionaryRef.findByValue(v);
			if (vit != dictionaryRef.end()){
				std::cout << "(assigned key '" << vit->second << "')\n"; // or vit->first?
			}
			*/
		}
	}

	return v;
}


/// List keys in their numeric order.
std::string FlagResolver::getKeys(const dict_t &dict, value_t v, char separator){
	std::stringstream sstr;
	keysToStream(dict, v, sstr, separator);
	return sstr.str();
}



//

/// List keys in their numeric order.
std::ostream & FlagResolver::keysToStream(const dict_t &dict, value_t value, std::ostream & ostr, char separator) {

	/* note: instead of shifting bits of this->value, traverses the dictionary which can contain
		- combined values
		- repeated values (aliases)
	*/

	if (!separator)
		separator = dict.separator;

	char sep = 0;

	for (dict_t::const_iterator it = dict.begin(); it != dict.end(); ++it){
		if ((it->second > 0) && ((it->second & value)) == it->second){ // fully covered in value
			if (sep)
				ostr <<  sep;// "{" << (int)(sep) << "}" <<
			else
				sep = separator;
			ostr << it->first;
		}
		else {

		}
	}

	return ostr;
}




/// Set flags
void Flagger::assign(const key_t & args){

	if (args.empty()){
		drain::Logger mout(__FUNCTION__, __FILE__);
		// Should it reset or skip?
		mout.warn() << "Skipping empty assignment" << mout.endl;
		return;
	}

	if (args == "0"){
		reset();
		return;
	}

	value = getValue(args);

}


/*
std::ostream & Flagger::toStream(std::ostream & ostr, char separator) const {

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
*/

/*
Flags::value_t Flags::add(const dict_t::key_t & key, value_t i){


	if (!usesOwnDict()){
		drain::Logger mout(__FUNCTION__, __FILE__);
		mout.fail() << "could not add " << key << " to external (const) dictionary:" << dictionaryRef << mout.endl;
		return 0;
	}

	return Flagger::add(dictionary, key, i);
};
*/



} // drain::
