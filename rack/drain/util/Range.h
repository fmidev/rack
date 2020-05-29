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
 * Rectangle.h
 *
 *  Created on: Sep 9, 2010
 *      Author: mpeura
 */

#ifndef DRAIN_RANGE_H_
#define DRAIN_RANGE_H_

#include <ostream>
//#include <cmath>
//#include <string>
//#include <sstream>
#include <vector>
// // using namespace std;


namespace drain {

template <class T>
class Range  {

public:

	Range() : vect(2, 0), min(vect[0]), max(vect[1]) { //
	}

	/// Copy constructor.
	Range(const Range<T> & range) : vect(2), min(vect[0]), max(vect[1]) {
		set(range);
	}

	Range(T min, T max) : vect(2), min(vect[0]=min), max(vect[1]=max) { //
		set(min, max);
	}

	std::vector<T> vect;
	//T vect[2]; Reference::link() caused problems
	T & min;
	T & max;

	void set(T min, T max){
		this->min = min;
		this->max = max;
	}

	void set(const Range<T> & range){
		this->min = range.min;
		this->max = range.max;
	}

	Range & operator=(const Range<T> & range){
		min = range.min;
		max = range.max;
		return *this;
	};

	Range<T> & operator=(const T &x){
		min = x;
		max = x;
		return *this;
	};

	bool contains(T x) const {
		return (min <= x) && (x <= max);
	}

};


template <class T>
std::ostream & operator<<(std::ostream & ostr, const Range<T> & r){
	ostr << r.min << ':' << r.max;
	return ostr;
}

} // namespace drain



#endif /* RECTANGLE_H_ */

