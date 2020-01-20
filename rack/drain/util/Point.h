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
#ifndef POINT_H_
#define POINT_H_


#include <vector>
#include <iostream>
#include <sstream>
#include <string>

namespace drain
{

//namespace image {

// using namespace std;


template <class T>
class Point2D
{
public:

	Point2D() : x(0), y(0)
	{
    };

	//Point2D(const T &x,const T &y) : x(x), y(y)
	Point2D(T x, T y) : x(x), y(y)
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
    Point2D<T> & setLocation(T2 x, T2 y){
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
std::ostream &operator<<(std::ostream &ostr,const drain::Point2D<T> &p)
{
	ostr << '[' << p.x << ',' << p.y << ']';
    return ostr;
}

template <class T>
std::ostream &operator<<(std::ostream &ostr,const drain::Point3D<T> &p)
{
	ostr << '[' << p.x << ',' << p.y << ',' << p.z << ']';
    return ostr;
}

// }

} // drain::


#endif /*POINT_H_*/
