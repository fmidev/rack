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

#include <sstream>
#include <stdexcept>
#include <set>


#include "Type.h"

namespace drain
{

DRAIN_TYPENAME_DEF(void);
DRAIN_TYPENAME_DEF(bool);
DRAIN_TYPENAME_DEF(char);
DRAIN_TYPENAME_DEF(unsigned char);
DRAIN_TYPENAME_DEF(short);
DRAIN_TYPENAME_DEF(unsigned short);
DRAIN_TYPENAME_DEF(int);
DRAIN_TYPENAME_DEF(unsigned int);
DRAIN_TYPENAME_DEF(long);
DRAIN_TYPENAME_DEF(unsigned long);
DRAIN_TYPENAME_DEF(float);
DRAIN_TYPENAME_DEF(double);
DRAIN_TYPENAME_DEF(char *);
DRAIN_TYPENAME_DEF(const char *);
DRAIN_TYPENAME_DEF(std::string);

/*
const drain::RegExp Type::trueRegExp("^[ ]*true[ ]*$", REG_EXTENDED | REG_ICASE ); // ignore case

const drain::RegExp Type::falseRegExp("^[ ]*false[ ]*$", REG_EXTENDED | REG_ICASE ); // ignore case
*/

/**
 *
 *  TODO: manual+includes, TypeUtils, Skip escaped \\
 *
 *
 */

/**
    Testing string '-123.456e+789' yields

	0 =	 -123.456e+789
	1 = 	'-123'
	2 = 	'.456'
	3 = 	'e+789'

 */

/*
const std::type_info & Type::guessType(const std::string & value){


	// TODO: better whitespace
	static drain::RegExp numeralRegExp("^[ ]*([-+]?[0-9]*)(\\.[0-9]+)?([eE][-+]?[0-9]+)?[ ]*$");

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

*/




} // drain::
