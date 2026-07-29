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

#ifndef DRAIN_ALIGN_SVG
#define DRAIN_ALIGN_SVG

#include <string>

#include "drain/Enum.h"
#include "Alignment.h"


namespace drain {

namespace image {


/// User-friendly programming interface for alignment considering two elements.
/**
 *   Designed to be contained by SVG elements.
 *   Considers two elements, or more specifically, the bounding boxes of two elements.
 *
 *   TODO: insert CompleteAlign above this
 */
struct AlignSVG { // : protected Align {


	virtual
	~AlignSVG(){};

	typedef Alignment<const AlignBase::Axis, AlignBase::Axis::HORZ> HorzAlign;

	/// Alias for {HORZ:MIN}
	static
	const HorzAlign LEFT;

	/// Alias for {HORZ:MID}
	static
	const HorzAlign CENTER;

	/// Alias for {HORZ:MAX}
	static
	const HorzAlign RIGHT;

	/// Alias for {HORZ:FILL}
	static
	const HorzAlign HORZ_FILL;

	static
	const HorzAlign UNDEFINED_HORZ;

	// ----------------------

	typedef Alignment<const AlignBase::Axis, AlignBase::Axis::VERT> VertAlign;

	/// Alias for {VERT:MIN}
	static
	const VertAlign TOP;

	/// Alias for {VERT:MID}
	static
	const VertAlign MIDDLE;

	/// Alias for {VERT:MAX}
	static
	const VertAlign BOTTOM;

	/// Alias for {VERT:FILL}
	static
	const VertAlign VERT_FILL;

	static
	const VertAlign UNDEFINED_VERT;


	enum Owner {
		OBJECT = 0, // 0b00001111,
		ANCHOR = 1, // 0b11110000,
	};


	/// Moved from Layput
	enum AlignClass {
		COMPOUND,     // Completed already. Internal elements are already aligned, bypass recursion.
		FIXED,        // Absolute position - do not change bbox, but traverse (align) children. (Do not adjust coordinates, but allow internal recursion.
		INDEPENDENT,  // No anchoring allowed – but inclusion by collective bounding box applies, unless NEUTRAL.
		NEUTRAL,      // Excluded from the collective bounding box – but anchoring supported, unless INDEPENDENT)
	};


	/// Low-level, "atomic" setter of alignment for OBJECT itself or its ANCHOR object.
	/*
	 *  Notice that for an alignment to be complete, this function should be called twice:
	 *  setting partial alignment for both owner=OBJECT and owner=ANCHOR .
	 *
	 *  \tparam OBJ  - enum type \c Owner  or string
	 *  \tparam A    - enum type \c Axis or string
	 *  \tparam V    - enum type \c Alignment or string
	 *  \param pos   - enum value \c OBJ or \c REF
	 *  \param axis  - enum value \c HORZ or \c VERT
	 *  \param value - enum value \c MAX , \c MID , or \c MIN (or string)
	 */
	template <typename OBJ, typename A, typename V>
	inline // in problems, rename this function, ie. remove polymorphism
	void setAlign(const OBJ & owner, const A & axis,  const V &value){
		modifyAlign(owner, axis, value);
		updateAlign();
	}

	/// Set a single alignment setting. "Intermediate-level": axis and pos are given separately.
	/**
	 *
	 *  \param topol  - \c INSIDE or \c OUTSIDE
	 *  \param axis   - \c HORZ or \c VERT
	 *  \param pos    - \c MIN , \c MID , or \c MAX
	 *
	 *  Used by file-svg.cpp
	 *
	 */
	inline
	void setAlign(const AlignBase::Axis & axis, const AlignBase::Pos & pos, MutualAlign::Topol topol=MutualAlign::Topol::INSIDE){
		if (pos != AlignBase::FILL){
			modifyAlign(ANCHOR, axis, pos);
		}
		modifyAlign(OBJECT, axis, (topol==MutualAlign::INSIDE) ? pos : AlignBase::flip(pos));
		updateAlign();
	}

	/// NEW High-level, user friendlier interface for setting INSIDE the alignments for both OBJECT itself and its ANCHOR object.
	/*
	 *  Template supports empty arg list.
	 *
	 *  \tparam T  - enum type \c Topol or string
	 *  \tparam AX - axis enum type: Axis or const Axis
	 *  \tparam A  - axis enum value: HORZ, VERT or UNDEFINED_AXIS
	 *  \param align  - \c HorzAlign or \c VertAlign
	 */
	template <typename ...T, typename AX, AlignBase::Axis A>
	void setAlign(const Alignment<AX,A> & align, const T...  args){
		if (align.pos == AlignBase::FILL){
			// Makes sense only for OBJECT, as it will be changed (and ANCHOR is never unchanged).
			setAlign(AlignSVG::OBJECT, align.axis, align.pos);
		}
		else {
			setAlign(align.axis, align.pos, args...);
		}
	}

	// Convenience: set both horz and vert alignments (INSIDE)
	/*
	 */
	template <typename AX1, AlignBase::Axis A1, typename AX2, AlignBase::Axis A2>
	void setAlign(const Alignment<AX1,A1> & align1, const Alignment<AX2,A2> & align2){
		setAlign(align1.axis, align1.pos, MutualAlign::Topol::INSIDE);
		setAlign(align2.axis, align2.pos, MutualAlign::Topol::INSIDE);
	}

	// Convenience: set both horz and vert alignments (INSIDE)
	/*
	 */
	template <typename AX1, AlignBase::Axis A1> // , typename AX2, AlignBase::Axis A2>
	void setAlign(const CompleteAlignment<AX1,A1> & align1){
		setAlign(align1.axis, align1.pos, align1.topol);
		//setAlign(align2.axis, align2.pos, MutualAlign::Topol::INSIDE);
	}


	/// Compiler trap: unimplemented for two of same kind: either \c HorzAlign or \c VertAlign twice.
	template <typename AX, AlignBase::Axis A>
	void setAlign(const Alignment<AX,A> & align1, const Alignment<AX,A> & align2);


	/// High-level, user friendlier interface for setting the alignments for both OBJECT itself and its ANCHOR object.
	/*
	 *  \tparam T - enum type \c Topol or string: \c INSIDE or \c OUTSIDE .
	 *  \param align  - Horizontal or vertical Alignment: \c LEFT|CENTER|RIGHT or \c TOP|MIDDLE|BOTTOM
	 *  \param topol  - \c INSIDE or \c OUTSIDE
	 */
	template <typename T>
	void setAlign(const std::string & align, const T & topol){
		const Alignment<> & a = Enum<Alignment<> >::getValue(align, false);
		const MutualAlign::Topol & t = Enum<MutualAlign::Topol>::getValue(topol, false);
		//const Alignment<> & a = Enum<Alignment<> >::getValue(align, false);
		setAlign(a.axis, a.pos, t);
	}

	/* "TODO"
	void setAlign(const CompleteAlignment<> & align){
	}
	*/

	/// High-level, user friendly interface for setting the alignments for both OBJECT itself and its ANCHOR object.

	/// Handler for command line or configuration file arguments
	void setAlign(const std::string & align);
	// Note: no mixed type, ANCHOR:LEFT

	void setAlign(AlignClass cls){
		setAlignClass(cls);
	}

protected:
	virtual
	void setAlignClass(AlignClass cls) = 0;

public:
	/// Returns true, if any setting is set...
	bool isAligned() const;

	void resetAlign();

	void swapAlign(AlignSVG & align);

	/// Return alignment setting of an object along horizontal or vertical axis  .
	/*
	 *  \tparam P - enum type Owner \c REF or \c OBJ , or respective string.
	 *  \tparam A - enum type axis_t \c HORZ or \c VERT , or respective string.
	 *  \param pos - target object \c OBJ or referred anchor object \c REF
	 *  \param axis - horizontal \c HORZ or vertical \c AXIS .
	 */
	template <typename P, typename A>
	AlignBase::Pos & getAlignPos(const P & pos, const A & axis);


	/// Return alignment setting of an object along horizontal or vertical axis  .
	/*
	 *  \tparam P - enum type Owner \c REF or \c OBJ , or respective string.
	 *  \tparam A - enum type axis_t \c HORZ or \c VERT , or respective string.
	 *  \param pos - target object \c OBJ or referred anchor object \c REF
	 *  \param axis - horizontal \c HORZ or vertical \c AXIS .
	 *
	 */
	template <typename P, typename A>
	const AlignBase::Pos & getAlignPos(const P & pos, const A & axis) const;

	/**
	 *  For some reason, toStream() conflicts with: drain::NodeXML<T>::toStream(std::ostream&, const T&, const string&, int)
	*/
	void confToStream(std::ostream & ostr) const;

	inline
	const std::string & getAlignStr() const {
		return alignStr;
	}

	void swapAlign();
	/*
	/// Checks if at least some combination of (ANCHOR|OBJECT) × (HORZ|VERT) × (LEFT|OBJECT),
	template <typename ...TT>
	bool isAlignSet(const TT... args) const {
		bitvect_t v = combineAlign(args...);
		return (alignment & v) == v;
	}

	*/

protected:

	// Future extension
	typedef int bitvect_t;  // or int_t ?

	// Future extension
	mutable
	bitvect_t alignment = 0;

	std::string alignStr;

	/// Change alignment configuration without updating the alignStr.
	template <typename P, typename A, typename V>
	void modifyAlign(const P & owner, const A & axis,  const V &value){
		getAlignPos(owner, axis) = Enum<AlignBase::Pos>::getValue(value, false);
	}


	void updateAlignStr();

	virtual inline
	void updateAlign(){
		updateAlignStr();
	};


	typedef std::vector<AlignBase::Pos> align_vect_t;
	typedef std::vector<align_vect_t > align_conf_t;

	align_conf_t alignments = align_conf_t(2, align_vect_t(2, AlignBase::Pos::UNDEFINED_POS));

	/// alignments[OBJECT][VERT] = alignments[OBJECT|ANCHOR][HORZ|VERT] = [MIN|MID|MAX]


};

}

// const drain::Enum<AlignSVG::Owner>::dict_t drain::Enum<AlignSVG::Owner>::dict;
DRAIN_ENUM_DICT(image::AlignSVG::Owner);
DRAIN_ENUM_DICT(image::AlignSVG::AlignClass);
DRAIN_ENUM_DICT(image::Alignment<>);

DRAIN_ENUM_DICT(image::AlignSVG::HorzAlign);
DRAIN_ENUM_DICT(image::AlignSVG::VertAlign);

// NEW

}

//DRAIN_ENUM_OSTREAM(drain::image::Alignment<>);

// template <>

// DRAIN_ENUM_OSTREAM(drain::image::Alignment<>);

namespace drain {

DRAIN_ENUM_OSTREAM(drain::image::AlignSVG::Owner);
DRAIN_ENUM_OSTREAM(drain::image::AlignSVG::AlignClass);


namespace image {

DRAIN_ENUM_OSTREAM(drain::image::Alignment<>);
DRAIN_ENUM_OSTREAM(drain::image::AlignSVG::HorzAlign);
DRAIN_ENUM_OSTREAM(drain::image::AlignSVG::VertAlign);

template <>
inline
void AlignSVG::HorzAlign::reset(){
	// axis = AlignBase::Axis::UNDEFINED_AXIS;
	pos  = AlignBase::Pos::UNDEFINED_POS;
}

template <>
inline
void AlignSVG::VertAlign::reset(){
	// axis = AlignBase::Axis::UNDEFINED_AXIS;
	pos  = AlignBase::Pos::UNDEFINED_POS;
}


/// Dictionary of horizontal (LEFT,CENTER,RIGHT) flags
/*
template <>
const drain::Enum<AlignSVG::HorzAlign>::dict_t  drain::Enum<AlignSVG::HorzAlign>::dict;

/// Dictionary of vertical (TOP,MIDDLE,BOTTOM) flags
template <>
const drain::Enum<AlignSVG::VertAlign>::dict_t  drain::Enum<AlignSVG::VertAlign>::dict;

/// Dictionary combining horizontal (LEFT,CENTER,RIGHT) and vertical (TOP,MIDDLE,BOTTOM) flags
template <>
const drain::Enum<Alignment<> >::dict_t  drain::Enum<Alignment<> >::dict;
*/




template <typename OBJ, typename A>
AlignBase::Pos & AlignSVG::getAlignPos(const OBJ & owner, const A & axis){
	const AlignSVG::Owner p = Enum<AlignSVG::Owner>::getValue(owner, false); // raise error
	const AlignBase::Axis a = Enum<AlignBase::Axis>::getValue(axis, false); // raise error
	return alignments[p][a];
}

template <typename OBJ, typename A>
const AlignBase::Pos & AlignSVG::getAlignPos(const OBJ & owner, const A & axis) const {
	const AlignSVG::Owner p  = Enum<AlignSVG::Owner>::getValue(owner, false); // raise error
	const AlignBase::Axis a  = Enum<AlignBase::Axis>::getValue(axis, false); // raise error
	return alignments[p][a];
}


}  // image::


DRAIN_TYPENAME(image::AlignSVG::HorzAlign);
DRAIN_TYPENAME(image::AlignSVG::VertAlign);

// NEW
/*
DRAIN_ENUM_DICT(image::AlignSVG::AlignClass);
DRAIN_ENUM_DICT(image::Alignment<>);
*/

}  // drain::

// DRAIN_ENUM_OSTREAM(drain::image::Alignment<>);
// DRAIN_ENUM_OSTREAM(drain::image::AlignSVG::Owner);


inline
std::ostream & operator<<(std::ostream &ostr, const drain::image::AlignSVG & align){
	//return ostr << align.axis << '_' << align.pos; // enums resolved above
	align.confToStream(ostr);
	return ostr; //  << "UNDER CONSTR...";  // RESOLVE!
}



#endif // DRAIN_ALIGN_SVG_H_

