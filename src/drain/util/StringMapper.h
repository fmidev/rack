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
#include "SmartMapTools.h"
#include "RegExp.h"
#include "Sprinter.h"
#include "String.h"
#include "Time.h"
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




template <class T>
class VariableHandler {

public:

	inline
	~VariableHandler(){};

	IosFormat iosFormat;

	/// Default
	/**
	 *  \return true if handles.
	 */
	/*
		typename std::map<std::string,T>::const_iterator it = variables.find(key);
		if (it != variables.end()){
			iosFormat.copyTo(ostr);
			//ostr.width(width);
			//std::cerr << __FILE__ << " assign -> " << stringlet << std::endl;
			//std::cerr << __FILE__ << " assign <---- " << mit->second << std::endl;
			ostr <<  it->second;
			return true;
		}
		else {
			return false;
		}
	 */

	/// Searches given key in a map, and if found, processes (formats) the value to ostream.  Return false, if variable not found.
	/**
	 *   Return false, if variable not found.
	 *   Then, further processors may handle the variable tag (remove, change, leave it).
	 */
	virtual
	bool handle(const std::string & key, const std::map<std::string,T> & variables, std::ostream & ostr) const {

		drain::Logger mout(__FILE__, __FUNCTION__);

		std::string k,format;
		drain::StringTools::split2(key, k, format, '|');
		// mout.attention("split '", key, "' to ", k, " + ", format);

		const typename std::map<std::string,T>::const_iterator it = variables.find(k);
		if (it == variables.end()) {
			return false;
		}


		if (format.empty()){
			iosFormat.copyTo(ostr);
			//vostr.width(width);
			//vstd::cerr << __FILE__ << " assign -> " << stringlet << std::endl;
			//vstd::cerr << __FILE__ << " assign <---- " << mit->second << std::endl;
			ostr <<  it->second;
			return true;
		}
		else {
    		// mout.attention("delegating '", k, "' to formatVariable: ", format);
			return formatVariable(k, variables, format, ostr);
		}

	}



	// NOTE: must return false, if not found. Then, further processors may handle the variable tag (remove, change, leave it).
	virtual
	bool formatVariable(const std::string & key, const std::map<std::string,T> & variables, const std::string & format, std::ostream & ostr) const {

		drain::Logger mout(__FILE__, __FUNCTION__);


		const char firstChar = format.at(0);
		const char lastChar = format.at(format.size()-1);

    	if (firstChar == ':'){

    		// mout.attention("substring extraction:", format);

    		std::string s;
    		drain::MapTools::get(variables, key, s);

    		std::vector<size_t> v;
    		drain::StringTools::split(format, v, ':');
    		size_t pos   = 0;
    		size_t count = s.size();

    		switch (v.size()) {
				case 3:
					count = v[2];
					// no break
				case 2:
					pos = v[1];
					if (pos >= s.size()){
						mout.warn("index ", pos, " greater than size (", s.size(), ") of string value '", s, "' of '", key, "'");
						return true;
					}
					count = std::min(count, s.size()-pos);
					ostr << s.substr(v[1], count);
					break;
				default:
					mout.warn("unsupported formatting '", format, "' for variable '", key, "'");
					mout.advice("use :startpos or :startpos:count for substring extraction");
			}
    		return true;

    	}
    	else if (firstChar == '%'){

    		// mout.attention("string formatting: ", format);

        	//else if (format.find('%') != std::string::npos){
    		std::string s;
    		drain::MapTools::get(variables, key, s);

    		const size_t BUFFER_SIZE = 256;
    		char buffer[BUFFER_SIZE];
    		buffer[0] = '\0';
    		size_t n = 0;

    		switch (lastChar){
    		case 's':
    			n = std::sprintf(buffer, format.c_str(), s.c_str());
    			break;
    		case 'c':
    			n = std::sprintf(buffer, format.c_str(), s.at(0)); // ?
    			break;
    		case 'p':
    			mout.unimplemented("pointer type: ", format);
    			break;
    		case 'f':
    		case 'F':
    		case 'e':
    		case 'E':
    		case 'a':
    		case 'A':
    		case 'g':
    		case 'G':
    		{
    			double d = NAN; //nand();
    			drain::MapTools::get(variables, key, d);
    			// ostr << d << '=';
    			n = std::sprintf(buffer, format.c_str(), d);
    		}
    		break;
    		case 'd':
    		case 'i':
    		case 'o':
    		case 'u':
    		case 'x':
    		case 'X':
    		{
    			int i = 0;
    			drain::MapTools::get(variables, key, i);
    			ostr << i << '=';
    			n = std::sprintf(buffer, format.c_str(), i);
    		}
    		break;
    		default:
    			mout.warn("formatting '", format, "' requested for '", key, "' : unsupported type key: ", lastChar);
    		}

    		ostr << buffer;
    		if (n > BUFFER_SIZE){
    			mout.fail("formatting with '", format, "' exceeded buffer size (", BUFFER_SIZE, ')');
    		}

    		// mout.warn("time formatting '", format, "' requested for '", k, "' not ending with 'time' or 'date'!");
    	}

    	return true;
	}


};




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
			//fillChar('0'),
		//((std::list<Stringlet> &)*this) = mapper;
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

	/// Converts a std::string containing variables like in "Hello, ${NAME}!" to a list of StringLet's.
	/**
	 *   The Stringlet list is internal.
	 *   \param s - string containing variables like in "Hello, ${NAME}!"
	 *   \param convertEscaped - convert backslash+letter segments to actual chars (\t, \n } first
	 */
	StringMapper & parse(const std::string &s, bool convertEscaped = false);

	/// Interpret commond special chars tab '\t' and newline '\n'.
	static
	std::string & convertEscaped(std::string &s);


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


	/// Expands the variables in the last
	/**
	 *  \par ostr - output stream
	 *  \par m    - map containing variable values
	 *  \par clear - if given, replace undefined variables with this char, or empty (if 0), else (-1) leave variable entry
	 */
	template <class T>
	std::ostream &  toStream(std::ostream & ostr, const std::map<std::string,T> & variables, int replace = 0, const VariableHandler<T> &handler = VariableHandler<T>()) const {

		for (const Stringlet & stringlet: *this){

			if (stringlet.isVariable()){

				if (handler.handle(stringlet, variables, ostr)){
					// std::cerr << __FILE__ << " ok stringlet variable: " << stringlet << std::endl;
					// ok, accepted and handled!
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
				else { // replace == 0
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

	std::string validChars;
	bool formatting;

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
