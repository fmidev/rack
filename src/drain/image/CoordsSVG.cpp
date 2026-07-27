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
 * TreeSVG.cpp
 *
 *      Author: mpeura
 */

#include "CoordsSVG.h"

namespace drain {

namespace image {

bool CoordSpanBase::getPosition(AlignBase::Pos alignLoc, svg::coord_t & coord) const {

	Logger mout(__FILE__, __FUNCTION__);

	switch (alignLoc){

	case AlignBase::Pos::MIN:
		coord = pos; // "+ 0%"
		return true;

	case AlignBase::Pos::MID:
		coord = pos + span/2.0; // " + 50%"
		return true;

	case AlignBase::Pos::MAX:
		coord = pos + span; // " + 100%"
		return true;

	case AlignBase::Pos::FILL:
		// Maybe ok, since GROUP can be an anchor but also
		// mout.suspicious<LOG_WARNING>("Alignment:: ANCHOR has fill request: HORZ FILL");
		break;
	case AlignBase::Pos::UNDEFINED_POS:  // -> consider MID or some absolute value, or margin. Or error:
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', AlignBase::Axis::HORZ, '=', pos);
		break;
	default:
		// assert undefined value.
		mout.unimplemented<LOG_ERR>("Alignment::Pos: ", (int)alignLoc);
	}

	return false;

}

template <>
// inline
void CoordSpan<AlignBase::Axis::HORZ>::copyFrom(const BBoxSVG & bbox){
	if (bbox.isDefined()){
		pos  = bbox.x;
		span = bbox.width;
	}
};

template <>
// inline
void CoordSpan<AlignBase::Axis::VERT>::copyFrom(const BBoxSVG & bbox){
	if (bbox.isDefined()){
		pos  = bbox.y;
		span = bbox.height;
	}
};


template <>
// inline
void CoordSpan<AlignBase::Axis::HORZ>::copyFrom(const NodeSVG & node){
	const BBoxSVG & bbox = node.getBoundingBox();
	if (bbox.isDefined()){
		CoordSpan<AlignBase::Axis::HORZ>::copyFrom(bbox);
		if (!node.typeIs(svg::TEXT)){
			pos += node.transform.translate.x;
		}
	}
};

template <>
// inline
void CoordSpan<AlignBase::Axis::VERT>::copyFrom(const NodeSVG & node){
	const BBoxSVG & bbox = node.getBoundingBox();
	if (bbox.isDefined()){
		CoordSpan<AlignBase::Axis::VERT>::copyFrom(bbox);
		if (!node.typeIs(svg::TEXT)){
			pos += node.transform.translate.y;
		}
	}
};

/* UNUSED?
template <AlignBase::Axis AX>
void expandBBox(BBoxSVG & bbox, CoordSpan<AX> & anchorSpan){
	Logger(__FILE__, __FUNCTION__).error("Unimplemented method");
}

template <>
void expandBBox(BBoxSVG & bbox, CoordSpan<AlignBase::Axis::HORZ> & anchorSpan){
	bbox.expandHorz(anchorSpan.pos);
	bbox.expandHorz(anchorSpan.pos + anchorSpan.span);
}

template <>
void expandBBox(BBoxSVG & bbox, CoordSpan<AlignBase::Axis::VERT> & anchorSpan){
	bbox.expandVert(anchorSpan.pos);
	bbox.expandVert(anchorSpan.pos + anchorSpan.span);
}
*/


}  // image::


}  // drain::

/*
template <drain::image::AlignBase::Axis AX>
std::ostream & operator<<(std::ostream & ostr, const drain::image::CoordSpan<AX> &span){
	ostr << "span" << AX << "=" << span.pos << "(+" << span.span <<  ')';
	return ostr;
}
*/


