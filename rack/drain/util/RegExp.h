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

#include <iostream>
#include <string>
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

	//    bool exp(const char *toStr);
	
	inline 
    void clear(){ writableResult.clear(); };

	/// Public interface for the result
	const std::vector<std::string> & result;

    /// Tests if the regular expression accepts the given std::string
    bool test(const std::string & str) const;

    /// Returns 0 on success, REG_NOMATCH on failure
    int execute(const std::string & str, std::vector<std::string> & result) const;

    /// Variant using internal vector
    // somewhat questionable whether should be const.
    int execute(const std::string & str) const {
    	return execute(str, writableResult);
    }

    inline
    const std::string & toStr() const {
    	return regExpString;
    };


  protected:
    int flags;
    regex_t regExpBinary;  // this is weird  FIXME: check pointer aspect
	
	std::string regExpString;
	
	mutable std::vector<std::string> writableResult;

  private:
    int expectedMatchCount() const;
  };

  std::ostream & operator<<(std::ostream &ostr, const drain::RegExp & r);

} // drain


#endif

// Drain
