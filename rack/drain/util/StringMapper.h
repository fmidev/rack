/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

    Author: mpeura
 */

#ifndef STRINGMAPPER_H_
#define STRINGMAPPER_H_

#include <map>
#include <list>
#include <iterator>
#include <sstream>
#include "RegExp.h"
//#include "Data.h"  // to convert base types in map entries
#include "Variable.h"  // to convert base types in map entries

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

	//Stringlet( const std::map<std::string, T> & m, const std::string & s = "", bool isVariable = false)
	inline
	Stringlet(const std::string & s = "", bool isVariable = false) : std::string(s), isVar(isVariable) { //, _map(m) {
		//assign(s);
	};

	inline
	bool isVariable() const { return isVar; };

	inline
	void setVariable(bool isVariable=true) { isVar = isVariable; };

	inline
	void setLiteral(const std::string &s) { assign(s); isVar = false; };


protected:

	//const
	bool isVar;
	//bool literal;
	//const std::map<std::string, T> & _map;
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
	StringMapper(const std::string & validKeys = "[a-zA-Z0-9_]+",
			const std::string & variablePrefix = "${", const std::string & variableSuffix = "}") :
				width(0), fill('0'),
				variablePrefix(variablePrefix), variableSuffix(variableSuffix), validChars(validKeys)
				 {};

	/// Converts a std::string containing variables like in "Hello, ${NAME}!" to a list of StringLet's.
	/**
	 *   The Stringlet list is internal.
	 */
	void parse(const std::string &s);

	/// Prints the mapper in its current state, ie. some variables may have been expanded to literals.
	/**
	 *  \par ostr - output stream
	 *  \par m    - map containing variable values
	 *  \par clear - if true, expand undefined variables as empty std::strings, else leave variable entry
	 */
	inline
	std::ostream &  toStream(std::ostream & ostr) const {
		for (StringMapper::const_iterator it = begin(); it != end(); it++){
			if (it->isVariable())
				ostr << variablePrefix << *it << variableSuffix;
			else
				ostr << *it;
		};
		return ostr;
	}


	/// Expands the variables in the last parsed std::string std::string.
	/**
	 *  \par ostr - output stream
	 *  \par m    - map containing variable values
	 *  \par clear - if true, expand undefined variables as empty std::strings, else leave variable entry
	 */
	template <class T>
	std::ostream &  toStream(std::ostream & ostr, const std::map<std::string,T> &m, bool clear = false) const {
		ostr.fill(fill);
		for (StringMapper::const_iterator it = begin(); it != end(); it++){
			if (it->isVariable()){
				typename std::map<std::string, T >::const_iterator mit = m.find(*it);
				if (mit != m.end()){
					ostr.width(width);
					ostr <<  mit->second;
				}
				else if (!clear)
					ostr << variablePrefix << *it << variableSuffix;
			}
			else
				ostr << *it;
		};
		return ostr;
	}

	/// Expands the variables in the last parsed std::string to a std::string.
	/**
	 *  \par m     - map containing variable values
	 *  \par clear - if true, expand undefined variables as empty std::strings, else leave variable entry
	 */
	template <class T>
	std::string toStr(const std::map<std::string,T> &m, bool clear = false) const {
		std::stringstream s;
		toStream(s, m, clear);
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
		ostr << "StringMapper '"<< "', validChars='" << validChars << "', " <<  size() << " segments:\n";
		//StringMapper::const_iterator it;
		for (StringMapper::const_iterator it = begin(); it != end(); it++){
			//ostr << *it;
			if (it->isVariable()){
				//ostr << "VAR: ";
				ostr << '\t';
				typename std::map<std::string, T >::const_iterator mit = m.find(*it);
				if (mit != m.end())
					ostr << '*' <<  mit->second << '*';
				else
					ostr << '<' << *it << '>';
				ostr << '\n';
			}
			else {
				ostr << "\t'" << *it << "'\n";
				//ostr << "LIT: '" << *it << "'\n";
			}
			//ostr << '\n';
		}
		return ostr;
	}

	std::streamsize width;
	char fill;

	/// Prefix used, when "preserving" a variable, ie. when variable is not found in the map but
	const std::string variablePrefix;
	/// Prefix used, when "preserving" a variable, ie. when variable is not found in the map but
	const std::string variableSuffix;

protected:

	const std::string validChars;
	void parse(const std::string &s, RegExp &r);

};

inline
std::ostream & operator<<(std::ostream & ostr, const StringMapper & strmap){
	return strmap.toStream(ostr);
}


} // NAMESPACE

#endif /* STRINGMAPPER_H_ */
