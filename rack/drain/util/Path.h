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
 * Path.h
 *
 *  Created on: Jul 21, 2010
 *      Author: mpeura
 */

#ifndef DRAIN_PATH_H_
#define DRAIN_PATH_H_

#include <stdexcept>
#include <iostream>
#include <string>
#include <list>
#include <iterator>


#include <util/String.h>



namespace drain {


/**
 *   \tparam T - path element, eg. PathElement
 */
template <class T>
class Path : public std::list<T> {

public:

	Path(char separator='/') : separator(separator){
		if (!separator)
			throw std::runtime_error("Path(char separator): separator=0, did you mean empty init (\"\")");
	};

	Path(const std::string &s, char separator='/') : separator(separator){
		if (!separator)
			throw std::runtime_error("Path(const string &s, char separator): separator=0");
		set(s);
	};


	inline
	Path(const Path<T> & p) : std::list<T>(p), separator(p.separator) {
	};

	virtual inline
	~Path(){};

	char separator;

	inline
	void set(const std::string & p){
			const std::string sep(1,separator);
			StringTools::split(p, *this, sep); //, sep);
			/*
			const size_t length = path.length();
			size_t i  = 0;
			size_t i2 = 0;
			this->clear();
			while ((i2 = path.find(this->separator, i)) != std::string::npos){
				//std::cout << (size_t)(i2) << '\n';
				//std::cout << path.substr(i, i2-i) << '\n';
				this->push_back(T(path.substr(i, i2-i)));
				i = i2+1;
				if (i == length) // last char was this->separator (warning?)
					return;
			}
			this->push_back(path.substr(i));
			*/
		}

	virtual inline
	std::ostream & toOStr(std::ostream & ostr) const {
		return drain::StringTools::join(*this, ostr, this->separator);
	}

	inline
	void toStr(std::string & str) const {
		std::stringstream sstr;
		toOStr(sstr);
		str = sstr.str();
	}

	operator std::string (){
		std::stringstream sstr;
		toOStr(sstr);
		return sstr.str();
	}

		/*
	Path<T> & operator=(const T & e){
		this->clear();
		push_front(e);
	}

	Path<T> & operator<<(const T & e){
		push_back(e);
	}
	*/

	Path<T> & operator<<(const T & e){
		this->push_back(e);
		return *this;
	}

	Path<T> & operator>>(T & e){
		//push_back(e);
		e = this->back();
		this->pop_back();
		return *this;
	}

};

template <class T>
inline
std::ostream & operator<<(std::ostream & ostr, const Path<T> & p) {
	return p.toOStr(ostr);
}

}

#endif /* Path_H_ */

// Drain
