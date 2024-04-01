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

#ifndef DRAIN_CONVERT_FUNDAMENTAL
#define DRAIN_CONVERT_FUNDAMENTAL

#include <map>
#include <list>
#include <iterator>
#include <sstream>

//#include "Log.h"
//#include "MapTools.h"
//#include "Sprinter.h"
#include "String.h"

namespace drain {

/// Utility class with static conversions
/**
 *
 */
class Convert {

public:

	/// Trivial case: source and destination are of same class.
	template <class T>
	static
	void convert(const T & src, T & dst){
		dst = src;
	}


	template <class S, class D>
	static
	void convert(const S & src, D & dst){
		std::stringstream sstr;
		sstr << src;
		sstr >> dst;
	}

	template <class D>
	static
	void convert(const char * src, D & dst){
		std::stringstream sstr(src);
		//sstr << src;
		sstr >> dst;
	}



};

/// Utility class with static conversions
/**
 *
 */
template <class T>
class Convert2 {

public:

	/// Trivial case: source and destination are of same class.
	static
	void convert(const T & src, T & dst){
		dst = src;
	}

	static
	void convert(const char * src, T & dst){
		std::stringstream sstr(src);
		sstr >> dst;
	}



	template <class D>
	static inline
	void convert(const T & src, D & dst){
		convertFrom(src, dst);
	}


	template <class S>
	static inline
	void convert(const S & src, T & dst){
		convertTo(src, dst);
	}


	/// Convert with cast source type.
	template <class D>
	static
	void convertFrom(const T & src, D & dst){
		std::stringstream sstr;
		sstr << src;
		sstr >> dst;
	}

	/// Convert with cast target type.
	template <class S>
	static
	void convertTo(const S & src, T & dst){
		std::stringstream sstr;
		sstr << src;
		sstr >> dst;
	}


};


/*
inline
std::ostream & operator<<(std::ostream & ostr, const StringMapper & strmap){
	return strmap.toStream(ostr);
}
*/


} // drain

#endif /* STRINGMAPPER_H_ */

// Drain
