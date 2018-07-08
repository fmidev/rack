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

*/

#include "Log.h"
#include "String.h"


#include <stdexcept>
#include <iostream>

namespace drain {

void StringTools::replace(const std::string &src, const std::string &search, const std::string &to, std::string & dst){

	if (&dst == &src){
		std::string tmp;
		StringTools::replace(src, search, to, tmp);
		dst = tmp;
		return;
	}

	std::string::size_type i = 0;
	std::string::size_type pos;
	dst.clear();

	while (true) {
		pos = src.find(search, i);
		if (pos == std::string::npos){
			dst.append(src, i, src.size()-i);
			//std::cerr << result << '\n';
			return; // result;
			//return;
		}
		dst.append(src, i, pos-i);
		//std::cerr << result << " 2\n";
		dst.append(to);
		//std::cerr << result << " 3\n";
		i = pos + search.size();
	}

}


bool StringTools::trim(const std::string &s, size_t & pos1, size_t & pos2, const std::string & trimChars){


	if (s.empty()){
		/*
		//pos1 = pos2 = std::string::npos; // or s.size()?
		if ((pos1 > 0)||(pos2 > 0)){

			drain::Logger mout(getLog(), "StringTools", __FUNCTION__);
			mout.warn() << "s='" << s << "', pos1=" << pos1 << ", pos2=" << pos2 << mout.endl;

			throw std::runtime_error("StringTools::trim() with empty string but non-zero indices");
		}
		*/
		pos1 = pos2 = 0;
		return false;
	}

	if (pos2 == 0){ // by definition, pos2 is after the scanned segment, hence empty string segment  => return false
		return false;
	}

	const size_t p1 = s.find_first_not_of(trimChars, pos1);

	if ((p1 == std::string::npos) || (p1 >= pos2)){
		pos1 = pos2;  // strlen = 0
		return false;
	}
	else {
		pos1 = p1;
		size_t p2 = s.find_last_not_of(trimChars, pos2-1); // pos2 != 0 (checked above)
		if ((p2 == std::string::npos)||(p2<=pos1)){
			// = no _trailing_ trimchars
			// So don't move, keep pos2!
			// std::cerr << __FUNCTION__ << " success1: '" << s << "' [" << pos1 << ',' << pos2 << '[' << " = '" << s.substr(pos1, pos2-pos1) << "'\n";
			return true;
		}
		else {
			// Set pos2 after last non-trimChar
			pos2 = p2+1;
			// std::cerr << __FUNCTION__ << " success2: '" << s << "' [" << pos1 << ',' << pos2 << '[' << " = '" << s.substr(pos1, pos2-pos1) << "'\n";
			return true;
		}
	}

}


std::string StringTools::trim(const std::string &s, const std::string & trimChars ){

	std::string::size_type pos1 = 0;
	std::string::size_type pos2 = std::string::npos;

	if (trim(s, pos1, pos2, trimChars))
		return s.substr(pos1, pos2-pos1);
	else {
		return "";
	}

}


char StringTools::upperCase(char c){
	static const int offset = 'A'-'a';
	if ((c>='a') && (c<='z'))
		c = c + offset;
	return c;
}

/// Turns n first characters uppercase. Ascii only.
/**
 *
 */
std::string & StringTools::upperCase(std::string & s, size_t n){ //  = std::numeric_limits<size_t>::max()) {
	n = std::min(s.length(), n);
	//static const int offset = 'A'-'a';
	for (size_t i=0; i<n; ++i){
		s[i] = StringTools::upperCase(s[i]);
	}
	return s;
}


char StringTools::lowerCase(char c){
	static const int offset = 'a'-'A';
	if ((c>='A') && (c<='Z'))
		c = c + offset;
	return c;
}

/// Turns n first characters lowercase. Ascii only.
/**
 *
 */
std::string & StringTools::lowerCase(std::string & s, size_t n){ // = std::numeric_limits<size_t>::max()) {
	n = std::min(s.length(), n);
	for (size_t i=0; i<n; ++i){
		s[i] = StringTools::lowerCase(s[i]);
	}
	return s;
}



}
