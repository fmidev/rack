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
/*
 * StringTools.h
 *
 *  Created on: Jul 21, 2010
 *      Author: mpeura
 */

#ifndef DRAIN_STRING_H_
#define DRAIN_STRING_H_

#include <limits>
#include <iterator>
#include <string>
#include <sstream>
#include <iostream>
#include <list>
#include <map>

//#include "RegExp.h"

namespace drain {

class StringTools {

public:

	/// Turns n first characters uppercase. ASCII only.
	/**
	 */
	static
	std::string & upperCase(std::string & s, size_t n = std::numeric_limits<size_t>::max());

	/// Turns a character to uppercase. ASCII only.
	/**
	 */
	static
	char upperCase(char c);

	/// Turns n first characters lowercase. ASCII only.
	/**
	 *
	 */
	static
	std::string & lowerCase(std::string & s, size_t n = std::numeric_limits<size_t>::max());

	/// Turns a character to lowercase. ASCII only.
	/**
	 */
	static
	char lowerCase(char c);

	/// Replaces instances of 'from' to 'to' in src, storing the result in dst.
	/// In src, replaces instances of 'from' to 'to', returning the result.
	/** Safe. Uses temporary std::string.
	 *  \see RegExp::replace.
	 */
	static inline
	std::string replace(const std::string &src, const std::string &from, const std::string & to){
		std::string dst;
		StringTools::replace(src, from, to, dst);
		return dst;
	}

	/// Replaces instances appearing as map keys to map values.
	/**
	 *  \see RegExp::replace.
	 */
	static
	void replace(const std::map<std::string,std::string> & m, std::string &s, std::size_t pos = 0);


	/// In src, replaces instances of 'from' to 'to', storing the result in dst.
	static
	void replace(const std::string &src, const std::string &from, const std::string & to, std::string & dst);


	/// Returns a string without leading and trailing whitespace (or str undesired chars).
	static
	std::string trim(const std::string &s, const std::string &trimChars=" \t\n\r");

	/// Returns indices to trimmed segment of s such that it can be extracted as s.substr(pos1, pos2-pos1).
	/**
	 *  Starting from pos1, searches for the first segment not containing trimChars.
	 *
	 *  \param s    - string to be scanned
	 *  \param pos1 - before: index of 1st char to scan right, after: index of 1st char of the segment, if found; else s.size().
	 *  \param pos2 - before: index of 1st char to scan left, after: index of the char \b after the segment, or s.size().
	 *  \return - true if a non-empty segment found.
	 *
	 *  \see trim(const std::string &, const std::string &)
	 *
	 */
	static
	bool trimScan(const std::string &s, size_t & pos1, size_t & pos2, const std::string &trimChars=" \t\n");

	/// Splits and trims a given std::string to a std Sequence.
	/**
	 *  The target sequence must implement end() and insert() operators.
	 *
	 *   Assumes that T::operator=(std::string) exists.
	 *
	 *  Given an empty std::string, returns a list containing an empty std::string.
	 */
	template <class T>
	static
	void split(const std::string & s, T & sequence, const std::string &separators, const std::string & trimChars=" \t\n");

	/// Splits and trims a given std::string to a std Sequence.
	template <class T>
	static inline
	void split(const std::string & s, T & sequence, char separator=',', const std::string & trimChars=" \t\n"){
		StringTools::split(s, sequence, std::string(1,separator), trimChars);
	}

	/// Splits and trims a given std::string to two substrings, by first separator encountered.
	/**
	 *  Given an empty std::string,
	 */
	template <class T1, class T2>
	static
	bool split2(const std::string & s, T1 & first, T2 & second, const std::string &separators, const std::string & trimChars=" \t\n");

	/// Writes a STL Container (list, vector, set) to a stream, using an optional separator char (e.g. ',').
	/**
	 *  \see StringTools::split()
	 */
	template <class T>
	static
	std::ostream & join(const T & container, std::ostream & ostr, char separator = 0){
		char sep = 0;
		for (typename T::const_iterator it = container.begin(); it != container.end(); ++it){
			if (sep)
				ostr << sep;
			ostr << *it;
			sep = separator;
		}
		return ostr;
	}

	/// Writes a STL Container (list, vector, set) to a string, using an optional separator char (like ',').
	/**
	 *  \see StringTools::split()
	 */
	template <class T>
	static inline
	std::string join(T & container, char separator = 0){
		std::stringstream sstr;
		StringTools::join(container, sstr, separator);
		return sstr.str();
	}

	/// Read input stream and append it to s.
	template <unsigned int S>
	static
	void read(std::istream &istr, std::string & s){
	    char buffer[S];
	    while (istr.read(buffer, S)){
	        s.append(buffer, S);
	    }
	    s.append(buffer, istr.gcount());
	}

//private:

	/// Conversion from std::string to basic types.
	/**
	 *  \param str - input string
	 *  \param tmp - "hidden" temporary value; returned reference should be read instead of this.
	 *
	 *  \return - reference to the result of the conversion.
	 */
	template <class T>
	static
	const T & convert(const std::string &str, T & tmp);

	template <class T>
	static
	std::string & import(const T & src, std::string & target);


private:

	template <class T>
	static
	void appendString(T & sequence, const std::string & str){
		typename T::value_type tmp;
		sequence.insert(sequence.end(), StringTools::convert(str, tmp));
	}

	template <class T>
	static
	void appendSubstring(T & sequence, const std::string & str, std::string::size_type pos, std::string::size_type n){

		if (n > 0){
			appendString(sequence, str.substr(pos, n));
		}
		else {
			appendString(sequence, "");
		}

	}

};


template <class T>
void StringTools::split(const std::string & str, T & sequence, const std::string & separators, const std::string & trimChars){

	sequence.clear();

	const bool TRIM = !trimChars.empty();
	const std::string::size_type n = str.size();

	std::string::size_type pos1 = 0; // Start of segment
	std::string::size_type pos2 = n; // std::string::npos;  // End of segment (index of last char + 1)

	if (separators.empty()){ // = no split! :-)

		if (TRIM){
			StringTools::trimScan(str, pos1, pos2, trimChars);
			appendSubstring(sequence, str, pos1, pos2-pos1);
		}
		else
			appendString(sequence, str);

		return;
	}
	else {

		// Index of the next separator position
		std::string::size_type pos = pos1; // "irreversible"

		while (true) {

			pos1 = pos;
			pos  = str.find_first_of(separators, pos);
			if (pos == std::string::npos){
				pos2 = n;
				if (TRIM)
					StringTools::trimScan(str, pos1, pos2, trimChars);
				appendSubstring(sequence, str, pos1, pos2-pos1);
				return;
			}
			else {
				pos2 = pos;
				if (TRIM)
					StringTools::trimScan(str, pos1, pos2, trimChars);
				appendSubstring(sequence, str, pos1, pos2-pos1);
				++pos; // for the next round
			}

		}
	}
}


template <class T1, class T2>
bool StringTools::split2(const std::string & s, T1 & first, T2 & second, const std::string &separators, const std::string & trimChars){

	std::size_t i = s.find_first_of(separators);

	if (i != std::string::npos){ // input of type "key=value" found

		std::stringstream sstr1;
		//std::cerr << "eka " << s.substr(0, i) << '\n';
		if (!trimChars.empty()){
			sstr1 << StringTools::trim(s.substr(0, i), trimChars);
		}
		else
			sstr1 << s.substr(0, i);
		sstr1 >> first;

		std::stringstream sstr2;
		++i;
		if (i<s.size()){
			//std::cerr << "toka " << s.substr(i) << '\n';
			if (!trimChars.empty()){
				sstr2 << StringTools::trim(s.substr(i), trimChars);
			}
			else
				sstr2 << s.substr(i);
		}
		sstr2 >> second;
		return true;
	}
	else {
		std::stringstream sstr;
		if (trimChars.empty())
			sstr << s;
		else
			sstr << StringTools::trim(s, trimChars);
		sstr >> first;
		return false;
	}


}


template <>
inline
const std::string & StringTools::convert(const std::string &str, std::string & dst){
	//target.assign(str);
	return str;
}


template <class T>
inline
const T & StringTools::convert(const std::string &str, T & dst	){
	std::stringstream sstr(str);
	sstr >> dst;
	//std::cerr << __FUNCTION__ << ": " << str << '>' << target << '\n';
	return dst;
}

template <>
inline
std::string & StringTools::import(const std::string & src, std::string & dst){
	return dst;
}

template <class T>
std::string & StringTools::import(const T & x, std::string & dst){
	std::stringstream sstr;
	sstr << x;
	dst.assign(sstr.str());
	//std::cerr << __FUNCTION__ << ": " << str << '>' << target << '\n';
	return dst;
}


} // drain::




#endif /* STRING_H_ */

// Drain
