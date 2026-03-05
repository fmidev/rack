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

#ifndef DRAIN_ENUM_FLAGGER
#define DRAIN_ENUM_FLAGGER


//#include <drain/Dictionary.h>
#include <drain/Enum.h>
#include "Flags.h"

namespace drain {




/// Flagger accepting values of enum type E.
/**
 *  \tparam F – SingleFlagger<E> or MultiFlagger<E>
 *
 *  Enum dictionary should be defined as follows:
 *
 *  template <>
 *  const drain::Enum<...>::dict_t  drain::Enum<...>::dict = {{"A", A}, {"B", B}, {"C", C}};
 *
 */
template <class F>  // F =SingleFlagger<E>
class EnumFlagger : public F {

public:

	typedef F fbase_t;
	typedef typename F::value_t value_t;
	typedef typename F::storage_t storage_t;
	typedef typename F::dict_t  dict_t;
	typedef FlagResolver::ivalue_t ivalue_t;

	/// Default constructor
	inline
	EnumFlagger(): fbase_t(Enum<value_t>::dict){
	}

	/// Constructor with initial value
	inline
	EnumFlagger(const storage_t & v): fbase_t(Enum<value_t>::dict) {
		this->value = v;
	}

	/*
	inline
	EnumFlagger(value_t v): fbase_t(v) {
	}
	*/

	///
	template <typename ... T>
	inline
	EnumFlagger(const T & ... arg): fbase_t(Enum<value_t>::dict, arg...) { // designed for MultiFlagger
	}

	/*  // keep
	virtual void reset() override {
		this->value = Enum<value_t>::defaultValue; // ALERT! enums need neutral value.
	};
	*/

	/// Returns the static dictionary created for this \c value_t .
	/**
	 *  Notice that this function is non-static but returns a reference to a static dictionary.
	 *  In C++ virtual functions must be non-static.
	 *  This is nevertheless handy in templated design, for example.
	virtual
	const dict_t & getDict() const override {
		return Enum<value_t>::dict;
	};
	 */

	/// Return the integer value corresponding to a key.
	/**
	 *  Retrieves the value directly from the dictionary.
	 *  Dictionary throws except if key now found.
	 */
	static
	ivalue_t getValueNEW(const std::string & key){
		return (ivalue_t)Enum<value_t>::dict.getValue(key);
	};

	// Raise?
	/*
	static inline
	std::string getKeysNEW2(const storage_t & value, char separator = ','){
		// currentStr = FlagResolver::getKeys(dict, this->value, this->separator);
		return FlagResolver::getKeys(Enum<value_t>::dict, value, separator);
	}
	*/

	/**
	 *  Dictionary throws except if key now found.
	static
	value_t getKeys(const value_t & value){
		return (value_t)Enum<value_t>::dict.getValue(key);
	};
    */

	/**
	 *  Consider handler through this->set(Flagger) ? Needs protected/private _set() (or assign()).
	 */
	inline
	EnumFlagger<F> & operator=(const EnumFlagger<F> & flagger){
		this->set(flagger.value);
		return *this;
	}

	template <class T>
	inline
	EnumFlagger<F> & operator=(const T & v){
		this->set(v);
		return *this;
	}

};



} // drain::



#endif
