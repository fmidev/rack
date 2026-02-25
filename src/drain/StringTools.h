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

#ifndef DRAIN_STRING_TOOLS
#define DRAIN_STRING_TOOLS

#include <limits>
#include <map>
#include <string>
#include <sstream>

/*
#include <iterator>
#include <iostream>
#include <list>
*/

// #include "Converter.h"


namespace drain {

/*
template <typename T>
class StringConverter {
public:

	static
	void convertToString(const T & value, std::string &s){
		std::stringstream sstr;
		sstr << value;
		s.assign(sstr.str());
	};

	static
	void convertFromString(const std::string &s, T & value){
		std::stringstream sstr(s);
		sstr >> value;
	};


};
*/

/*
template <typename T=std::string>
class StringWrapper : public std::string { // , protected StringConverter<T> {

public:

	inline
	StringWrapper(){
	};

	/// All the other constructors, including default constructor.
	//template <typename T>
	inline
	StringWrapper(const T & x){
		set(x);
	};


	inline
	void set(const std::string & s=""){
		std::string::assign(s);
	};

	inline
	void set(const StringWrapper & e){
		std::string::assign(e);
	};

	inline
	void set(const char *s){
		std::string::assign(s);
	};


	/// Set the value from an other, user-defined dictionary.
	template <typename T2>
	inline
	void set(const T2 & x){
		Converter<T2>::convert(x, *this);
		//StringConverter<T>::convertToString(x, *this);
	};

	inline
	bool operator==(const T & x){
		std::string s;
		Converter<T>::convert(x, s);
		// StringConverter<T>::convertToString(x, s);
		return *this == s;
	}




};
*/


class StringTools {

public:

	static
	bool startsWith(const std::string &s, const std::string & substring);


	static
	bool endsWith(const std::string &s, const std::string & substring);


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



	/// Replaces instances of 'search' to 'repl' in src
	/**
	 *  \see RegExp::replace.
	 */
	static
	void replace(const std::string &src, char search, char repl, std::ostream & ostr);

	/// In src, replaces instances of \c 'search' to \c 'repl', storing the result in dst.
	/**
	 *
	 */
	static
	void replace(const std::string &src, char from, char repl, std::string &dst);

	/// In src, replaces instances of 'search' to 'repl', storing the result in dst.
	/**
	 *
	 */
	template <typename S, typename R>
	static
	void replace(const std::string &src, const S & search, const R & repl, std::ostream & ostr){

		const size_t length = getLength(search);
		std::string::size_type i = 0;
		std::string::size_type pos;

		while (true) {
			pos = src.find(search, i);
			if (pos == std::string::npos){
				ostr << src.substr(i);
				return;
			}

			// String was found
			ostr << src.substr(i, pos-i) << repl;

			// Jump over <search>: move forward len(search) chars
			i = pos + length;
		}
	}


	/// In src, replaces instances of 'from' to 'to', storing the result in dst.
	/**
	 *
	 */
	template <typename T1, typename T2>
	static inline
	void replace(const std::string &src, const T1 &search, const T2 & repl, std::string & dst){
		std::stringstream result;
		replace(src, search, repl, result);
		dst = result.str();
	}


	/// Convenience using copying of string.
	/**
	 *  Replaces a char or a substring.
	 *  Inconsistent design: other methods return void
	 *
	 *  \return result
	 */
	/*
	template <typename T1, typename T2>
	static inline
	std::string replace(const std::string &src, const T1 & search, const T2 & repl){
		std::string dst;
		replace(src, search, repl, dst);
		return dst;
	}
	*/

	// MAP VERSIONS

	/// NEW Fast implementation of char replace.
	template <typename T>
	static
	void replace(const std::string & src, const std::map<char,T> & m, std::ostream & ostr){
		typename std::map<char,T>::const_iterator it;
		for (char c: src){
			it = m.find(c);
			if (it == m.end()){
				ostr << c;
			}
			else {
				ostr << it->second;
			}
		}
	};

	template <typename T>
	static inline
	void replace(const std::string & src, const std::map<char,T> & m, std::string & dst){
		std::stringstream result;
		replace(src, m, result);
		dst = result.str();
	}

	/// NEW Fast (in-place) implementation of char-to-char replace.
	/**
	 *
	 */
	static
	void replace(const std::string & src, const std::map<char,char> & m, std::string & dst);

	/**
	 *  Implementation for all such results than can have length different than source.
	 *  That is, from-set and/or to-set are strings.
	 *
	 */
	template <class K, class V>
	static inline
	void replace(const std::string & src, const std::map<K,V> & m, std::string & dst){
		replaceWithMap(src, m, dst);
	};

	template <class K, class V>
	static inline
	void replace(const std::string & src, const std::initializer_list<std::pair<K,V> > & m, std::string & dst){
		replaceWithMap(src, m, dst);
	};

	/// Interpret commond special chars: tab '\t', newline '\n' and carriage return '\r'.
	static
	std::string & convertEscaped(std::string &s);


private:

	static inline
	size_t getLength(char c){
		return 1;
	};

	static inline
	size_t getLength(const std::string & s){
		return s.length();
	};

	template <class M>
	static inline
	void replaceWithMap(const std::string & src, const M & m, std::string & dst){

		if (m.empty()){
			// Replace nothing in src, hence assign it directly to result.
			if (&dst != &src){
				dst = src; // clumsy?
			}
			return;
		}

		for (const auto & entry: m){
			// std::cerr << "replacing " << entry.first << " -> " << entry.second << " in " << dst << std::endl;
			replace(dst, entry.first, entry.second, dst);
		}
	};


public:



	/// Returns a string without leading and trailing whitespace (or str undesired chars).
	static
	std::string trim(const std::string &s, const std::string &trimChars=" \t\n\r");

	/// Coupled trimming: remove a single leading and trailing char, if both found.
	/**
	 *   \code
	 *   s = trim("\"test\"");
	 *   s = trim("{test}", "{", "}");
	 *   \endcode
	 *
	 */
	static
	std::string trimSymmetric(const std::string &s, const std::string &leading= "'\"", const std::string & trailing="");


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
	bool trimScan(const std::string &s, std::size_t & pos1, std::size_t & pos2, const std::string &trimChars=" \t\n");

	/// Splits and trims a given std::string to a std Sequence.
	/**
	 *
	 * \param s – string to be chopped
	 * \param sequence –  target sequence; must implement end() and insert() operators.
	 * \param separators – single char, array or string
	 * \param trimChars – single char, array or string
	 *
	 * Assumes that T::operator=(std::string) exists.
	 *
	 * Given an empty std::string, returns a sequence containing an empty std::string. Hence, does not return an empty sequence.
	 *
	 * Given a null or empty separator, returns a single-element sequence containing the original string.
	 */
	template <class T, class C>
	static
	void split(const std::string & s, T & sequence, const C &separators, const std::string & trimChars=" \t\n");

	/// Splits and trims a given std::string to a std Sequence.
	/*
	template <class T>
	static inline
	void split(const std::string & s, T & sequence, char separator=',', const std::string & trimChars=" \t\n"){
		StringTools::split(s, sequence, std::string(1,separator), trimChars);
	}
	*/

	/// Splits and trims a given std::string to two substrings, by first separator encountered.
	/**
	 *
	 *  \tparam T1 - target of 1st part ("key")
	 *  \tparam T2 - target of 1st part ("value")
	 *  \tparam C - character or string (any char in which will split)
	 *  \param s - source string
	 *  \param first - target of 1st part, typically the "key"
	 *  \param second - target of 2nd part, typically the "object"
	 *
	 *  Given an empty std::string, returns empty (intact) destinations.
	 *  Given a string that does contain a separator, assigns the string on \c first , leaving \c empty/intact.
	 *
	 *  Note: the classes of target objects \c first and \c second should have an
	 *  assignment operator=() that does not clear the object when assigning itself obj=obj !
	 */
	template <class T1, class T2, class C>
	static
	bool split2(const std::string & s, T1 & first, T2 & second, const C &separators, const std::string & trimChars=" \t\n");

	/// Determine the length of a common leading part, rounding up the result with step.
	/**
	 *   \param src1 - input string
	 *   \param src2 - input string
	 *   \param step - minimum size difference (consider hex strings, and step=2 or step=4)
	 *   \return - index of the first deviating character, ie. length of the common part, rounded by \c step
	 */
	static
	size_t extractPrefixLength(const std::string & src1, const std::string & src2, size_t step = 1);

	/// Extract common leading part, truncating the source strings
	/**
	 *   \param src1 - input string
	 *   \param src2 - input string
	 *   \param prefix - common leading part of src1 and src2
	 *   \param dst1 - src1 without prefix
	 *   \param dst2 - src2 without prefix
	 *   \return - index of the first deviating character, ie. length of the common part, rounded by \c step
	 */
	static
	size_t extractPrefix(const std::string & src1, const std::string & src2,
			std::string & prefix, std::string & dst1, std::string & dst2, size_t step = 1);

	/// Extract common leading part, truncating the input strings
	/**
	 *   \param src1 - input string
	 *   \param src2 - input string
	 *   \param prefix - common leading part of src1 and src2
	 *   \return - index of the first deviating character, ie. length of the common part.
	 */
	static inline
	size_t extractPrefix(std::string & s1, std::string & s2, std::string & prefix, size_t step = 1){
		return extractPrefix(s1, s2, prefix, s1, s2, step);
	}


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


	/// Conversion from std::string to basic types, including std::string.
	/**
	 *  This version is slow for string targets (T).
	 *
	 *  \param str - input string
	 *  \param dst - destination
	 *
	 *  \return - reference to the result of the conversion.
	 */
	template <class T>
	static
	void convert(const std::string &s, T & dst);

	/// Conversion from std::string to basic types, including std::string.
	/**
	 *  This version is slow for string targets (T).
	 *
	 *  \param str - input string
	 *  \return - converted value.
	 */
	template <class T>
	static
	T convert(const std::string &s);


	template <class T>
	static
	std::string & import(const T & src, std::string & target);




//private: CONSIDER!

	/// Convert only if needed. If T is std::string, returns s directly.
	/**
	 *  \param str - input string
	 *  \param tmp - "hidden" temporary value; returned reference should be read instead of this.
	 *  \tparam - string or any class that does not clear the object upon identity assigment (obj=obj).
	 *  \return - reference to the result of the conversion.
	 *
	 *
	 * Note: the classes of target objects \c first and \c second should have an
	 * assignment operator=() that does not clear the object when assigning itself obj=obj !
	 */
	template <class T>
	static
	const T & lazyConvert(const std::string &s, T & tmp);

private:

	template <class T>
	static
	void appendString(T & sequence, const std::string & str){
		typename T::value_type tmp;
		sequence.insert(sequence.end(), StringTools::lazyConvert(str, tmp));
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


template <class T, class C>
void StringTools::split(const std::string & str, T & sequence, const C & separators, const std::string & trimChars){

	sequence.clear();

	const bool TRIM = !trimChars.empty();
	const std::string::size_type n = str.size();

	std::string::size_type pos1 = 0; // Start of segment
	std::string::size_type pos2 = n; // std::string::npos;  // End of segment (index of last char + 1)

	if (false){ //separators.empty()){ // = no split! :-)

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
				pos  = str.find_first_not_of(trimChars, pos+1); // if separator is space, or other also in trimChars
				//pos  = str.find_last_not_of(trimChars, pos);
				//++pos; // for the next round
			}

		}
	}
}

/*
template <>
bool StringTools::split2(const std::string & s, std::string & first, std::string & second, const char *separators, const std::string & trimChars);
*/

template <class T1, class T2, class S>
bool StringTools::split2(const std::string & s, T1 & first, T2 & second, const S & separators, const std::string & trimChars){

	std::size_t i = s.find_first_of(separators);

	if (i != std::string::npos){ // input of type "key=value" found

		std::string srcFirst(s, 0, i);
		++i;
		std::string srcSecond(s, std::min(s.size(), i));

		if (!trimChars.empty()){
			first = lazyConvert(StringTools::trim(srcFirst, trimChars), first); // first=first should not clear the object!
		}
		else {
			first = lazyConvert(srcFirst, first); // Note: first=first should not clear the object!
		}

		if (!srcSecond.empty()){
			if (!trimChars.empty()){
				second = lazyConvert(StringTools::trim(srcSecond), second);
			}
			else {
				second = lazyConvert(srcSecond, second);
			}
		}

		/*
		++i;
		if (i<s.size()){
			//std::cerr << "toka " << s.substr(i) << '\n';
			if (!trimChars.empty()){
				// sstr2 << StringTools::trim(s.substr(i), trimChars);
				// sstr2 >> second;
				second = lazyConvert(StringTools::trim(s.substr(i), trimChars), second);
			}
			else {
				// sstr2 << s.substr(i);
				// sstr2 >> second;
				second = lazyConvert(s.substr(i), second);
			}
		}
		*/
		return true;
	}
	else {
		// std::stringstream sstr;
		if (trimChars.empty()){
			//sstr << s;
			//sstr >> first;
			first = lazyConvert(s, first);
		}
		else {
			// sstr << StringTools::trim(s, trimChars);
			// sstr >> first;
			first = lazyConvert(StringTools::trim(s, trimChars), first);
		}
		return false;
	}


}







template <>
inline
const std::string & StringTools::lazyConvert(const std::string &str, std::string & tmp){
	return str;
}


template <class T>
inline
const T & StringTools::lazyConvert(const std::string &str, T & dst){
	std::stringstream sstr(str);
	sstr >> dst;
	return dst;
}

template <class T>
inline
void StringTools::convert(const std::string &str, T & dst){
	dst = lazyConvert(str, dst);
}


template <class T>
inline
T StringTools::convert(const std::string &s){
	T tmp;
	tmp = lazyConvert(s, tmp);
	return tmp;
}




template <>
inline
std::string & StringTools::import(const std::string & src, std::string & dst){
	dst = src;
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

