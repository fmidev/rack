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

//#include <ostream>
//
//#include <utility>

#include "UniTuple.h"

namespace drain {



template <class T>
class Range : public drain::UniTuple<T,2> {
public:

	T & min;
	T & max;

	/// Default constructor.
	Range(T min=T(), T max=T()) : min(this->next()),  max(this->next()) {
		this->set(min, max);
	};

	/// Copy constructor.
	Range(const Range & r) :  min(this->next()), max(this->next()){
		this->set(r.tuple());
	};

	// Reference (parasite)
	template <size_t N>
	Range(drain::UniTuple<T,N> & tuple, size_t i) :
		drain::UniTuple<T,2>(tuple, i), // start in new pos!
		min(this->next()),max(this->next()){
	};


	/// Default assignment operator
	Range & operator=(const Range<T> & range){
		this->assign(range);
		return *this;
	};

	Range & operator=(const UniTuple<T,2> & range){
		this->assign(range);
		//this->set(range);
		return *this;
	};


	Range<T> & operator=(T x){
		this->set(x, x);
		return *this;
	};

	inline
	bool contains(T x) const {
		return (this->min <= x) && (x <= this->max);
	}

	inline
	bool limit(T x) const {
		if (x < this->min)
			return this->min;
		else if (x > this->max)
			return this->max;
		else
			return x;
	}

	// Returns (max - min) which may be negative if max
	inline
	T width() const {
		return this->max - this->min;
	}

	// Returns abs(max - min).
	inline
	T span() const {
		if (this->max > this->min)
			return this->max - this->min;
		else
			return this->min - this->max;
	}

};

/*
template <class T>
std::ostream & operator<<(std::ostream & ostr, const Range<T> & r){
	ostr << r.min << ':' << r.max;
	return ostr;
}
*/



} // namespace drain



#endif /* RECTANGLE_H_ */

