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

#include "Log.h"
#include "RegExp.h"

#include "Type.h"

namespace drain
{


/**
 *
 *  TODO: manual+includes, TypeUtils, Skip escaped \\
 *
 *
 */

const std::type_info & Type::guessType(const std::string & value){

	/**
	    Testing string '-123.456e+789' yields

		0 =	 -123.456e+789
		1 = 	'-123'
		2 = 	'.456'
		3 = 	'e+789'

	 */
	static drain::RegExp numeralRegExp("^[ ]*([-+]?[0-9]*)(\\.[0-9]+)?([eE][-+]?[0-9]+)?[ ]*$");

	if (numeralRegExp.execute(value)){ // true == REJECT
		return typeid(std::string);
	}
	else if (numeralRegExp.result.size() == 2){
		return typeid(int);
	}
	else {
		return typeid(double);
	}

}

const std::type_info & Type::guessArrayType(const std::list<std::string> & l){


	typedef std::set<const std::type_info *> typeset;

	typeset s;
	for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); ++it) {
		s.insert(& guessType(*it));
	}

	if (s.find(& typeid(std::string)) != s.end())
		return typeid(std::string);

	if (s.find(& typeid(double)) != s.end())
		return typeid(double);

	if (s.find(& typeid(int)) != s.end())
		return typeid(int);

	return typeid(std::string);
}



} // drain::
