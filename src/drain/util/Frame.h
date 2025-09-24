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

#include <stdexcept>
#include <ostream>
#include <limits>
#include <cmath> // isnan

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


template <class T>
struct Stub : public drain::UniTuple<T,2>{

	typedef T coord_t;
	coord_t & pos;
	coord_t & span;

	inline
	Stub(coord_t pos=0, coord_t span=0) : drain::UniTuple<T,2>(pos, span), pos(this->next()), span(this->next()){
	};

	inline
	Stub(const Stub & stub) : drain::UniTuple<T,2>(stub.tuple()), pos(this->next()), span(this->next()){
	};


};

DRAIN_TYPENAME_T0(Stub, T);

/*
template <class T>
struct StubHorz : public drain::UniTuple<T,2>{

	typedef T coord_t;
	coord_t & x;
	coord_t & width;

	inline
	StubHorz(coord_t x=0, coord_t width=0) : drain::UniTuple<T,2>(x, width), x(this->next()), width(this->next()){
	};

	inline
	StubHorz(const StubHorz & stub) : drain::UniTuple<T,2>(stub.tuple()), x(this->next()), width(this->next()){
	};


};

template <class T>
struct StubVert : public drain::UniTuple<T,2>{

	typedef T coord_t;
	coord_t & y;
	coord_t & height;

	inline
	StubVert(coord_t y=0, coord_t width=0) : drain::UniTuple<T,2>(x, width), x(this->next()), width(this->next()){
	};

	inline
	StubVert(const StubVert & stub) : drain::UniTuple<T,2>(stub.tuple()), x(this->next()), width(this->next()){
	};

};
*/


/// Something that has coordinates (x,y) and dimensions (width, height).
/**
 *
 */
template <class T>
struct Box : public drain::Point2D<T>, public drain::Frame2D<T> {

public:


	typedef T coord_t;


	inline
	Box(coord_t x=0, coord_t y=0, coord_t width=0, coord_t height=0) :
		drain::Point2D<coord_t>(x, y), drain::Frame2D<coord_t>(width, height)  {
	}

	inline
	Box(const Box & box) : drain::Point2D<coord_t>(box), drain::Frame2D<coord_t>(box)  {
	}

	static
	const coord_t undefined; //  = std::numeric_limits<coord_t>::max();

	inline
	void reset(){
		this->setArea(0,0);
		// max is "readable", whereas float-mins approach zero
		this->setLocation(undefined, undefined);
	}

	inline
	void expand(coord_t x, coord_t y){
		expandHorz(x);
		expandVert(y);
	}

	inline
	void expand(const drain::Point2D<coord_t> & p){
		expandHorz(p.x);
		expandVert(p.y);
	}

	/// Update this box such that it contains the given \c bbox .
	inline
	void expand(const Box & box){
		expandHorz(box);
		expandVert(box);
	}

	inline
	void expandHorz(const Box & box){
		expandHorz(box.x);
		expandHorz(box.x + box.width);
		// box.y + box.height);
	}

	inline
	void expandVert(const Box & box){
		expandVert(box.y);
		expandVert(box.y + box.height);
	}

	void expandHorz(coord_t x){
		//if (this->x != undefined){
		if (!isUndefined(this->x)){
			const coord_t xMax = std::max(x, this->x + this->width);
			this->x = std::min(this->x, x);
			this->width = xMax - this->x;
		}
		else {
			this->x = x;
			this->width = 0; // probably was already, but ensure
		}
	}

	void expandVert(coord_t y){
		//if (this->y != undefined){
		if (!isUndefined(this->y)){
			const coord_t yMax = std::max(y, this->y + this->height);
			this->y = std::min(this->y, y);
			this->height = yMax - this->y;
		}
		else {
			this->y = y;
			this->height = 0; // probably was already, but ensure
		}
	}



	drain::Point2D<coord_t> & getLocation(){
		return *this;
	};

	const drain::Point2D<coord_t> & getLocation() const {
		return *this;
	};

	drain::Frame2D<coord_t> & getFrame(){
			return *this;
	};

	const drain::Frame2D<coord_t> & getFrame() const {
			return *this;
	};

protected:

	inline
	bool isUndefined(coord_t coord){
		return std::isnan(coord);
	}

};

DRAIN_TYPENAME_T(Box, T);


template <class T>
// const T Box<T>::undefined = std::numeric_limits<T>::max();
const T Box<T>::undefined = std::numeric_limits<T>::quiet_NaN();


template <class T>
std::ostream &operator<<(std::ostream &ostr,const drain::Box<T> &box)
{
	ostr << (const drain::Point2D<T> &)box  << ' ' <<  (const drain::Frame2D<T> &)box; // [' << p.x << ',' << p.y << ',' << p.z << ']';
    return ostr;
}

} // drain

#endif
