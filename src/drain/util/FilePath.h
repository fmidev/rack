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

#ifndef DRAIN_FILES_H_
#define DRAIN_FILES_H_

#include <stdexcept>
#include <iostream>
#include <list>
#include <iterator>
#include <iostream>
#include <sstream>
#include <list>

#include "RegExp.h"
#include "Path.h"


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

	typedef Path<std::string,'/',true,false,true> path_t;

	/// Constructor
	FilePath(const std::string & s = "");

	/// Copy constructor
	FilePath(const FilePath & s);

	inline
	~FilePath(){};


	void set(const std::string & s);

	/*
	template<typename ... TT>
	void set(const TT &... rest, const std::string & elem){
		//this->clear();
		appendElems(rest...);
		set(elem);
	}
	*/




	path_t dir;
	std::string basename;
	std::string extension;

	/// Directory path separator
	//static
	//char separator;

	//static const RegExp pathRegExp;


	virtual
	std::ostream & toStream(std::ostream & ostr) const { //, char dirSeparator = 0) const {

		if (!dir.empty()){
			ostr << dir;
			if (!dir.back().empty())
				ostr << dir.separator.character;
		}

		ostr << basename;

		if (!extension.empty())
			ostr << '.' << extension;

		return ostr;
	}

	// Not inhrerited?
	virtual
	std::string  str() const {
		std::stringstream sstr;
		toStream(sstr);
		return sstr.str();
	}

	inline
	FilePath & operator<<(const FilePath & path){
		this->dir << path.dir;
		if (!this->basename.empty())
			std::cerr << __FILE__ << " warning: dropped:" << this->basename << '\n';
		this->basename  = path.basename;
		this->extension = path.extension;
		//this->insert(this->end(), path.begin(), path.end());
		return *this;
	}


};

inline
std::ostream & operator<<(std::ostream & ostr, const FilePath & p) {
	return p.toStream(ostr);
}


} // drain::


#endif
