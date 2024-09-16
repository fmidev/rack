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


//#include <vector>
//#include <iostream>
//#include <sstream>
//#include <string>

#include <drain/UniTuple.h>
#include <drain/Type.h>

namespace drain
{


template <class T>
struct Point2D : public drain::UniTuple<T,2> {

	T &x;
	T &y;

	Point2D(T x=0, T y=0) : x(this->next()), y(this->next()){
		this->set(x, y);
	};

	Point2D(const Point2D & p) : x(this->next()), y(this->next()){
		this->set(p.tuple());
	};

	// Reference
	template <size_t N>
	Point2D(drain::UniTuple<T,N> & tuple, size_t i) :
		drain::UniTuple<T,2>(tuple, i),  // start in new pos!
		x(this->next()), y(this->next()){
	};

	Point2D & operator=(const Point2D & p){
		this->set(p.tuple());
		return *this;
	}

	// set(x, y);
    template <class T2>
    Point2D<T> & setLocation(T2 x, T2 y){
    	this->set(x, y);
    	// this->x = static_cast<T>(x);
    	// this->y = static_cast<T>(y);
    	return *this;
    }

    template <class T2>
    Point2D<T> & setLocation(const Point2D<T2> & p){
    	this->set(p);
    	// this->x = static_cast<T>(p.x);
    	// this->y = static_cast<T>(p.y);
    	return *this;
    }

};

template <class T>
struct Point3D : public drain::UniTuple<T,3> {

	T &x;
	T &y;
	T &z;

	Point3D(T x=0, T y=0, T z=0) : x(this->at(0)=x), y(this->at(1)=y), z(this->at(2)=z){
	};

	Point3D(const Point3D & p) : x(this->at(0)=p.x), y(this->at(1)=p.y), z(this->at(2)=p.z){
	};

	Point3D & operator=(const Point3D & p){
		this->set(p.tuple());
		return *this;
	}

	// Consider generalized (for smaller tuples?)
	template <class T2>
	Point3D & operator=(const drain::UniTuple<T2,3> & p){
		this->set(p.tuple());
		return *this;
	}

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


};

//template <class T>
DRAIN_TYPENAME_T(Point2D, T);

//template <class T>
DRAIN_TYPENAME_T(Point3D, T);


/*
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
*/

// }

} // drain::


#endif /*POINT_H_*/
