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
#include <sstream>

#include <drain/Enum.h>
#include <drain/RegExp.h>
#include <drain/Sprinter.h>

#include "IosFormat.h"
#include "Time.h"
#include "VariableFormatter.h"

namespace drain {

/// A helper class for StringMapper.
/**  Stringlet is a std::string that is used literally or as a variable, ie. a key pointing to a map.
 *   When a StringMapper parses a given std::string, it splits the std::string to segments containing
 *   literals and variables, in turn.
 *   The result is stored in a list.
 */
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






/// Expands a std::string containing variables like "Hello, ${name}!" to a literal std::string.
/**
 *  A tool for expanding variables embedded in a std::string to literals.
 *  The input variables are provided as a map, which is allowed to change dynamically.
 *  The input std::string, issued with parse(), contains the variables as "$key" or "${key}", a
 *  format familiar in Unix shell.
 *
 *  StringMapper parses a given std::string, it splits the std::string into segments containing
 *  literals and variables in turn.
 *  The result is stored in a list of Stringlet:s. The variables are provided to a StringMap
 *  by means of a std::map<std::string,T> .
 *
 *  \example StringMapper-example.cpp
 *
 */
class StringMapper : public std::list<Stringlet> {

public:

	/// Default constructor.
	/**
	 * \param format - string containing variables like \c ${name}
	 * \param validChars - allowed characters in variable names, ie inside braces.
	 * \param formatting - support postprocessing of variable value (number rounding, string operations)
	 *
	 */
	StringMapper(
			const std::string & format = "",
			const std::string & validChars = "[a-zA-Z0-9_]+",
			bool formatting=true
			): formatting(formatting)
	{
		setValidChars(validChars);
		regExp.setFlags(REG_EXTENDED);
		if (!format.empty())
			parse(format);
		//regExp.setFlags(REG_EXTENDED); // ORDER? Should be before parse!?
	};

	/// Initialize with the given RegExp // REMOVE!
	StringMapper(const RegExp & regexp, bool formatting=true) : formatting(formatting), regExp(regexp)  { // fieldWidth(0),
		// fillChar('0'),
		// ((std::list<Stringlet> &)*this) = mapper;
	}

	/// Copy constructor copies the parsed string and the regExp
	StringMapper(const StringMapper & mapper) : std::list<Stringlet>(mapper), formatting(mapper.formatting), regExp(mapper.regExp) { //  Buggy: regExp(mapper.regExp) {
	}


	inline
	StringMapper & setValidChars(const std::string & chars){
		this->validChars = chars;
		/*
		std::stringstream sstr;
		sstr << "^(.*)\\$\\{(" << chars << ")\\}(.*)$";
		regExp.setExpression(sstr.str());
		*/
		updateRegExp();
		return *this;
	}

	/// Enable variable formatting, followed by pipe '|'.
	/**
	 *  The formatting depends on the implementation. In this base class, VariableFormatter<T> is applied.
	 *
	 */
	inline
	StringMapper & enableFormatting(bool formatting){
		if (this->formatting != formatting){
			this->formatting = formatting;
			updateRegExp();
		}
		else {
			this->formatting = formatting;
		}
		return *this;
	}



	/// Converts a std::string containing variables like in "Hello, ${NAME}!" to a list of StringLet's.
	/**
	 *   The Stringlet list is internal.
	 *   \param s - string containing variables like in "Hello, ${NAME}!"
	 *   \param convertEscaped - convert backslash+letter segments to actual chars (\t, \n } first
	 */
	StringMapper & parse(const std::string &s, bool convertEscaped = false);

	/*
	/// Interpret commond special chars tab '\t' and newline '\n'.
	static
	std::string & convertEscaped(std::string &s);
	*/

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
		Sprinter::sequenceToStream(ostr, *this, Sprinter::emptyLayout);
		return ostr;
	}

	IosFormat iosFormat;

	// static const int KEEP_MISSING_VARIABLE;
	// static const int REMOVE_MISSING_VARIABLE;
	enum handleMissing {
		REMOVE_MISSING_VARIABLE, //=0,
		KEEP_MISSING_VARIABLE // =1,
	};

	/// Expands the variables in the last
	/**
	 *  \par ostr - output stream
	 *  \par m    - map containing variable values
	 *  \par replace - if given, replace undefined variables with this char, or empty (if 0), else (-1) leave variable entry
	 */
	template <class T>
	std::ostream &  toStream(std::ostream & ostr, const std::map<std::string,T> & variables, int replace = REMOVE_MISSING_VARIABLE,
			const VariableFormatter<T> &formatter = VariableFormatter<T>()) const {

		for (const Stringlet & stringlet: *this){

			if (stringlet.isVariable()){

				if (formatter.handle(stringlet, variables, ostr)){
					// std::cerr << __FILE__ << " ok stringlet variable: " << stringlet << std::endl;
					// Variable found and assigned.
				}
				else if (replace == KEEP_MISSING_VARIABLE){ // (replace < 0)
					ostr <<  stringlet; // is Variable -> use layout  "${variable}";
				}
				else if (replace != REMOVE_MISSING_VARIABLE){
					ostr << (char)replace;
				}
				// else: silently skip the variable.
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
	std::string toStr(const std::map<std::string,T> &m, int replaceChar = -1, const VariableFormatter<T> & formatter = VariableFormatter<T>()) const {
		std::stringstream s;
		toStream(s, m, replaceChar, formatter);
		return s.str();
	}

	/// Expands the variables \em in StringMapper, turning expanded variables to constants.
	/**
	 *  \par m    - map containing variable values
	 *  \par clear - if true, replace undefined variables with empty std::strings.
	 */
	template <class T>
	void expand(const std::map<std::string,T> &m, const VariableFormatter<T> & formatter=VariableFormatter<T>(), bool clear=false) {
		for (auto & entry: *this){
			if (entry.isVariable()){
				std::stringstream s;
				if (formatter.handle(entry, m, s)){
					entry.setLiteral(s.str());
				}
				else if (clear) {
					entry.setLiteral("");
				}
			}
		};
	}


	/// Dumps the list of StringLet's
	template <class T>
	std::ostream &  debug(std::ostream & ostr, const std::map<std::string,T> &m ) const {

		const VariableFormatter<T> formatter;

		ostr << "StringMapper '"<< "', RegExp='" << regExp << "', " <<  size() << " segments:\n";
		//StringMapper::const_iterator it;
		for (const auto & entry: *this){
			ostr << "  ";
			if (entry.isVariable()){
				ostr  << entry << "=";
				if (!formatter.handle(entry, m, ostr)){
					ostr << "???";
				}
			}
			else {
				// Literal
				ostr << "'" << entry << "'";
			}
			ostr << '\n';
		};

		return ostr;
	}


protected:

	inline
	void updateRegExp(){
		std::stringstream sstr;
		sstr << "^(.*)\\$\\{(" << validChars; //  << ")\\}(.*)$";
		if (formatting){
			 sstr << "(\\|[^}]*)?";
		}
		sstr << ")\\}(.*)$";
		// std::cout << sstr.str() << '\n';
		regExp.setExpression(sstr.str());
	}

	StringMapper & parse(const std::string &s, RegExp &r);

	std::string validChars;
	bool formatting;

	RegExp regExp;
	//  | REG_NEWLINE |  RE_DOT_NEWLINE); // | RE_DOT_NEWLINE); //  | REG_NEWLINE |  RE_DOT_NEWLINE

};

inline
std::ostream & operator<<(std::ostream & ostr, const StringMapper & strmap){
	return strmap.toStream(ostr);
}

// DRAIN_ENUM_DICT(StringMapper::KEEP_MISSING_VARIABLE);
// template <> const drain::Enum<enumtype,owner>::dict_t drain::Enum<enumtype,owner>::dict
// template <>
// const drain::Enum<int,StringMapper>::dict_t drain::Enum<int,StringMapper>::dict;
DRAIN_TYPENAME(StringMapper::handleMissing);

DRAIN_ENUM_DICT(StringMapper::handleMissing);
// DRAIN_ENUM_OSTREAM(StringMapper::handleMissing);


} // NAMESPACE

#endif /* STRINGMAPPER_H_ */

// Drain
