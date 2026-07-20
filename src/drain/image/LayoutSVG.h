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
 *  Created on: Nov, 2024
 *      Author: mpeura
 */

#ifndef DRAIN_LAYOUT_SVG
#define DRAIN_LAYOUT_SVG

#include <drain/util/EnumFlagger.h>

#include "Direction.h"

#include "TreeSVG.h"

namespace drain {

namespace image {



/// Higher level controller for setting alignments.
/**
 *  Also applied by PanelSVG
 */
class LayoutSVG {

public:

	/// Direction for "Stacked", horziontally or vertically sequentially aligned layout
	/**
	 *  Typically, the axis is Align::Axis .
	 *
	 */
	enum Direction {
		UNDEFINED_DIRECTION=0,
		/*
		RIGHT = 1,
		LEFT  = 2,
		DOWN  = 4,
		UP    = 8,
		INCR = RIGHT | DOWN,
		DECR = LEFT | UP,
		*/
		/*
		INCR_HORZ = 1,
		INCR_VERT = 2,
		INCR = INCR_HORZ | INCR_VERT,
		DECR_HORZ = 4,
		DECR_VERT = 8,
		DECR = DECR_HORZ | DECR_VERT,
		*/
		INCR = 1,
		DECR = 2,
		/*
		INCR_HORZ = 1+4,
		DECR_HORZ = 2+4,
		INCR_VERT = 1+8,
		DECR_VERT = 2+8,
		*/
	};

	enum DirectionHorz {
		//UNDEFINED_DIRECTION=0,
		RIGHT = INCR,
		LEFT  = DECR,
	};

	enum DirectionVert {
		//tDEFINED_DIRECTION=0,
		DOWN  = INCR,
		UP    = DECR,
	};


	/// Experimental SVG style classes (requests)
	enum GroupType {
		STACK_LAYOUT, // Align elements in rows or columns. (Alternate the axis in nesting STACK_LAYOUT levels)
		ADAPTER,      // Translate this group object such that the upper right corner of graphics are in the origin (0,0).
		// ALIGN,        // Align this object, with applicable rules and preferences (populate me with align instructions, unless already set)
		COMPOUND,     // Internal elements are already aligned, bypass recursion.
		FIXED,        // Absolute position - do not align. (Do not adjust coordinates, but allow internal recursion.
		INDEPENDENT,  // No anchoring allowed – but inclusion by collective bounding box applies, unless NEUTRAL.
		NEUTRAL,      // Excluded from the collective bounding box – but anchoring supported, unless INDEPENDENT)
		//
		CROP,         // minimize bbox covering all the included objects. \see GroupType::FIXED
		// Future options
		// HEADER,       // Requests alignment as a title, typically title box combining TEXT (and TSPAN) on a background RECT
	};


	typedef drain::EnumFlagger<drain::SingleFlagger<Direction> > DirectionFlagger;
	typedef drain::EnumFlagger<drain::SingleFlagger<AlignBase::Axis> > AxisFlagger;

protected:

	AxisFlagger orientation = AlignBase::HORZ;

	DirectionFlagger direction = INCR;

	inline
	LayoutSVG(AlignBase::Axis v=AlignBase::HORZ, Direction d=INCR) : orientation(v), direction(d) {
	}

	inline
	LayoutSVG(const LayoutSVG & layout) : orientation(layout.orientation), direction(layout.direction){
	}

	/// Set orientation: horizontal or vertical axis
	/**
	 *   \tparam V - enum type AlignBase::Axis or string.
	 *   \param axis - axis as enum type (HORZ or VERT) or string.
	 */
	template <typename V>
	inline
	void setOrientation(const V & axis){
		orientation.set(Enum<AlignBase::Axis>::getValue(axis));
	};

	/// Set direction: coordinates increasing or decreasing
	/**
	 *   \tparam D  - enum type LayoutSVG::Direction or string.
	 *   \param dir - direction as enum type (INCR or DECR) or string.
	 */
	template <typename D>
	inline
	void setDirection(const D & dir){
		direction.set(Enum<LayoutSVG::Direction>::getValue(dir));
	};

	/// Set direction and orientation
	/**
	 *
	 */
	template <typename D, typename V>
	inline
	void set(const D & d, const V &v){
		direction.set(Enum<LayoutSVG::Direction>::getValue(d));
		orientation.set(Enum<AlignBase::Axis>::getValue(v));
	};

	/*
	static inline
	Direction flip(Direction ...){
	};
	*/
};


}  // image::

DRAIN_ENUM_DICT(image::LayoutSVG::Direction);
DRAIN_ENUM_OSTREAM(image::LayoutSVG::Direction);

DRAIN_ENUM_DICT(image::LayoutSVG::DirectionHorz);
DRAIN_ENUM_OSTREAM(image::LayoutSVG::DirectionHorz);

DRAIN_ENUM_DICT(image::LayoutSVG::DirectionVert);
DRAIN_ENUM_OSTREAM(image::LayoutSVG::DirectionVert);


DRAIN_ENUM_DICT(image::LayoutSVG::GroupType);

DRAIN_XML_ENUM_KEY(image::TreeSVG, image::LayoutSVG::GroupType);



}  // drain::


//DRAIN_ENUM_OSTREAM(drain::image::LayoutSVG::Direction);
DRAIN_ENUM_OSTREAM(drain::image::LayoutSVG::GroupType);

namespace drain {
//DRAIN_XML_ENUM_KEY(image::TreeSVG, image::LayoutSVG::GroupType);
}

#endif // DRAIN_ALIGN_SVG_H_

