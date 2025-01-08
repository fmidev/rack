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

#ifndef DRAIN_STRING_BUILDER
#define DRAIN_STRING_BUILDER

/*
#include <limits>
#include <iterator>
#include <sstream>
#include <iostream>
#include <list>
#include <map>
*/

//#include "RegExp.h"

#include "StreamBuilder.h"

namespace drain {


template <char SEP=0>
class StringBuilder : public std::string {

public:

	/// Faster short-cut for single-arg initialization.
	StringBuilder(const std::string & arg="") : std::string(arg){
	}


	template<typename ... TT>
	StringBuilder(const TT &... args){
		create(args...);
		//assign(streamBuilder.str());
	}


	template<typename ... TT>
	StringBuilder & create(const TT &... args){
		this->clear();
		this->add(args...);
		return *this;
	}

	template<typename ... TT>
	StringBuilder & add(const TT &... args){
		streamBuilder.create(args...);
		//std::cerr << "Test " << streamBuilder.str() << '\n';
		std::stringbuf *buf = streamBuilder.rdbuf();
		if (buf->in_avail() > 0){ // IMPORTANT! Otherwise corrupts (swallows) output stream
			int c;
			while ((c = buf->sbumpc()) != EOF){ //  != EOF
				// std::cerr << *this << ",\t size:" << this->capacity() << '\n';
				this->push_back(static_cast<char>(c));
				//if (++i > 100) return *this;
			}
		}
		return *this;
	}

	/// For explicit string cast, esp. for (skipping) template deduction.
	const std::string & str() const {
		return *this;
	}

protected:

	drain::StreamBuilder<SEP> streamBuilder;

};


} // drain::

#endif /* DRAIN_STRING_BUILDER_H_ */

