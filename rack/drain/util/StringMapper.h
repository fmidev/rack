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

#ifndef STRINGMAPPER_H_
#define STRINGMAPPER_H_

#include <map>
#include <list>
#include <iterator>
#include <sstream>

#include "IosFormat.h"
#include "Log.h"
#include "RegExp.h"
#include "Sprinter.h"
#include "String.h"
#include "Variable.h"

namespace drain {

/// A helper class for StringMapper.
/**  Stringlet is a std::string that is used literally or as a variable, ie. a key pointing to a map.
 *   When a StringMapper parses a given std::string, it splits the std::string to segments containing
 *   literals and variables, in turn.
 *   The result is stored in a list.
 */
//template<class T>
// Todo: rename VariableMapper
class Stringlet: public std::string {
public:

	inline
	Stringlet(const std::string & s = "", bool isVariable = false) : std::string(s), isVar(isVariable) {
	};

	/// Copy constructor.
	Stringlet(const Stringlet & s) : std::string(s), isVar(s.isVar) {
	}

	inline
	bool isVariable() const { return isVar; };

	inline
	void setVariable(bool isVariable=true) { isVar = isVariable; };

	inline
	void setLiteral(const std::string &s) { assign(s); isVar = false; };

	// Consider!
	// But has to share variable syntax ${...} with string mapper(s), which recognizes it...
	// std::ostream & toStream(std::ostream & ostr, std::map<std::string, T> & environment, bool clearMissing=true){}

protected:

	bool isVar;

};


inline
std::ostream & operator<<(std::ostream & ostr, const Stringlet & s) {
	if (s.isVariable())
		return ostr << "${" << (const std::string &) s << "}";
	else
		return ostr << (const std::string &) s;
}

/**  Expands a std::string containing variables like "Hello, ${name}!" to a literal std::string.
 *   StringMapper parses a given std::string, it splits the std::string into segments containing
 *   literals and variables in turn.
 *   The result is stored in a list of Stringlet:s. The variables are provided to a StringMap
 *   by means of a std::map<std::string,T> .
 *
 *   \example StringMapper-example.cpp
 */
/// A tool for expanding variables embedded in a std::string to literals.
/** The input variables are provided as a map, which is allowed to change dynamically.
 *  The input std::string, issued with parse(), contains the variables as "$key" or "${key}", a
 *  format familiar in Unix shell.
 *
 */
class StringMapper : public std::list<Stringlet> {

public:

	/** Constructor.
	 *
	 */
	// TODO validKeys?
	StringMapper(
			const std::string & format = "",
			const std::string & validChars = "[a-zA-Z0-9_]+"
			)
//				fieldWidth(0),
//				fillChar('0')
	{
		setValidChars(validChars);
		regExp.setFlags(REG_EXTENDED);
		if (!format.empty())
			parse(format);
		//regExp.setFlags(REG_EXTENDED); // ORDER? Should be before parse!?
	};

	/// Initialize with the given RegExp
	StringMapper(const RegExp & regexp) : regExp(regexp)  { // fieldWidth(0),
			//fillChar('0'),
		//((std::list<Stringlet> &)*this) = mapper;
	}

	/// Copy constructor copies the parsed string and the regExp
	StringMapper(const StringMapper & mapper) : std::list<Stringlet>(mapper), regExp(mapper.regExp) { //  Buggy: regExp(mapper.regExp) {
	}


	inline
	StringMapper & setValidChars(const std::string & chars){
		std::stringstream sstr;
		sstr << "^(.*)\\$\\{(" << chars << ")\\}(.*)$";
		regExp.setExpression(sstr.str());
		return *this;
	}

	/// Converts a std::string containing variables like in "Hello, ${NAME}!" to a list of StringLet's.
	/**
	 *   The Stringlet list is internal.
	 *   \param s - string containing variables like in "Hello, ${NAME}!"
	 *   \param convertEscaped - convert backslash+letter segments to actual chars (\t, \n } first
	 */
	StringMapper & parse(const std::string &s, bool convertEscaped = false);

	/// Interpret commond special chars tab '\t' and newline '\n'.
	static
	std::string & convertEscaped(std::string &s){
		std::string s2;
		drain::StringTools::replace(s,  "\\t", "\t", s2);
		drain::StringTools::replace(s2, "\\n", "\n",  s);
		return s;
	}


	/// Return true, if all the elements are literal.
	bool isLiteral() const;

	/// Output a concatenated chain of stringlets: literals as such and variables surrounded with "${" and "}"
	/**
	 *  Prints the mapper in its current state, ie. some variables may have been expanded to literals.
	 *
	 *  \param ostr - output stream
	 */
	inline
	std::ostream &  toStream(std::ostream & ostr) const {
		SprinterBase::sequenceToStream(ostr, *this, SprinterBase::emptyLayout);
		return ostr;
	}

	IosFormat iosFormat;


	/// Expands the variables in the last
	/**
	 *  \par ostr - output stream
	 *  \par m    - map containing variable values
	 *  \par clear - if given, replace undefined variables with this char, or empty (if 0), else (-1) leave variable entry
	 */
	template <class T>
	//std::ostream &  toStream(std::ostream & ostr, const std::map<std::string,T> &m, bool keepUnknowns=false) const {
	std::ostream &  toStream(std::ostream & ostr, const std::map<std::string,T> &m, char replace = 0) const {

		for (const Stringlet & stringlet: *this){
			//const Stringlet & stringlet = *it;
			if (stringlet.isVariable()){
				// Find the entry in the map
				typename std::map<std::string, T >::const_iterator mit = m.find(stringlet);
				if (mit != m.end()){
					iosFormat.copyTo(ostr);
					//ostr.width(width);
					//std::cerr << __FILE__ << " assign -> " << stringlet << std::endl;
					//std::cerr << __FILE__ << " assign <---- " << mit->second << std::endl;
					ostr <<  mit->second;
				}
				else if (replace){
					//else if (keepUnknowns){ // = "recycle", add back "${variable}";
					if (replace < 0)
						ostr <<  stringlet; // is Variable -> use layout  "${variable}";
					else
						ostr << (char)replace;
					// if zero, skip silently (replace with empty string)
					/*
					if (replaceChar<0)
						ostr << '#' <<  *it << '$'; // is Variable -> use layout  "${variable}";
					else if (replaceChar>1)
						ostr << (char)replaceChar;
					*/
				}
				else {
					// Skip unknown (unfound) key
				}
			}
			else
				ostr << stringlet;
		};
		return ostr;
	}

	/// Expands the variables in the last parsed std::string to a std::string.
	/**
	 *  \par m     - map containing variable values
	 *  \par clear - if true, expand undefined variables as empty std::strings, else leave variable entry
	 */
	template <class T>
	std::string toStr(const std::map<std::string,T> &m, int replaceChar = -1) const {
		std::stringstream s;
		toStream(s, m, replaceChar);
		return s.str();
	}

	/// Expands the variables \em in StringMapper, turning expanded variables to constants.
	/**
	 *  \par m    - map containing variable values
	 *  \par clear - if true, replace undefined variables with empty std::strings.
	 */
	template <class T>
	void expand(const std::map<std::string,T> &m, bool clear=false) {
		for (StringMapper::iterator it = begin(); it != end(); it++){
			if (it->isVariable()){
				typename std::map<std::string, T >::const_iterator mit = m.find(*it);
				//std::cerr << __FUNCTION__ << " variable: " << *it << std::endl;
				if (mit != m.end()){
					//insert(it, Stringlet(Variable(mit->second).toStr()));
					it->setLiteral(Variable(mit->second).toStr());
					//it = erase(it);
				}
				else if (clear)
					it->setLiteral("");
					//it = erase(it);
			}
		};
	}


	/// Dumps the list of StringLet's
	template <class T>
	std::ostream &  debug(std::ostream & ostr, const std::map<std::string,T> &m ) const {
		ostr << "StringMapper '"<< "', RegExp='" << regExp << "', " <<  size() << " segments:\n";
		//StringMapper::const_iterator it;
		for (StringMapper::const_iterator it = begin(); it != end(); it++){
			//ostr << *it;
			ostr << '\t';
			if (it->isVariable()){
				//ostr << "VAR: ";
				typename std::map<std::string, T >::const_iterator mit = m.find(*it);
				if (mit != m.end())
					ostr << "\"" <<  mit->second << "\"";
				else
					ostr << *it;
			}
			else {
				ostr << "'"<<  *it << "'";
				//ostr << "LIT: '" << *it << "'\n";
			}
			ostr << '\n';
			//ostr << '\n';
		}
		return ostr;
	}


protected:


	StringMapper & parse(const std::string &s, RegExp &r);

	RegExp regExp;
	//  | REG_NEWLINE |  RE_DOT_NEWLINE); // | RE_DOT_NEWLINE); //  | REG_NEWLINE |  RE_DOT_NEWLINE

};

inline
std::ostream & operator<<(std::ostream & ostr, const StringMapper & strmap){
	return strmap.toStream(ostr);
}


} // NAMESPACE

#endif /* STRINGMAPPER_H_ */

// Drain
