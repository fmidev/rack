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

#include <cmath>
#include <stdexcept>

#include <drain/Type.h>
#include <drain/TypeUtils.h>

namespace drain {


// static
const drain::RegExp  TypeUtils::trueRegExp("^[ ]*true[ ]*$", REG_EXTENDED | REG_ICASE ); // ignore case

//static
const drain::RegExp TypeUtils::falseRegExp("^[ ]*false[ ]*$", REG_EXTENDED | REG_ICASE ); // ignore case

// TODO: better whitespace
// static
const drain::RegExp TypeUtils::numeralRegExp("^[ ]*([-+]?[0-9]*)(\\.[0-9]+)?([eE][-+]?[0-9]+)?[ ]*$");


const std::type_info & TypeUtils::guessType(const std::string & value){


	/**
	    Testing string '-123.456e+789' yields

		0 =	 -123.456e+789
		1 = 	'-123'
		2 = 	'.456'
		3 = 	'e+789'

	 */


	//static drain::RegExp booleanRegExp("^[ ]*(true|false)[ ]*$", REG_EXTENDED | REG_ICASE ); // ignore case

	if (numeralRegExp.execute(value)){ // true == REJECT

		if (trueRegExp.test(value) || falseRegExp.test(value)){ // "true" or "false", in any lower/upper case composition
			return typeid(bool);
		}
		else {
			return typeid(std::string);
		}

	}
	else if (numeralRegExp.result.size() == 2){
		return typeid(int);
	}
	else {
		return typeid(double);
	}

}


const std::type_info & TypeUtils::minimizeIntType(double value, const std::type_info & type){ // , const std::type_info & type = typeid(unsigned char)){

	// const bool IS_INTEGER = (d == ::trunc(d));

	if (value != ::trunc(value)){
		return typeid(double); // later, also float option?
	}

	//if (isWithinRange<T>(d)){
	if (Type::call<typeLimiter<double> >(type)(value) == value){
		return type;
	}

	// static const std::type_info & type = typeid(T);
	// Type::call<isSigned>(type)
	// Later, directly "inline" through type specification ?

	if (Type::call<isSigned>(type)){
		// throw std::runtime_error(StringBuilder<' '>(__FUNCTION__, "  requested signed type: ", TypeName<T>::str()));
		if (&type == &typeid(signed char)){
			//return minimizeIntType<signed short>(d);
			return minimizeIntType(value, typeid(signed short));
		}
		else if (&type == &typeid(signed short)){
			return minimizeIntType(value, typeid(signed int));
		}
		else if (&type == &typeid(signed int)){
			return minimizeIntType(value, typeid(signed long));
		}
	}
	else {
		if (&type == &typeid(unsigned char)){
			return minimizeIntType(value, typeid(unsigned short));
		}
		else if (&type == &typeid(unsigned short)){
			return minimizeIntType(value, typeid(unsigned int));
		}
		else if (&type == &typeid(unsigned int)){
			return minimizeIntType(value, typeid(unsigned long));
		}
	}

	return typeid(double);

}

class fitsIn {

public:

	typedef bool value_t;

	/**
	 *  \tparam S - type to be analyzed
	 *  \tparam T - destination type  (practically value_t)
	 */
	template <class S, class T>
	static inline
	T callback(){
		return static_cast<T>(isWithinRange<S>()); // static cast unneeded in these ?
	}

protected:

	// Must be here to get template<void> implemented
	template <class S>
	static inline
	size_t isWithinRange(){
		return sizeof(S);
	}

};





}

