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

//#include <algorithm>
//#include <syslog.h>  // levels: LOG_ERROR etc.

#include <drain/RegExp.h>
#include <drain/Sprinter.h>
#include <drain/Type.h>

#include "KeyMatcher.h"

namespace drain {

const std::string KeyMatcher::regExpSpecialChars = "^.?*[]()$";


void KeyMatcher::set(const std::string & s){
	reset();
	isRegExp = false;
	if (s.empty()){
		return;
	} // "^.?*[]()$"
	else if (s.find_first_of(regExpSpecialChars) != std::string::npos){
		isRegExp = true;
		setExpression(s);
	}
	else {
		isRegExp = false;
		regExpString = s;
	}
}

const std::string & KeyMatcher::getType() const {
	//const std::string re("regExp");
	//const std::ring str("string");
	if (isRegExp)
		return drain::TypeName<drain::RegExp>::str();
	else
		return drain::TypeName<std::string>::str();
}


bool KeyMatcher::test(const std::string & s) const {
	if (isRegExp){
		return RegExp::test(s);
	}
	else {
		return value == s; // string comparison
	}
}

/*
bool QuantityMatcher::validateKey(const std::string & key){
	static const drain::RegExp re("^[A-Z]+[A-Z0-9_\\-]*$");
	return re.test(key);
}
*/
DRAIN_TYPENAME_DEF(KeyMatcher);

}  // rack::
