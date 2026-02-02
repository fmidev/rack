/*

MIT License

Copyright (c) 2023 FMI Open Development / Markus Peura, first.last@fmi.fi

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
 * TreeSVG.h
 *
 *  Created on: 2025
 *      Author: mpeura
 */

#ifndef DRAIN_TREE_ELEM_UTILS_SVG
#define DRAIN_TREE_ELEM_UTILS_SVG

//
/*
#include <drain/util/Frame.h>
#include <drain/util/EnumFlags.h>
#include <drain/util/Rectangle.h>

#include "AlignAnchorSVG.h"
#include "LayoutSVG.h"
*/

#include <drain/util/XML.h>
#include <drain/util/TreeXML.h>

#include "TreeSVG.h"

//namespace drain {

#define DRAIN_SVG_ELEM_CLS(E)         drain::NodeXML<drain::image::svg::tag_t>::Elem<drain::image::svg::tag_t::E>
#define DRAIN_SVG_ELEM(E)     typedef drain::NodeXML<drain::image::svg::tag_t>::Elem<drain::image::svg::tag_t::E> svg##E;

//#define DRAIN_SVG_SUPER(E) class drain::NodeXML<drain::image::svg::tag_t>::Elem<drain::image::svg::tag_t::E>; typedef drain::NodeXML<drain::image::svg::tag_t>::Elem<drain::image::svg::tag_t::E> svg##E;


namespace drain {



template <> template <>
class DRAIN_SVG_ELEM_CLS(RECT){
// class NodeXML<image::svg::tag_t>::Elem<image::svg::tag_t::RECT>{
public:

	inline
	Elem(image::NodeSVG & node) : node(node = image::svg::tag_t::RECT), x(node["x"]), y(node["y"]), width(node["width"]), height(node["height"]){
	};

	NodeXML<image::svg::tag_t> & node;

	FlexibleVariable & x;
	FlexibleVariable & y;
	FlexibleVariable & width;
	FlexibleVariable & height;
};
DRAIN_SVG_ELEM(RECT)



template <>
template <>
class DRAIN_SVG_ELEM_CLS(CIRCLE){

public:

	NodeXML<image::svg::tag_t> & node;

	inline
	Elem(image::NodeSVG & node) : node(node = image::svg::tag_t::CIRCLE), cx(node["cx"]), cy(node["cy"]), r(node["r"] = 0.0){
	};

	// Center point, x coord
	FlexibleVariable & cx;
	// Center point, y coord
	FlexibleVariable & cy;
	// Radius
	FlexibleVariable & r;

	// NodeXML<T> class could not access NodeSVG.box
    // Elem(image::NodeSVG & node) : node(node = image::svg::tag_t::CIRCLE), cx(node.box.x), cy(node.box.y), r(node["r"]){

};
DRAIN_SVG_ELEM(CIRCLE)



template <>
template <>
class DRAIN_SVG_ELEM_CLS(POLYGON){

public:

	inline
	Elem(image::NodeSVG & node) : node(node = image::svg::tag_t::POLYGON), points(node["points"]), writablePoints(node["points"]){
	};

	NodeXML<image::svg::tag_t> & node;

	const FlexibleVariable & points;

protected:

	FlexibleVariable & writablePoints;

public:

	void clear(){
		writablePoints.clear();
	}

	template <typename T>
	inline
	void append(const T &x, const T &y){
		writablePoints << x << ',' << y << ' ';
	}

	template <typename T>
	inline
	void append(drain::Point2D<T> &p){
		writablePoints << p.x << ',' << p.y << ' ';
	}

};
DRAIN_SVG_ELEM(POLYGON)



template <>
template <>
class DRAIN_SVG_ELEM_CLS(PATH){
//class svgPATH {
public:

	inline
	Elem(image::NodeSVG & node) : node(node = image::svg::tag_t::PATH), d(node["d"]) {
		//node[d].setType<std::string>();
		//node["d"].setSeparator(' ');
	};

	~Elem(){
		flush();
	}

	bool noReset = true;

	NodeXML<image::svg::tag_t> & node;

	// Path description, "d" attribute.
	const FlexibleVariable & d;

	enum Command {
		MOVE = 'M',
		LINE = 'L',
		// Cubic Bezier curves
		CURVE_C = 'C',
		CURVE_C_SHORT = 'S',
		// Quadratic Bezier curves
		CURVE_Q = 'Q',
		CURVE_Q_SHORT = 'T',
		CLOSE = 'Z',
	};

	enum Coord {
		ABSOLUTE = 0,
		RELATIVE = 32, // upper case 'm' -> 'M'
	};

public:

	template <Command C, typename ...T>
	void relative(const T... args){
		appendCmd<C,RELATIVE>();
		appendArgs<C>(args...);
	}

	template <Command C, typename ...T>
	void absolute(const T... args){
		appendCmd<C,ABSOLUTE>();
		appendArgs<C>(args...);
	}


	void clear(){
		sstr.str("");
		node["d"].clear();
	}

protected:

	mutable
	std::stringstream sstr;

	// Could be protected
	void flush(){
		// Logger mout(__FILE__, __FUNCTION__);
		// mout.warn("contents1: ", d);
		if (noReset){
			node["d"].append(sstr.str());
			// mout.warn("appended contents_: ", d);
		}
		else {
			node["d"] = sstr.str();
			// mout.warn("NEW contents_: ", d);
		}
		sstr.str(""); // Destructor would do this anyway
	}

	template <Command C, Coord R=RELATIVE>
	inline
	void appendCmd(){
		sstr << char(int(C) + int(R)) << ' ';
	}

	template <Command C>
	void appendArgs();

	template <Command C>
	void appendArgs(double x, double y);

	template <Command C>
	void appendArgs(double x, double y, double x2, double y2);

	template <Command C>
	void appendArgs(double x, double y, double x2, double y2, double x3, double y3);


	template <Command C, typename T>
	inline
	void appendArgs(const drain::Point2D<T> & p){
		appendArgs<C>(p.x, p.y);
	}

	template <Command C, typename T>
	inline
	void appendArgs(const drain::Point2D<T> & p, const drain::Point2D<T> & p2){
		appendArgs<C>(p.x, p.y, p2.x, p2.y);
	}

	template <Command C, typename T>
	inline
	void appendArgs(const drain::Point2D<T> & p, const drain::Point2D<T> & p2, const drain::Point2D<T> & p3){
		appendArgs<C>(p.x, p.y, p2.x, p2.y, p3.x, p3.y);
	}



	inline
	void appendPoint(double x){
		sstr << x << ' ';
	}

	template <typename T>
	inline
	void appendPoint(const drain::Point2D<T> & p){
		sstr << p.x << ' ' << p.y << ' ';
	}



	template <typename ...TT>
	inline
	void appendPoints(double x, double y, const TT... args){
		//appendPoint(p);
		sstr << x << ' ' << y;
		appendMorePoints(args...);
		//sstr << ' ';
	}

	template <typename T, typename ...TT>
	inline
	void appendPoints(const drain::Point2D<double> & p, const TT... args){
		sstr << p.x << ' ' << p.y;
		//appendPoint(p);
		appendMorePoints(args...);
	}

	template <typename ...TT>
	inline
	void appendMorePoints(const TT... args){
		sstr << ',';
		appendPoints(args...);
	}

	// Terminal function
	inline
	void appendMorePoints(){
		sstr << ' ';
	}




};
DRAIN_SVG_ELEM(PATH)


template <>
inline
void svgPATH::appendArgs<svgPATH::CLOSE>(){
};


template <>
inline
void svgPATH::appendArgs<svgPATH::MOVE>(double x, double y){
	appendPoints(x, y);
};

template <>
inline
void svgPATH::appendArgs<svgPATH::LINE>(double x, double y){
	appendPoints(x, y);
};

template <>
inline
void svgPATH::appendArgs<svgPATH::CURVE_C>(double x1, double y1, double x2, double y2, double x3, double y3){
	appendPoints(x1,y1, x2,y2, x3,y3);
};

template <>
inline
void svgPATH::appendArgs<svgPATH::CURVE_C_SHORT>(double x2, double y2, double x, double y){
	appendPoints(x2,y2, x,y);
};



namespace image {

}  // image::

}  // drain::


/**
 *
 *

    r = radius
    θ = central angle of the arc segment (radians)

    The curve starts at angle α and ends at α + θ.

    Start point:
    P0 = (r cos α, r sin α)

    End point:
    P3 = (r cos(α + θ), r sin(α + θ))

    Control points:
	P1 = P0 + k * (-r sin α, r cos α)
	P2 = P3 + k * (r sin(α + θ), -r cos(α + θ))

	where
	k = 4/3 * tan(θ / 4)

    But with notations
    px0 = r cos α,
    py0 = r sin α
    px3 = r cos (α + θ),
    py3 = r sin (α + θ)

    P0 = (px0, py0)
    P3 = (px3, py3)

    P1 = P0 + k* (-py0,  px0)
    P2 = P3 + k* ( py3, -px3)


 */



#endif // TREESVG_H_

