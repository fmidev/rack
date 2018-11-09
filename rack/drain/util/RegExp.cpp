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
#include <stddef.h>  // size_t
//#include <regex.h> // wants mall
#include <iostream>
#include <string>
#include <sstream>
//#include <vector>
#include <stdexcept>

// g++ deer_regexp.cpp -o deer_regexp

#include "RegExp.h"

// // using namespace std;

namespace drain {

/// BUGS! Reads a regular expression std::string and stores it in a preprocessed format.
/*!
 *  BUGS! Calling exp() later does not work!
 *
 *  Based on POSIX regex functions <regex.h>, see man regex.
 *
 *  Examples:
 *  \example RegExp-example.cpp  RegExp-example.cpp
 */
//RegExp::RegExp() : result(writableResult) {	};
//RegExp::RegExp(const char *toStr,int flags)
RegExp::RegExp(const std::string &str, int flags) :  result(writableResult), flags(flags){
	regcomp(&regExpBinary,"", flags);
	setExpression(str);
}

// Lack of this caused memory leakages.
RegExp::RegExp(const RegExp &r) :  result(writableResult), flags(r.flags){
	regcomp(&regExpBinary,"",0);  // IMPORTANT, because setExpression calls regfree.
	setExpression(r.regExpString);
}

RegExp::~RegExp(){
	// Clear memory;
	//cerr << "~RegExp()" << endl;
	regfree(&regExpBinary);
	//    regExpBinary = NULL;
	// Clear result variables
	writableResult.clear();
}

RegExp &RegExp::operator=(const RegExp &r){
	setExpression(r.regExpString);
	return *this;
}

// TODO: skip this?
//RegExp &RegExp::operator=(const char *toStr){
RegExp &RegExp::operator=(const std::string &str){
	setExpression(str);
	return *this;
}

bool RegExp::setExpression(const std::string &str){

	regfree(&regExpBinary);
	writableResult.clear();

	int result = regcomp(&regExpBinary, str.c_str(), flags);

	if (result > 0){
		std::stringstream sstr;
		sstr << "RegExp::setExpression(str) failed with str='" << str << "': ";
		const unsigned long int CBUF_LENGTH = 256;
		char cbuf[CBUF_LENGTH];
		regerror(result, &regExpBinary, cbuf, CBUF_LENGTH);
		sstr << cbuf << '\n';
		regExpString = "";  // ? should be saved instead?
		throw std::runtime_error(sstr.str());
		//regExpBinary = NULL;
		return false;
	}

	regExpString = str;
	//matches.clear();
	//      this->clear();
	return true;

}




/// Attempts to match given std::string against the (compiled) regexp.
/**
 *  \par toStr - std::string to pe tested
 *  \return - true in success.
 */
bool RegExp::test(const std::string &str) const {
	return (regexec(&regExpBinary,str.c_str(),0,NULL,0) == 0);
}
/*
  bool RegExp::test(const char *toStr) const {  
      return (regexec(&regExpBinary,toStr,0,NULL,0) == 0);
  }
 */


/// Like test, but stores the matches.
/// Attempts to match given std::string against the (compiled) regexp.
//  bool RegExp::exec(const char *toStr){
/**
 *  \return REG_NOMATCH if match fails, resultcode otherwise. TODO consider returning bool, saving return code.
 */


void RegExp::replace(const std::string &src, const std::string & replacement, std::ostream  & ostr) const {

	//const size_t n = regExpBinary.re_nsub + 1;
	//cerr << "binary has subs:" << regExpBinary.re_nsub << endl;

	/// Allocates temp array for <regex.h> processing.
	std::vector<regmatch_t> pmatch(regExpBinary.re_nsub + 1);

	/// The essential <regex.h> wrapper.
	/// (Notice the negation of boolean return values.)
	/// In success, indices (rm_so,rm_eo)
	/// will point to matching segments of toStr.
	/// Eflags not implemented (yet?).

	//cerr << "\nTrying " << toStr.c_str() << endl;
	int resultCode = regexec(&regExpBinary, src.c_str(), pmatch.size(), &pmatch[0], 0) ;

	if (resultCode == REG_NOMATCH){
		//dst = src;
		ostr << src;
	}
	else {

		const regmatch_t &m = pmatch[0];
		if (m.rm_so != -1){
			//std::stringstream sstr;
			//ostr << src.substr(0, m.rm_so);
			replace(src.substr(0, m.rm_so), replacement, ostr);
			//sstr << src.substr(m.rm_so, m.rm_eo - m.rm_so);
			ostr << replacement;
			replace(src.substr(m.rm_eo), replacement, ostr);
			//ostr << src.substr(m.rm_eo);
			//dst = sstr.str();
		}
		else {
			// When does this happen?
			ostr << src; //dst = src;
		}

	}

	return; //resultCode;

}

std::ostream & operator<<(std::ostream &ostr, const drain::RegExp & r){

	ostr << r.toStr() << ", flags=" << r.flags;
	for (std::vector<std::string>::const_iterator it = r.result.begin(); it != r.result.end(); ++it){
		ostr << '|' << *it;
	}
	return ostr;
}


} // drain



// Drain
