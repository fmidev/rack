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

#ifndef DRAIN_FileS_H_
#define DRAIN_FileS_H_

#include <stdexcept>
#include <iostream>
#include <list>
#include <iterator>
#include <iostream>
#include <sstream>
#include <list>

#include "util/RegExp.h"
#include "util/Path.h"


namespace drain {

/// Extracts and stores directory path, base filename and extension
/**
 *    Upon construction and after setting, splits a full path string to
 * 	- \c dir      - directory path (Path<std::string>)
 * 	- \c basename - filename without directory path and extension
 * 	- \c ext      - extension, without dot (png, txt
 */
class FilePath {

public:

	typedef Path<std::string> path_t;

	/// Constructor
	FilePath(const std::string & s = "", char separator = 0);

	/// Copy constructor
	FilePath(const FilePath & s);

	inline
	~FilePath(){};


	void set(const std::string & s);

	path_t dir;
	std::string basename;
	std::string extension;

	/// Directory path separator
	static
	char separator;

	static
	const RegExp pathRegExp;

	virtual inline
	std::ostream & toOStr(std::ostream & ostr, char dirSeparator = 0) const {
		if (!dir.empty()){
			dir.toOStr(ostr, dirSeparator);
			ostr << (dirSeparator ? dirSeparator : dir.separator);
		}
		ostr << basename << '.' << extension;
		return ostr;
	}

	operator std::string(){
		return toStr();
	}

	inline
	void toStr(std::string & str, char separator = 0) const {
		std::stringstream sstr;
		toOStr(sstr, separator);
		str = sstr.str();
	}

	inline
	std::string toStr(char separator = 0) const {
		std::stringstream sstr;
		toOStr(sstr, separator);
		return sstr.str();
	}

};


//template <class T>
inline
std::ostream & operator<<(std::ostream & ostr, const FilePath & f) {
	return f.toOStr(ostr);
}

} // drain::


#endif
