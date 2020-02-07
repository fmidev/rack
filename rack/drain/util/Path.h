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

	typedef T elem_t;

	typedef std::list<Path<T> > list_t;

	// Consider flags
	/*
	static const int BEGIN  = 1;
	static const int MIDDLE = 2;
	static const int END    = 4;
	*/

	inline
	Path(char separator='/') : separator(separator), rooted(true){
		if (!separator)
			throw std::runtime_error("Path(char separator): separator=0, did you mean empty init (\"\")");
	};

	/// Copy constructor. Note: copies also the separator.
	inline
	Path(const Path<T> & p) : std::list<T>(p), separator(p.separator), rooted(p.rooted) {
	};

	inline
	Path(const std::string & s, char separator='/') : separator(separator), rooted(true) {
		set(s);
	};

	inline
	Path(const char *s, char separator='/') : separator(separator), rooted(true) {
		set(s);
	};


	/// Constructor with an initialises element - typically a root.
	//  PROBLEMATIC - elem may be std::string, and also the full path representation
	/**
	 *  The root might correspond to an empty string.
	 */
	/*
	inline
	Path(const elem_t & e, char separator='/') : separator(separator) {
		this->push_back(e);
	};
	*/

	virtual inline
	~Path(){};

	char separator;

	/// If true, recognize plain separator (e.g. "/") as a root
	bool rooted;

	inline
	bool isRoot() const {
		return rooted && ((this->size()==1) && this->front().empty());
	}

	void set(const std::string & p){

		std::list<T>::clear();

		std::list<std::string> l;
		StringTools::split(p, l, separator);

		for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); ++it) {

			if (!it->empty()){
				*this << *it;
			}
			else if ((it == l.begin()) && rooted){
				*this << *it;
			}
			else { // trailing: if (it == --l.end()){
				// skip
				// warn, esp. intermediate empties?
			}

			/*
			else if (it == --l.end()){
				// else if ((acceptEmpty & END) && (it == --l.end())){
				// else if ((acceptEmpty & END) && ((++std::list<std::string>::const_iterator(it)) == l.end())){
				// *this << *it;
			}
			else if (acceptEmpty & MIDDLE){
				*this << *it;
			}
			else if ((this->size()==1) && (it->at(0)==separator)){

			}
			else {
				// throw
				std::cerr << "Path::set: not accepting empty element at this position '" << p << "', flags=" << acceptEmpty << "'\n";
			}
			 */
		}

	}

	Path<T> & operator=(const Path<T> & p){
		std::list<T>::operator=(p);
		return *this;
	}

	/// Conversion from str path type
	template <class T2>
	Path<T> & operator=(const Path<T2> & p){
		std::list<T>::clear();
		for (typename Path<T2>::const_iterator it = p.begin(); it != p.end(); ++it) {
			*this << *it;
		}
		return *this;
	}


	inline
	Path<T> & operator=(const std::string & p){
		set(p);
		return *this;
	}

	inline
	Path<T> & operator=(const char *p){
		set(p);
		return *this;
	}

	// Note: this would be ambiguous!
	// If (elem_t == std::string), elem cannot be assigned directly, because string suggest full path assignment, at least
	// Path<T> & operator=(const elem_t & e)

	/// Append an element, unless empty string.
	/*
	Path<T> & operator<<(const std::string & s){
		if (this->empty()) {
			std::cerr << "root! (empty)\n";
			*this << *it;
		}
		else {
			// warn about empty path elements?
		}

		if (!s.empty())
			this->push_back(s);
		return *this;
	}
	*/

	/// Append an element
	template <class T2>
	Path<T> & operator<<(const T2 & elem){
		this->push_back(elem);
		return *this;
	}

	/// Extract last element.
	Path<T> & operator>>(elem_t & e){
		e = this->back();
		this->pop_back();
		return *this;
	}


	operator std::string () const {
		std::stringstream sstr;
		toOStr(sstr);
		return sstr.str();
	}


	virtual inline
	std::ostream & toOStr(std::ostream & ostr, char separator = 0) const {
		separator = separator ? separator : this->separator;
		if (isRoot())
			return (ostr << separator);
		else
			return drain::StringTools::join(*this, ostr, separator);
	}

	inline
	void toStr(std::string & str, char separator = 0) const {
		std::stringstream sstr;
		toOStr(sstr, separator);
		str = sstr.str();
	}


};

//template <class T>
//typedef std::list<Path<T>> PathList<T>;


template <class T>
inline
std::ostream & operator<<(std::ostream & ostr, const Path<T> & p) {
	return p.toOStr(ostr);
}

template <class T>
inline
std::istream & operator>>(std::istream &istr, Path<T> & p) {
	std::string s;
	istr >> s;
	p = s;
	return istr;
}


}

#endif /* Path_H_ */

// Drain
