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
#include "String.h"


#include <stdexcept>
#include <iostream>

namespace drain {

/*
#ifdef DRAIN_STRING_H_

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

#endif
*/


std::string String::replace(const std::string &src, const std::string &search, const std::string &to) { //,std::string &dst){

	std::string result;

	std::string::size_type i = 0;
	std::string::size_type pos;

	while (true) {
		pos = src.find(search,i);
		if (pos == std::string::npos){
			result.append(src,i,src.size()-i);
			//std::cerr << result << '\n';
			return result;
			//return;
		}
		result.append(src,i,pos-i);
		//std::cerr << result << " 2\n";
		result.append(to);
		//std::cerr << result << " 3\n";
		i = pos + search.size();
	}

}

std::string String::replace_regexp(const std::string &src, RegExp &search, const std::string & dst) { //,std::string &dst){

	std::string result = src;

	while (search.execute(result) != REG_NOMATCH){
		//std::cerr << "replace_searchexp: regexp "<< reg.regExpString << " found in " << result << '\n';
		if (search.result.size()==4){

			// Skip infinite loop. ('dst' would be matched infinitely)
			if (search.result[2] == dst)
				return result;
			result = search.result[1] + dst + search.result[3];
		}
		else {
			throw std::runtime_error(search.toStr() + " [ERROR] regexp error");
		}
	}

	return result;
}

/// Replaces std::string segment matching regExp to the given std::string.
/*
inline
static std::string replace_regexp(const std::string &str,const std::string &regExp,const std::string &to){
	//RegExp r(std::string("^(.*)(") + regExp + std::string(")(.*)$"));
	// TODO: allow ends ?
	RegExp r(std::string("(^.*)?(") + regExp + std::string(")(.*$)?"));
	return replace_regexp(str,r,to);
};
*/

std::string String::trim(const std::string &s, const std::string & trimChars ){ // =" \t\n") {

	const std::string::size_type pos = s.find_first_not_of(trimChars);

	// If trim characters found only, trim all.
	if (pos == std::string::npos){
		//s.clear();
		return "";
	}
	else {
		const std::string::size_type lastPos = s.find_last_not_of(trimChars);
		return s.substr(pos, lastPos-pos+1);
	}

}

/// Turns n first characters uppercase. Ascii only.
/**
 *
 */
std::string & String::upperCase(std::string & s, size_t n){ //  = std::numeric_limits<size_t>::max()) {
	n = std::min(s.length(), n);
	static const int offset = 'A'-'a';
	for (size_t i=0; i<n; ++i){
		char & c = s[i];
		if ((c>='a') && (c<='z'))
			c = c + offset;
	}
	return s;
}

/// Turns n first characters lowercase. Ascii only.
/**
 *
 */
std::string & String::lowerCase(std::string & s, size_t n){ // = std::numeric_limits<size_t>::max()) {
	n = std::min(s.length(), n);
	static const int offset = 'a'-'A';
	for (size_t i=0; i<n; ++i){
		char & c = s[i];
		if ((c>='A') && (c<='Z'))
			c = c + offset;
	}
	return s;
}



}

// Drain
