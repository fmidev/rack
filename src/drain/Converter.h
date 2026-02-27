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

#ifndef DRAIN_CONVERTER
#define DRAIN_CONVERTER

// #include <map>
// #include <list>
// #include <iterator>
#include <sstream>

#include "Log.h"
//#include "MapTools.h"
//#include "Sprinter.h"
// #include "StringTools.h"
//#include "drain/EnumUtils.h"

namespace drain {

/// Utility class with static conversions
/**
 *  For now, just a StringConverter, other types are future options.
 */
template <typename T>
class Converter {
public:

	static
	void convert(const T & value, std::string &s){
		std::ostringstream sstr;
		sstr << value;
		if (!sstr.good()){
			Logger(__FILE__, __FUNCTION__).warn("failed in reading ", value);
		}
		s.assign(sstr.str());
	};

	static
	void convert(const std::string &s, T & value){
		std::istringstream sstr(s);
		sstr >> value;
		if (sstr){
			Logger(__FILE__, __FUNCTION__).warn(sstr.str(), " was unread from ", s);
		}
	};


};

// ChatGPT
/*
template <class Enum>
struct enum_traits;

template <class E>
struct enum_traits<E> {

    static std::string_view to_string(E e) {
    	return Enum<E>::dict.getKey(e);
    }

    static E from_string(std::string_view s) {
    	return Enum<E>::dict.getValue(s);
    }
};
*/


} // drain

#endif // DRAIN_CONVERTER


