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
 * String.h
 *
 *  Created on: Jul 21, 2010
 *      Author: mpeura
 */

#ifndef DRAIN_STRING_H_
#define DRAIN_STRING_H_

#include <string>
#include <list>
#include <iterator>
#include <limits>

#include "RegExp.h"

namespace drain {

class String {

public:

	/// Replaces instances of 'from' to 'to' in src, storing the result in dst.
	/** Safe. Uses temporary std::string.
	 *
	 */
	static std::string replace(const std::string &src, const std::string &from, const std::string & dst);

	/// Replaces std::string segment matching search regexp to the given std::string.
	/**
	 */
	static std::string replace_regexp(const std::string &src, RegExp & search,const std::string & dst);

	/// Replaces std::string segment matching search to the given std::string.
	inline
	static std::string replace_regexp(const std::string &src, const std::string &search, const std::string & dst){
		RegExp r(std::string("(^.*)?(") + search + std::string(")(.*$)?"));
		return replace_regexp(src, r, dst);
	};

	/**
	 *
	 *   Assumes that T::operator=(std::string) exists.
	 *   Given an empty std::string, returns a list containing an empty std::string.
	 *
	 */
	//template<class T>
	//static 	void split(const std::string &str, std::list<std::string> &l, const std::string &separators=",", const std::string & trimChars=" \t\n");

	template <class T>
	static
	void split(const std::string &str, T & sequence, const std::string &separators =",", const std::string & trimChars=" \t\n");

	static
	std::string trim(const std::string &s, const std::string &trimChars=" \t\n");

	/// Turns n first characters uppercase. Ascii only.
	/**
	 *
	 */
	static
	std::string & upperCase(std::string & s, size_t n = std::numeric_limits<size_t>::max());

	/// Turns n first characters lowercase. Ascii only.
	/**
	 *
	 */
	static
	std::string & lowerCase(std::string & s, size_t n = std::numeric_limits<size_t>::max());

};

//#include "String.cpp"

} // drain::




namespace drain {

template <class T>
void String::split(const std::string &str, T & sequence, const std::string &separators, const std::string & trimChars){

	const std::string::size_type n = str.size();
	const bool trimEach = trimChars.empty();

	sequence.clear();

	if (separators.empty()){
		//sequence.push_back(trim(str, trimChars));
		sequence.insert(sequence.end(), trim(str, trimChars));
		return;
	}

	std::string::size_type pos = 0;
	std::string::size_type pos2 = 0;

	do {

		pos2 = str.find_first_of(separators, pos);

		if (pos2 == std::string::npos)
			pos2 = n; // last

		// Pick std::string segment, notice +1 missing because separator
		if (trimEach)
			sequence.insert(sequence.end(), str.substr(pos, pos2 - pos)); // sequence.push_back(
		else
			sequence.insert(sequence.end(), trim(str.substr(pos, pos2 - pos), trimChars)); // sequence.push_back(

		pos = pos2 + 1;

	} while (pos2 != n);

}


} // drain::




#endif /* STRING_H_ */

// Drain
