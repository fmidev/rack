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

#include <drain/Log.h>
#include "String.h"


#include <stdexcept>
#include <iostream>

namespace drain {

bool StringTools::startsWith(const std::string &s, const std::string & substring){

	if (substring.size() > s.size()){
		return false;
	}

	std::string::const_iterator  it = s.begin();
	std::string::const_iterator sit = substring.begin();
	while (sit != substring.end()){
		if (*it != *sit){
			return false;
		}
		++it;
		++sit;
	}

	return true;
}


bool StringTools::endsWith(const std::string &s, const std::string & substring){

	if (substring.size() > s.size()){
		return false;
	}

	std::string::const_reverse_iterator  it = s.rbegin();
	std::string::const_reverse_iterator sit = substring.rbegin();
	while (sit != substring.rend()){
		if (*it != *sit){
			return false;
		}
		++it;
		++sit;
	}

	return true;
}
// virtual
// bool handle(const std::string & key, const std::map<std::string,T> & variables, std::ostream & ostr) const {


void StringTools::replace(const std::string &src, char from, char to, std::string &dst){
	if (&dst != &src){
		dst.assign(src);
	};
	for (char & c: dst){
		if (c==from){
			c = to;
		}
	}
}


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

void StringTools::replace(const conv_map_t & m, std::string & s, std::size_t pos){

	std::size_t len;
	std::size_t step;

	while (pos < s.size()){

		step = 1;

		for (std::map<std::string,std::string>::const_iterator it = m.begin(); it!=m.end(); ++it){
			len = it->first.length();
			if (s.compare(pos, len, it->first) == 0){
				s.replace(pos, len, it->second);
				step = it->second.length();
				break;
			}
		}

		pos = pos + step;
	}

}

void StringTools::replace(const std::map<char,std::string> & m, const std::string & src, std::ostream & ostr){


	std::string::const_iterator it = src.begin(); //  + pos;
	//std::forward(it, len);
	while (it != src.end()){

		std::map<char,std::string>::const_iterator mit = m.find(*it);
		if (mit == m.end()){
			ostr << *it;
		}
		else {
			ostr << mit->second;
		}

		++it;
	}


}


std::string StringTools::trim(const std::string &s, const std::string & trimChars ){

	std::string::size_type pos1 = 0;
	std::string::size_type pos2 = s.size();

	if (trimScan(s, pos1, pos2, trimChars))
		return s.substr(pos1, pos2-pos1);
	else {
		return "";
	}

}

/// Coupled trimming: remove a single leading and trailing char, if both found.
std::string StringTools::trimSymmetric(const std::string &s, const std::string &leading, const std::string & trailing){

	const std::string::size_type length = s.length();

	if (length < 2){
		return s;
	}

	const std::string & lead  = leading;
	const std::string & trail = trailing.empty() ? leading : trailing;

	// Check if a leading special character is found, at all
	std::size_t i = lead.find(s.at(0));
	if (i != std::string::npos){
		// Ok, check if a matching ending character is found (ie. ending char at the same position).
		if (trail.find(s.at(length - 1)) == i){
			return s.substr(1, length-2);
		}
	}

	return s;

}


// https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string/237280#237280
// CONSIDER:  istringstream iss(s); while (iss) { string subs; iss >> subs; cout << "Substring: " << sub << endl; }

bool StringTools::trimScan(const std::string &s, size_t & posLeft, size_t & posRight, const std::string & trimChars){


	if (s.empty()){
		posLeft = posRight = 0;
		return false;
	}

	if (posRight == 0){ // by definition, posRight is after the scanned segment, hence empty string segment  => return false
		return false;
	}

	// Possibly posLeft>=s.length() or posLeft>=posRight, but checked next anyway:

	const size_t p1 = s.find_first_not_of(trimChars, posLeft);

	//if ((p1 == std::string::npos) || (p1 >= posRight)){
	if (posRight > s.length())
		posRight = s.length();

	if (p1 >= posRight){ // includes npos check
		posLeft = posRight;  // strlen = 0, note: now posLeft may be s.length(), hence illegal index.
		return false;
	}
	else {

		posLeft = p1;
		//std::cerr << __FUNCTION__ << " search2: " << (posRight-1) << '\n';


		size_t p2 = s.find_last_not_of(trimChars, posRight-1); // posRight != 0 (checked above)

		if (p2 == std::string::npos){ // not found (left of posRight)
			// keep pos2
		}
		else if (p2<posLeft){
			// = no trailing trimchars, but leading
			// keep pos2 also here!

			//std::cerr << __FUNCTION__ << " success1: '" << s << "' [" << posLeft << ',' << posRight << '[' << " = '" << s.substr(posLeft, posRight-posLeft) << "'\n";
			//std::cerr << __FUNCTION__ << " success1: " << p2 << '\n';
			return false; // WAS true?
		}
		else {
			// Set posRight after last non-trimChar
			posRight = p2+1;
			// std::cerr << __FUNCTION__ << " success2: '" << s << "' [" << posLeft << ',' << posRight << '[' << " = '" << s.substr(posLeft, posRight-posLeft) << "'\n";
			//return true;
		}
		return (posLeft < posRight);
		//return true;
		//return (posRight > posRight);
	}

}


/// Extract common leading part
/**
 *  \return - index of the first deviating character, ie. length of the common part.
 */
size_t StringTools::extractPrefixLength(const std::string & src1, const std::string & src2, size_t step){

	if (step == 0){
		throw std::runtime_error("StringTools::extractPrefixLength step=0");
	}

	const size_t L = std::min(src1.length(), src2.length());
	for (size_t i=0; i<L; ++i){
		if (src1[i] != src2[i]){
			return i - (i%step);
		}
	}
	return L - (L%step);
}

/// Extract common leading part
size_t StringTools::extractPrefix(const std::string & src1, const std::string & src2, std::string & prefix, std::string & dst1, std::string & dst2, size_t step){
	const size_t length = extractPrefixLength(src1, src2, step);
	prefix = src1.substr(0, length);
	dst1 = src1.substr(length);
	dst2 = src2.substr(length);
	return length;
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

// Drain
