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

#ifndef DRAIN_FRAME_H_
#define DRAIN_FRAME_H_

#include <ostream>
#include <stdexcept>

#include <drain/Log.h>
#include <drain/UniTuple.h>
#include <drain/Type.h>
#include <drain/util/Point.h> // for Box
//#include "drain/util/Range.h"


namespace drain {

/// Something that has \c width and \c height.
/**
 *   \see drain::Rectangle
 *   \see drain::Point2D
 */
template <class T>
class Frame2D : public drain::UniTuple<T,2> {

public:

	T & width;
    T & height;

	inline
	Frame2D(T width=0, T height=0) : width(this->next()), height(this->next()){
		this->set(width, height?height:width);
	};

	Frame2D(const Frame2D<T> & geometry)  : width(this->next()), height(this->next()) {
		this->set(geometry.width, geometry.height);
	}

	template <class T2>
	Frame2D(const Frame2D<T2> & geometry)  : width(this->next()), height(this->next()) {
		this->set(geometry.width, geometry.height);
	}

	// Reference, N>=2
	template <size_t N>
	// Frame2D(drain::UniTuple<T,N> & tuple, T i) : // WHY T i, not int index type?size_t
	Frame2D(drain::UniTuple<T,N> & tuple, size_t i) : // 2025 fixed
	drain::UniTuple<T,2>(tuple, i),
	width(this->next()),
	height(this->next()){
		//updateTuple();
	};

	virtual ~Frame2D(){};


	Frame2D & operator=(const Frame2D & geometry){
		this->set(geometry.width, geometry.height);
		//this->assign(geometry);
		return *this;
	}


	template <class T2>
	inline
	Frame2D & operator=(const T2 & frame){
		this->set(0,0); // could be init!
		this->assign(frame);
		return *this;
	}


	inline
	void setWidth(T w){
		width = w;
		// updateTuple();
	}

	inline
	void setHeight(T h){
		height = h;
		// updateTuple();
	}

	inline
	void setArea(T w, T h){
		width = w;
		height = h;
	}

	void setArea(const drain::UniTuple<T,2> & tuple){
		width  = tuple[0];
		height = tuple[1];
	}




    inline
    T getWidth() const {
    	return width;
    };

    inline
    T getHeight() const {
    	return height;
    };

	inline
	T getArea() const {
		return width*height;
	};

	inline
	bool empty() const {
		return (width==0) || (height==0);
	};



};

DRAIN_TYPENAME_T(Frame2D, T);

/// Something that has coordinates (x,y) and dimensions (width, height).
/**
 *
 */
template <class T>
struct Box : public drain::Point2D<T>, public drain::Frame2D<T> {
// struct Box : public drain::UniTuple<T,4> {

public:

	/*
	typedef drain::UniTuple<T,4> tuple_t;
	drain::Point2D<T> location;
	drain::Frame2D<T> frame;

	inline
	Box(T x=0, T y=0, T width=0, T height=0) : tuple_t(), location((tuple_t &)*this, 0), frame((tuple_t &)*this, 2)  {
		location.setLocation(x, y);
		frame.setArea(width, height);
	}

	inline
	Box(const Box<T> & box) : tuple_t(), location((tuple_t &)*this, 0), frame((tuple_t &)*this, 2){
		location.setLocation(box.location);
		frame.setArea(box.frame);
	}
	*/


	inline
	Box(T x=0, T y=0, T width=0, T height=0) : drain::Point2D<T>(x, y), drain::Frame2D<T>(width, height)  {
	}

	inline
	Box(const Box & box) : drain::Point2D<T>(box), drain::Frame2D<T>(box)  {
	}


	/// Update this box such that it contains the given \c bbox .
	void expand(const Box & box){
		/*
		const T xMax = std::max(this->x + this->width,  box.x+box.width);
		const T yMax = std::max(this->y + this->height, box.y+box.height);
		this->x = std::min(this->x, box.x);
		this->y = std::min(this->y, box.y);
		this->width  = xMax - this->x;
		this->height = yMax - this->y;
		expand(box.x, box.y);
		*/
		expand(box.x, box.y);
		expand(box.x + box.width, box.y + box.height);
	}

	void expand(const T & x, const T & y){
		// Logic fails if width or height is negative?
		T m;

		m = std::max(this->x + this->width,  x);
		this->x = std::min(this->x, x);
		this->width  = m - this->x;

		m = std::max(this->y + this->height, y);
		this->y = std::min(this->y, y);
		this->height = m - this->y;

	}

	inline
	void expand(const drain::Point2D<T> & p){
		expand(p.x, p.y);
	}

	drain::Point2D<T> & getLocation(){
		return *this;
	};

	const drain::Point2D<T> & getLocation() const {
		return *this;
	};

	drain::Frame2D<T> & getFrame(){
			return *this;
	};

	const drain::Frame2D<T> & getFrame() const {
			return *this;
	};

};

DRAIN_TYPENAME_T(Box, T);


template <class T>
std::ostream &operator<<(std::ostream &ostr,const drain::Box<T> &box)
{
	ostr << (const drain::Point2D<T> &)box  << ' ' <<  (const drain::Frame2D<T> &)box; // [' << p.x << ',' << p.y << ',' << p.z << ']';
    return ostr;
}

} // drain

#endif
