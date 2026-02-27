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

#ifndef DRAIN_WRAPPER
#define DRAIN_WRAPPER


#include "Converter.h"

namespace drain {


/// String-like object easily supporting conversion from other types, like Enum values.
/**
 *
 *
 *  \see SelectorXML
 *  \see StringWrapper<PseudoClassCSS>
 *
 */
template <typename T>
class StringWrapper : public std::string {

public:

	inline
	StringWrapper(){
	};

	/// All the other constructors, including default constructor.
	//template <typename T>
	inline
	StringWrapper(const T & x){
		set(x);
	};

	// imitating std::string
	StringWrapper<T> & operator=(const std::string & s){
		assign(s);
		return *this;
	}

	StringWrapper<T> & operator=(const char *s){
		assign(s);
		return *this;
	}

	StringWrapper<T> & operator=(char c){
		std::string::operator=(c);
		return *this;
	}

	/// Copy string of another StringWrapper.
	template <typename T2>
	StringWrapper<T> & operator=(const StringWrapper<T2> & s){
		assign(s);
		return *this;
	}

	template <typename T2>
	StringWrapper<T> & operator=(const T2 & x){
		set(x);
		return *this;
	}


	inline
	void set(const std::string & s=""){
		std::string::assign(s);
	};

	inline
	void set(const StringWrapper & e){
		std::string::assign(e);
	};

	inline
	void set(const char *s){
		std::string::assign(s);
	};


	/// Set the value from an other, user-defined dictionary.
	/**
	 *
	 *   - The enum dict must be defined.
	 *
	 *   Loose template: assumes T2 can be converted to T
	 */
	//template <typename T2>
	inline
	void set(const T & x){
		Converter<T>::convert(x, *this);
	};

	inline
	bool operator==(const T & x){
		std::string s;
		Converter<T>::convert(x, s);
		return *this == s;
	}


};



} // drain::

#endif /* STRING_H_ */

