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

#ifndef DRAIN_FLAGS_H_
#define DRAIN_FLAGS_H_

#include <stdexcept>
#include <iostream>
#include <list>
#include <iterator>
#include <iostream>
#include <sstream>
#include <list>

#include "util/Dictionary.h"


namespace drain {

/// Bit vector accessible also with integer values associated with strings.
class Flags {

public:

	typedef unsigned int value_t;
	//typedef drain::Dictionary<value_t> dict_t;
	typedef drain::Dictionary2<std::string, value_t> dict_t;

	Flags(const dict_t &dictionary, char separator = 0) : value(0), dictionary(dictionary), separator(separator?separator:dictionary.separator) {
	}

	Flags(const Flags & flagset) : value(flagset.value), dictionary(flagset.dictionary), separator(flagset.separator) {

	};

	/// Set flags, combined to
	/**
	 *   \param x - bit values 1, 2, 4, 8 ... etc combined with \c OR function.
	 */
	inline
	Flags & operator=(value_t x){
		value = x;
		return *this;
	}

	/// Set flags
	/**
	 *   \param x - comma separated keys or bit values 1, 2, 4, 8
	 */
	Flags & operator=(const std::string & params);

	operator std::string() const {
		std::stringstream sstr;
		valueKeysToStream(sstr);
		return sstr.str();
	}

	inline
	operator value_t() const {
		return value;
	}

	/// Display current value as key values of which the value is composed with \c OR function.
	/**
	 *   \param ostr
	 *   \param separator - character appended between keys
	 */
	std::ostream & valueKeysToStream(std::ostream & ostr, char separator=0) const;

	/// List all the keys in their numeric order.
	/**
	 *   \param ostr - output stream
	 *   \param separator - character appended between keys
	 */
	std::ostream & keysToStream(std::ostream & ostr, char separator=0) const;

	value_t value;

	const dict_t & dictionary;

	char separator;

};


inline
std::ostream & operator<<(std::ostream & ostr, const drain::Flags & flags) {
	return flags.valueKeysToStream(ostr);
}

} // drain::


#endif
