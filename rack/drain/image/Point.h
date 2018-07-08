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
#ifndef POINT_H_
#define POINT_H_


#include <vector>
#include <iostream>
#include <sstream>
#include <string>

namespace drain
{

namespace image
{

// using namespace std;


template <class T>
class Point2D
{
public:

	Point2D() : x(0), y(0)
	{
    };

	Point2D(const T &x,const T &y) : x(x), y(y)
	{
    };


	template <class T2>
	bool operator==(const Point2D<T2> &other) const
	{
		if ((*this).x == other.x && (*this).y == other.y)
			return true;
		return false;
	}

	template <class T2>
	bool operator!=(const Point2D<T2> &other) const
	{
		return !((*this) == other);
	}

    template <class T2>
    Point2D<T> & setLocation(const T2 & x, const T2 & y){
    	this->x = static_cast<T>(x);
    	this->y = static_cast<T>(y);
    	return *this;
    }

    template <class T2>
    Point2D<T> & setLocation(const Point2D<T2> & p){
    	this->x = static_cast<T>(p.x);
    	this->y = static_cast<T>(p.y);
    	return *this;
    }


    T x;
    T y;


};







//-----------------------------------------

template <class T>
class Point3D : public Point2D<T>
{
public:

	Point3D() : Point2D<T>(), z(0)
	{
    };

    Point3D(const T &x, const T &y, const T &z) : Point2D<T>(x,y), z(z) 
    {
    };
    
	template <class T2>
    void setLocation(const T2 & x, const T2 & y,  const T2 & z){
    	this->x = static_cast<T>(x);
    	this->y = static_cast<T>(y);
    	this->z = static_cast<T>(z);
    }

    template <class T2>
    void setLocation(const Point3D<T2> & p){
    	*this = p;	
    }

    T z;
};





template <class T>
std::ostream &operator<<(std::ostream &ostr,const drain::image::Point2D<T> &p)
{
	ostr << '[' << p.x << ',' << p.y << ']';
    return ostr;
}

template <class T>
std::ostream &operator<<(std::ostream &ostr,const drain::image::Point3D<T> &p)
{
	ostr << '[' << p.x << ',' << p.y << ',' << p.z << ']';
    return ostr;
}



}
}


#endif /*POINT_H_*/
