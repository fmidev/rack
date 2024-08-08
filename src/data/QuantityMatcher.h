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

#ifndef QUANTITY_MATCHER_H_
#define QUANTITY_MATCHER_H_

#include <set>
#include <list>
#include <map>
#include <stdexcept>

#include <drain/RegExp.h>
/*
#include <drain/Sprinter.h>
#include <drain/util/BeanLike.h>
#include <drain/util/Range.h>
#include <drain/util/ReferenceMap.h>
//#include <drain/util/Variable.h>

#include "ODIM.h"
#include "ODIMPathTools.h"
#include "ODIMPathMatcher.h"
#include "PolarODIM.h" // elangle
*/

namespace rack {


/// General-purpose key matcher.
/**
 *  The internal value can be a literal string or a regular expression.
 *
 *  When setting the value, it is recognized as a literal if it contains no special characters
 *
 *   against which equality of an input string is tested.
 *
 */
class QuantityMatcher : protected drain::RegExp {

public:

	static
	const std::string regExpSpecialChars;

	/// Visible reference to the RegExp.
	const std::string & value;

	inline
	QuantityMatcher(const std::string & s = "") : value(regExpString), isRegExp(false){
		set(s);
	}

	inline
	QuantityMatcher(const QuantityMatcher & matcher) : value(regExpString), isRegExp(false){
		set(matcher.value);
	}

	/// Assign string which may be literal or a regular expression.
	/**
	 *
	 */
	void set(const std::string & s);

	inline
	bool operator==(const std::string &s) const {
		return test(s);
	}

	inline
	bool operator==(const char *s) const {
		return test(s);
	}

	/// Test with direct string matching or regExp, if defined.
	/**
	 *
	 */
	bool test(const std::string & s) const;

	const std::string & getType() const;

	/// Checks if the key conforms to ODIM convention: DBZH, VRAD, etc. (capital letters, underscores)
	/**
	 *  This is used for example by PaletteOp
	 *
	 *	static bool validateKey(const std::string & key);
	 *
	 */

protected:

	bool isRegExp;


};


inline
std::ostream & operator<<(std::ostream & ostr, const QuantityMatcher & m){
	ostr << m.value;
	return ostr;
}


} // rack::

#endif /* DATASELECTOR_H_ */
