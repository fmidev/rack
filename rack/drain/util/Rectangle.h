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

#ifndef DRAIN_RECTANGLE2_H_
#define DRAIN_RECTANGLE2_H_

//#include <ostream>
#include <cmath>
// #include <string>
// #include <sstream>
#include <vector>

#include "UniTuple.h"
#include "Point.h"

// // using namespace std;


namespace drain {

/// Rectange defined through lower left and upper right coordinates.
/**
 *
 *  \see drain::Range<T>
 *
 */


template <class T>
struct Rectangle : public drain::UniTuple<T,4> {

	Point2D<T> lowerLeft;
	Point2D<T> upperRight;
	// consider Range<T> x, Range<T> y,

	Rectangle(T x=0, T y=0, T x2=0, T y2=0) : lowerLeft(this->tuple(), 0), upperRight(this->tuple(), 2) {
		this->set(x, y, x2, y2);
	}


	Rectangle(const Rectangle & r) : lowerLeft(this->tuple(), 0), upperRight(this->tuple(), 2){
		this->assign(r);
	};

	Rectangle<T> & operator=(const Rectangle<T> & rect){
		this->set(rect.tuple());
		return *this;
	}


	inline
	T getWidth() const { return (this->upperRight.x - this->lowerLeft.x); };

	inline
	T getHeight() const { return (this->upperRight.y - this->lowerLeft.y); };

	inline
	T getArea() const { return std::abs(getWidth()*getHeight()); };

	inline
	void getCenter(drain::Point2D<T> &p) const {
		p.x = static_cast<T>((lowerLeft.x + upperRight.x)/2.0);
		p.y = static_cast<T>((lowerLeft.y + upperRight.y)/2.0);
	};


	/// This becomes the intersection of r and this.
	/**
	 *   Two way bounds are needed, because cropping max can be lower than this min.
	 */
	void crop(const Rectangle<T> & r);
	/*
	{
		const Rectangle<T> bounds(*this);
		*this = r;
		limit(bounds.lowerLeft.x, bounds.upperRight.x, this->lowerLeft.x);
		limit(bounds.lowerLeft.x, bounds.upperRight.x, this->upperRight.x);
		limit(bounds.lowerLeft.y, bounds.upperRight.y, this->lowerLeft.y);
		limit(bounds.lowerLeft.y, bounds.upperRight.y, this->upperRight.y);
	}
	*/

	/// The instance extends to its union with r.
	void extend(const Rectangle & r);

	/// The instance reduces to itse intersection with r.
	void contract(const Rectangle & r);


	inline
	bool isInside(const T &x,const T &y) const {
		return ((x>this->lowerLeft.x) && (x<this->upperRight.x) && (y>this->lowerLeft.y) && (y<this->upperRight.y));
	};

	inline
	bool isOverLapping(const Rectangle &r) const {
		const bool xOverLap = !((r.upperRight.x < this->lowerLeft.x) || (r.lowerLeft.x > this->upperRight.x));
		const bool yOverLap = !((r.upperRight.y < this->lowerLeft.y) || (r.lowerLeft.y > this->upperRight.y));
		return (xOverLap && yOverLap);
	};

	/// Write corner points to a vector [llX, llY, urX, urY]
	inline
	std::vector<T> toVector() const {
		std::vector<T> v;
		UniTuple<T,4>::toSequence(v);
		return v;
	}


protected:

	static inline
	void limit(const T & lowerBound, const T & upperBound, T & x){
		if (x < lowerBound)
			x = lowerBound;
		else if (x > upperBound)
			x = upperBound;
	}

};


template <class T>
void Rectangle<T>::crop(const Rectangle<T> & r){
	const Rectangle<T> bounds(*this);
	*this = r;
	limit(bounds.lowerLeft.x, bounds.upperRight.x, this->lowerLeft.x);
	limit(bounds.lowerLeft.x, bounds.upperRight.x, this->upperRight.x);
	limit(bounds.lowerLeft.y, bounds.upperRight.y, this->lowerLeft.y);
	limit(bounds.lowerLeft.y, bounds.upperRight.y, this->upperRight.y);
}


template <class T>
void Rectangle<T>::extend(const Rectangle<T> & r)
{
	this->lowerLeft.x  = std::min(this->lowerLeft.x,r.lowerLeft.x);
	this->lowerLeft.y  = std::min(this->lowerLeft.y,r.lowerLeft.y);
	this->upperRight.x = std::max(this->upperRight.x,r.upperRight.x);
	this->upperRight.y = std::max(this->upperRight.y,r.upperRight.y);
}


template <class T>
void Rectangle<T>::contract(const Rectangle<T> & r)
{
	this->lowerLeft.x  = std::max(this->lowerLeft.x,r.lowerLeft.x);
	this->lowerLeft.y  = std::max(this->lowerLeft.y,r.lowerLeft.y);
	this->upperRight.x = std::min(this->upperRight.x,r.upperRight.x);
	this->upperRight.y = std::min(this->upperRight.y,r.upperRight.y);
}


template <class T>
std::ostream & operator<<(std::ostream &ostr,const drain::Rectangle<T> &r){
	ostr << r.lowerLeft.x << ',' << r.lowerLeft.y << ' ' <<  r.upperRight.x << ',' << r.upperRight.y;
	return ostr;
}


} // namespace drain

/*
template <class T>
std::ostream & operator<<(std::ostream &ostr,const drain::Rectangle<T> &r){
	ostr << r.lowerLeft.x << ',' << r.lowerLeft.y << ' ' <<  r.upperRight.x << ',' << r.upperRight.y;
	return ostr;
}
*/





#endif /* RECTANGLE_H_ */

// Drain
