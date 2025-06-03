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
#include <sys/stat.h>


#include <drain/RegExp.h>
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
	//FilePath(const std::string & s = "");
	template<typename ...TT>
	inline
	FilePath(const TT &... args){
		set(args...);
	}


	/// Copy constructor
	FilePath(const FilePath & s);

	virtual inline
	~FilePath(){};

	inline
	void clear(){
		this->dir.clear();
		this->basename.clear();
		this->extension.clear();
	}

	//void set(const std::string & s);

	template<typename ...TT>
	inline
	void set(const TT &... args){
		this->clear();
		append(args...);
	}

	template<typename T, typename ...TT>
	void append(const T & arg, const TT &... args){
		this->dir.append(arg);
		append(args...);
		//set(elem);
	}

	/// Appends directory and sets filename (basename and extension).
	void append(const FilePath & path);

	/// Extracts at least filename. If the string contains leading dir elements, \i append them to \c this->dir .
	/**
	 *
	 */
	void append(const std::string & s);


	inline
	void append(){};
	/*
	template<typename T>
	void append(const T & arg){

	}
	*/


	inline
	bool operator==(const FilePath & p) const {

		if (p.basename != basename)
			return false;

		if (p.extension != extension)
			return false;

		if (p.dir != dir)
			return false;

		return true;
	}


	path_t dir;
	std::string basename;
	std::string extension;

	//bool isRooted

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

	static inline
	int mkdir(const std::string & dirpath, int flags = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH){
		return mkdir(FilePath::path_t(dirpath), flags);
	}

	// Note: Path, not FilePath
	static
	int mkdir(const FilePath::path_t & dirpath, int flags = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);


};

inline
std::ostream & operator<<(std::ostream & ostr, const FilePath & p) {
	return p.toStream(ostr);
}


} // drain::


#endif
