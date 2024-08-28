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

#ifndef DRAIN_KEY_MATCHER
#define DRAIN_KEY_MATCHER

#include <set>
#include <list>
#include <map>
#include <stdexcept>

#include <drain/Type.h>
#include <drain/RegExp.h>


namespace drain {

/// General-purpose key matcher.
/**
 *  The internal value can be a literal string or a regular expression.
 *
 *  When setting the value, it is recognized as a literal if it contains no special characters
 *
 *   against which equality of an input string is tested.
 *
 */
class KeyMatcher : protected drain::RegExp {

public:

	static
	const std::string regExpSpecialChars;

	/// Visible reference to the RegExp.
	const std::string & value;

	inline
	KeyMatcher(const std::string & s = "") : value(regExpString), isRegExp(false){
		set(s);
	}

	inline
	KeyMatcher(const KeyMatcher & matcher) : value(regExpString), isRegExp(false){
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
std::ostream & operator<<(std::ostream & ostr, const KeyMatcher & m){
	ostr << m.value;
	return ostr;
}

DRAIN_TYPENAME(KeyMatcher);

} // drain::

#endif
