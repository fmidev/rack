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




template <>
const std::string TypeName<bool>::name("bool");

template <>
const std::string TypeName<char>::name("char");

template <>
const std::string TypeName<unsigned char>::name("unsigned-char");

template <>
const std::string TypeName<short>::name("short");

template <>
const std::string TypeName<unsigned short>::name("unsigned-short");

template <>
const std::string TypeName<int>::name("int");

template <>
const std::string TypeName<unsigned int>::name("unsigned-int");

template <>
const std::string TypeName<long>::name("long");

template <>
const std::string TypeName<unsigned long>:: name("unsigned-long");

template <>
const std::string TypeName<float>:: name("float");

template <>
const std::string TypeName<double>:: name("double");

template <>
const std::string TypeName<char *>:: name("char *");

template <>
const std::string TypeName<const char *>:: name("const char *");

template <>
const std::string TypeName<std::string>:: name("string");


}
