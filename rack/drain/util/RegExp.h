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
#ifndef REGEXP_H__
#define REGEXP_H__ "deer::RegExp v0.1 Dec 2004 Markus.Peura@fmi.fi"

#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <regex.h> // wants malloc?

// g++ deer_regexp.cpp -o deer_regexp
// typedef size_t int;
// // using namespace std;

namespace drain {

/*!
 *  Based on POSIX regex functions <regex.h>, see man regex.
 *
 *  Examples:
 *  Regexp r='ksooskks';
 *  r.regexp('dkksks');
 *  //r.exec('Test (s)tribng');
 *  r == 'test_std::string';
 *
 * \example RegExp-example.cpp
 */
class RegExp { //: public vector<string> {
public:

	/// Constructor
	//RegExp(const char *toStr = "");

	/// Constructor
	//RegExp(const char *toStr = "",int flags = REG_EXTENDED);
	RegExp(const std::string &str = "", int flags = REG_EXTENDED);

	/// Copy constructor.
	// Lack of this caused memory leakages.
	RegExp(const RegExp &r);

	/// Destructor.
	virtual
	~RegExp();

	//RegExp &operator=(const char *toStr);
	RegExp &operator=(const std::string &str);
	RegExp &operator=(const RegExp &r);

	//    bool exp(const char *toStr);
	bool setExpression(const std::string &str);

	inline 
	void clear(){ writableResult.clear(); };

	inline
	bool isSet() const {
		return !regExpString.empty();
	};

	/// Public interface for the result
	const std::vector<std::string> & result;

	/// Tests if the regular expression accepts the given std::string
	bool test(const std::string & str) const;

	/// Apply regexp matching in a string. Returns 0 on success, REG_NOMATCH on failure
	/**
	 *  \tparam T - A class implementing clear() and push_back() like STL Sequence, eg. std::vector or std::list
	 *
	 *  \param str    - source string
	 *  \param result - result of the match (STL Sequence)
	 *
	 *  \return - 0 on success, REG_NOMATCH on failure
	 */
	template <class T>
	int execute(const std::string & str, T & result) const;

	/// Variant using internal vector
	// somewhat questionable whether should be const.
	int execute(const std::string & str) const {
		return execute(str, writableResult);
	}

	/// Replaces std::string segment matching search regexp to the given std::string.
	/**
	 *  \see StringTools::replace.
	 */
	inline
	void replace(const std::string &src, const std::string &replacement, std::string & dst) const {
		std::stringstream sstr;
		replace(src, replacement, sstr);
		dst = sstr.str();
	}


	void replace(const std::string &src, const std::string &replacement, std::ostream & sstr) const;

	/// Replaces std::string segment matching search to the given std::string.
	static inline
	//void RegExp::replace(const std::string &src, std::string & dst) { //,std::string &dst){
	void replace(const std::string &src, const std::string &regexp, const std::string &replacement, std::string & dst){
		RegExp r(std::string("(^.*)?(") + regexp + std::string(")(.*$)?"));
		r.replace(src, replacement, dst);
		return; // r.replace(src, dst);
	};



	inline
	const std::string & toStr() const {
		return regExpString;
	};

	int flags;

protected:

	regex_t regExpBinary;  // this is weird  FIXME: check pointer aspect

	std::string regExpString;

	mutable std::vector<std::string> writableResult;

private:
	int expectedMatchCount() const;
};

std::ostream & operator<<(std::ostream &ostr, const drain::RegExp & r);


template <class T>
int RegExp::execute(const std::string &src, T & result) const {

	/// Allocates space for the matches.
	const size_t n = regExpBinary.re_nsub + 1;
	// cerr << "binary has subs:" << regExpBinary.re_nsub << endl;

	result.clear();
	//result.resize(n);

	//cout << "resize => " << this->size() << endl;

	/// Allocates temp array for <regex.h> processing.
	std::vector<regmatch_t> pmatch(n);

	/// The essential <regex.h> wrapper.
	/// (Notice the negation of boolean return values.)
	/// In success, indices (rm_so,rm_eo)
	/// will point to matching segments of toStr.
	/// e-flags not implemented (yet?).

	//cerr << "\nTrying " << toStr.c_str() << endl;
	int resultCode = regexec(&regExpBinary, src.c_str(), pmatch.size(), &pmatch[0], 0) ;

	if (resultCode != REG_NOMATCH){
		for (std::vector<regmatch_t>::const_iterator it =  pmatch.begin(); it < pmatch.end(); ++it) {
			const regmatch_t &m = *it;
			if (m.rm_so != -1){
				result.push_back(src.substr(m.rm_so, m.rm_eo - m.rm_so));
			}
		}
	}

	//delete[] pmatch; // valgrind herjasi muodosta: delete pmatch
	return resultCode;

}

} // drain


#endif

// Drain
