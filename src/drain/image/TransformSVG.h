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
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#ifndef DRAIN_TRANSFORM_SVG
#define DRAIN_TRANSFORM_SVG

//#include "drain/util/EnumFlags.h"
#include "drain/UniTuple.h"
//#include "drain/util/FileInfo.h"
//#include "drain/util/Frame.h"
//#include "drain/util/SelectorXML.h"
//#include "drain/util/TreeXML.h"
//#include "AlignSVG.h"

namespace drain {

namespace image {


template <size_t N>
class Transform : public UniTuple<double,N> {

public:

	bool empty() const {
		return (*this == 0.0);
	}

	virtual inline
	void toStream(std::ostream & ostr) const override {
		ostr << '(';
		TupleBase<double,N>::toStreamFormatted(ostr, ' ');
		ostr << ')';
	}

};

}

}


template <size_t N>
std::ostream & operator<<(std::ostream & ostr, const drain::image::Transform<N> & tr){
	tr.toStream(ostr);
	return ostr;
}


namespace drain {

namespace image {


// Future option
class TransformSVG {

public:

	typedef double coord_t;

	TransformSVG();

	inline
	bool empty() const {
		return (rotate.empty() && scale.empty() && translate.empty() && matrix.empty()) ;
	}

	inline
	void setTranslate(const coord_t & x, const coord_t & y){
		// translate.ensureSize(2);
		translate.set(x,y);
	}

	inline
	void setTranslateX(const coord_t & x){
		// translate.ensureSize(1);
		// translate.set(x);
		translate[0] = x;
	}

	inline
	void setTranslateY(const coord_t & y){
		// translate.ensureSize(2);
		// svg::coord_t x = translate.get<svg::coord_t>(1);
		translate[1] = y;
		// translate.set(x, y);
	}

	/// Angle (deg), [x,y]


	struct Translate : public Transform<2> {

		double & x;
		double & y;

		inline
		Translate(double x=0.0, double y=0.0) : x(++next()=x), y(++next()=y){
		}

		inline
		Translate(const Translate & t) : x(++next()=t.x), y(++next()=t.y){
		}

	};


	Transform<3> rotate;
	Transform<2> scale;
	//Transform<2> translate;
	Translate translate;
	Transform<6> matrix;

	void toStream(std::ostream & ostr) const;

};


} // image::

} // drain::

inline
std::ostream & operator<<(std::ostream & ostr, const drain::image::TransformSVG & tr){
	tr.toStream(ostr);
	return ostr;
}

#endif

