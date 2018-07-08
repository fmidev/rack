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
