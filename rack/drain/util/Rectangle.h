/**

    Copyright 2010 -   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
 * Rectangle.h
 *
 *  Created on: Sep 9, 2010
 *      Author: mpeura
 */

#ifndef RECTANGLE2_H_
#define RECTANGLE2_H_

#include <ostream>
#include <cmath>
#include <string>
#include <sstream>

// // using namespace std;

namespace drain {

template <class T>
class Rectangle {
public:

	Rectangle(){
		set(0, 0, 0, 0);
	};

	Rectangle(T xLowerLeft, T yLowerLeft, T xUpperRight, T yUpperRight){
		set(xLowerLeft, yLowerLeft, xUpperRight, yUpperRight);
	};

	Rectangle(const Rectangle & r){
		set(r.xLowerLeft, r.yLowerLeft, r.xUpperRight, r.yUpperRight);
	};


	inline
	void set(T xLowerLeft, T yLowerLeft, T xUpperRight, T yUpperRight){
		this->xLowerLeft  = xLowerLeft;
		this->yLowerLeft  = yLowerLeft;
		this->xUpperRight = xUpperRight;
		this->yUpperRight = yUpperRight;
	};


	inline
	T getWidth() const { return (xUpperRight - xLowerLeft); };

	inline
	T getHeight() const { return (yUpperRight - yLowerLeft); };

	inline
	T getArea() const { return std::abs(getWidth()*getHeight()); };


	/// This becomes the intersection of r and this.
	/**
	 *   Two way bounds are needed, because cropping max can be lower than this min.
	 */
	void crop(const Rectangle<T> & r){
		const Rectangle<T> bounds(*this);
		*this = r;
		limit(bounds.xLowerLeft, bounds.xUpperRight, xLowerLeft);
		limit(bounds.xLowerLeft, bounds.xUpperRight, xUpperRight);
		limit(bounds.yLowerLeft, bounds.yUpperRight, yLowerLeft);
		limit(bounds.yLowerLeft, bounds.yUpperRight, yUpperRight);
	}

	/// The instance extends to its union with r.
	void extend(const Rectangle & r);

	/// The instance reduces to itse intersection with r.
	void contract(const Rectangle & r);


	inline
	bool isInside(const T &x,const T &y) const {
		return ((x>xLowerLeft) && (x<xUpperRight) && (y>yLowerLeft) && (y<yUpperRight));
	};

	inline
	bool isOverLapping(const Rectangle &r) const {
		const bool xOverLap = !((r.xUpperRight < xLowerLeft) || (r.xLowerLeft > xUpperRight));
		const bool yOverLap = !((r.yUpperRight < yLowerLeft) || (r.yLowerLeft > yUpperRight));
		return (xOverLap && yOverLap);
	};

	/// Write corner points to a stream
	template <class S>
	inline
	void toStream(S & ostr, char separator=',') const {
		ostr << xLowerLeft << separator << yLowerLeft << separator <<  xUpperRight << separator << yUpperRight;
	}

	/// Return corner points to a string
	inline
	std::string toStr(char separator=',') const {
		std::stringstream sstr;
		toStream(sstr, separator);
		return sstr.str();
	}

	T xLowerLeft;
	T yLowerLeft;
	T xUpperRight;
	T yUpperRight;

protected:


	static
	inline
	void limit(const T & lowerBound, const T & upperBound, T & x){
		if (x < lowerBound)
			x = lowerBound;
		else if (x > upperBound)
			x = upperBound;
	}

	//mutable std::string toStr;
};


/*
template<class T> inline void Rectangle<T>::crop(const Rectangle & r)
{
	xLowerLeft  = std::max(xLowerLeft,r.xLowerLeft); // at least xLL
	//xLowerLeft  = std::min(xUpperRight,xLowerLeft);  // at max   xUR

	yLowerLeft  = std::max(yLowerLeft,r.yLowerLeft);
	//yLowerLeft  = std::min(yUpperRight, yLowerLeft);

	xUpperRight = std::min(xUpperRight, r.xUpperRight);
	//xUpperRight = std::max(xLowerLeft, xUpperRight);

	yUpperRight = std::min(yUpperRight,r.yUpperRight);

}
*/

template<class T> inline void Rectangle<T>::extend(const Rectangle & r)
{
	xLowerLeft  = std::min(xLowerLeft,r.xLowerLeft);
	yLowerLeft  = std::min(yLowerLeft,r.yLowerLeft);
	xUpperRight = std::max(xUpperRight,r.xUpperRight);
	yUpperRight = std::max(yUpperRight,r.yUpperRight);
}

template<class T> inline void Rectangle<T>::contract(const Rectangle & r)
{
	xLowerLeft  = std::max(xLowerLeft,r.xLowerLeft);
	yLowerLeft  = std::max(yLowerLeft,r.yLowerLeft);
	xUpperRight = std::min(xUpperRight,r.xUpperRight);
	yUpperRight = std::min(yUpperRight,r.yUpperRight);
}


template <class T>
std::ostream & operator<<(std::ostream &ostr,const drain::Rectangle<T> &r){
	ostr << r.xLowerLeft << ',' << r.yLowerLeft << ' ' <<  r.xUpperRight << ',' << r.yUpperRight;
	return ostr;
}


} // namespace drain

/*
template <class T>
std::ostream & operator<<(std::ostream &ostr,const drain::Rectangle<T> &r){
	ostr << r.xLowerLeft << ',' << r.yLowerLeft << ' ' <<  r.xUpperRight << ',' << r.yUpperRight;
	return ostr;
}
*/





#endif /* RECTANGLE_H_ */
