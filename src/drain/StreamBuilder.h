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
 * StringTools.h
 *
 *  Created on: Feb, 2022
 *      Author: mpeura
 */

#ifndef DRAIN_STREAM_BUILDER
#define DRAIN_STREAM_BUILDER


//#include "RegExp.h"
//#include <iostream>
#include <string>
#include <sstream>

#include "Type.h"

namespace drain {

// TODO:
template <char SEP=0>
class StreamBuilder : public std::stringstream {

public:

	template<typename ... TT>
	StreamBuilder(const TT &... args){
		create(args...);
	}

	template<typename ... TT>
	StreamBuilder & create(const TT &... args) {
		std::stringstream::str("");
		return add(args...);
	}

	template<class T, typename ... TT>
	StreamBuilder & add(const T & arg, const TT &... args) {
		if (SEP && (tellp() > 0)){
			*this << (SEP);
		}
		*this << arg;
		return add(args...);
	}



protected:

	template<class T>
	void append(const T & arg) {
		*this << arg;
	}

	void append(const std::type_info & type) {
		*this << Type::call<simpleName>(type);
		// *this << arg;
	}


	// Terminal
	inline
	StreamBuilder & add(){
		 return *this;
	};

};


} // drain::

#endif /* DRAIN_STREAM_BUILDER */

