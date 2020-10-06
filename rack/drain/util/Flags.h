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

#include "drain/util/Dictionary.h"


namespace drain {

/// Bit vector accessible also with integer values associated with strings.
class Flags {

public:

	typedef unsigned int value_t;

	typedef drain::Dictionary2<std::string, value_t> dict_t;

	inline
	Flags(value_t & value, const dict_t & d, char separator = 0) : value(value), separator(separator?separator:d.separator), dictionaryRef(d) {
	}

	//Flags(const dict_t & d, value_t & value, char separator = 0) : value(value), separator(separator?separator:d.separator), dictionaryRef(d) {
	//}

	inline
	Flags(const Flags & flags) : value(flags.value), separator(flags.separator), dictionaryRef(flags.dictionaryRef){
	};


	/// Set desired flags. Does not reset any flag.
	inline
	void set(value_t x){
		value = value | x;
	};

	/// Set desired flags. Does not reset any flag.
	/*
	inline
	void set(value_t x, value_t & dst){
		dst = dst | x;
	};
	*/


	inline
	void set(const std::string & key){
		set(dictionaryRef.getValue(key));
	};

	/*
	inline
	void set(const std::string & key, value_t & dst){
		set(dictionaryRef.getValue(key), dst);
	};
	*/


	/// Unset desired flags. Does not set any flag.
	inline
	void unset(value_t x){
		value = (value & ~x);
	};

	/// Unset desired flags. Does not set any flag.
	inline
	void unset(const std::string & key){
		unset(dictionaryRef.getValue(key));
	};

	/// Unset desired flags. Does not set any flag.
	inline
	void unset(value_t x, value_t & dst){
		dst = (dst & ~x);
	};


	/// Set desired flags. Does not unset any flag.
	inline
	void reset(){
		value = 0;
	}

	/// Unset desired flags. Does not set any flag.
	inline
	bool isSet(value_t x){
		return (value & x) != 0;
	};

	inline
	bool isSet(const std::string & key){
		return isSet(dictionaryRef.getValue(key));
	};


	// For some reason this cannot be templated (see below)
	inline
	Flags & operator=(const Flags &x){
		assign(x);
		return *this;
	}


	/// Sets value, ie. set or unsets all the flags.
	/**
	 *   \param x - bit values 1, 2, 4, 8 ... etc combined with \c OR function.
	 */
	template <class T>
	inline
	Flags & operator=(const T &x){
		assign(x);
		return *this;
	}

	/// Sets value, ie. set or unsets all the flags.
	/**
	 *   \param x - bit values 1, 2, 4, 8 ... etc combined with \c OR function.
	 */
	inline
    void assign(value_t x){
		value = x;
	}

	/// Copies flags as an integer \c value. Same dictionaryRef not checked.
	/**
	 *   \param flags -
	 */
	inline
	void assign(const Flags & flags){
		value = flags.value;
	}

	/// Set flags
	/**
	 *   \param x - comma separated keys or bit values 1, 2, 4, 8
	 */
	void assign(const std::string & params);


	inline
	operator std::string() const {
		std::stringstream sstr;
		toStream(sstr);
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
	// Todo rename toStream?
	std::ostream & toStream(std::ostream & ostr=std::cout, char separator=0) const;

	/// List all the keys in their numeric order.
	/**
	 *   \param ostr - output stream
	 *   \param separator - character appended between keys
	 */
	std::ostream & keysToStream(std::ostream & ostr=std::cout, char separator=0) const;

	value_t & value;


	char separator;

protected:

	const dict_t & dictionaryRef;

};

class Flags2 : public Flags {

protected:

	value_t ownValue;

public:

	dict_t dictionary;

	Flags2(char separator = ',') : Flags(ownValue, dictionary, separator){
	}

	Flags2(const Flags2 & flags) : Flags(
			ownValue = flags.value,
			flags.usesOwnDict() ? dictionary=flags.dictionary : flags.dictionaryRef,
			flags.separator){
	}

	/*
	Flags2(char separator = ',') : Flags(dictionary, ownValue, separator){
	}

	Flags2(const Flags2 & flags) : Flags(
			flags.usesOwnDict() ? dictionary=flags.dictionary : flags.dictionaryRef,
			ownValue = flags.value,
			flags.separator){
	}
	*/


	/// Sets value, ie. set or unsets all the flags.
	template <class T>
	inline
	Flags2 & operator=(const T &x){
		assign(x);
		return *this;
	}

protected:

	bool usesOwnDict() const {
		return (&dictionaryRef == &dictionary);
	}
};


inline
std::ostream & operator<<(std::ostream & ostr, const drain::Flags & flags) {
	return flags.keysToStream(ostr);
}

} // drain::


#endif
