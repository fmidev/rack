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

//#include "drain/util/FileInfo.h"
// #include "drain/util/Frame.h"
//
#include <string>
#include "drain/util/EnumFlags.h"

namespace drain {

namespace image {


/// Low level alignment instructions
struct AlignBase {

public:

	/// Reference position at Axis (HORZ or VERT)
	enum Pos {
		UNDEFINED_POS = 0,
		MIN = 1, // LEFT  (if HORZ) or TOP    (when VERT) 0b01010101,
		MAX = 2, // RIGHT (if HORZ) or BOTTOM (when VERT) 0b10101010,
		MID = 3, // 0b11111111,
		FILL = 4, /// Maximize, with respect to HORZ or VERT extent. Currently, for OBJECT only.
		// ABSOLUTE?
	};

	enum Axis {
		// Index, must contain 0 and 1 for HORZ and VERT
		HORZ      = 0, // 0b00110011,
		VERT      = 1, // b11001100,
		UNDEFINED_AXIS = 2,
	};

	static inline
	Axis flip(Axis axis){
		switch (axis){
		case HORZ:
			return AlignBase::VERT;
		case VERT:
			return HORZ;
		default:
			return UNDEFINED_AXIS;
		}
	};

	//typedef drain::EnumDict<Coord>::dict_t pos_dict_t;

	static inline
	AlignBase::Pos flip(AlignBase::Pos pos){
		switch (pos){
		case MAX:
			return MIN;
		case MIN:
			return MAX;
		case MID:
		case FILL:
		case UNDEFINED_POS:
		default:
			return pos;
		}
	};

};


template <>
const drain::EnumDict<AlignBase::Axis>::dict_t drain::EnumDict<AlignBase::Axis>::dict;
DRAIN_ENUM_OSTREAM(AlignBase::Axis);

template <>
const drain::EnumDict<AlignBase::Pos>::dict_t drain::EnumDict<AlignBase::Pos>::dict;
DRAIN_ENUM_OSTREAM(AlignBase::Pos);


/// Container for Axis and Pos.
/**
 *   \tparam AX - const or non-const Align::Axis
 *   \tparam  A - axis init value AlignBase::Axis::HORZ or AlignBase::Axis::VERT
 *
 *
 *   For \c const implementations, see AlignSVG
 *   Alignment<Align::Axis::HORZ>
 *   Alignment<Align::Axis::VERT>
 */
template <typename AX = AlignBase::Axis, AlignBase::Axis A = AlignBase::Axis::UNDEFINED_AXIS, typename POS = AlignBase::Pos> // , Align::Coord POS = Align::Coord::UNDEFINED_POS>
struct Alignment {

	// Align::Axis
	AX axis; // = V; // compiler error if different type?

	//AlignBase::Pos pos
	POS pos = AlignBase::Pos::UNDEFINED_POS; // or middle?

	inline
	Alignment(AlignBase::Pos pos = AlignBase::Pos::UNDEFINED_POS) : axis(A), pos(pos){
	}

	/*
	inline
	Alignment(AlignBase::Axis axis, AlignBase::Pos pos = AlignBase::Pos::UNDEFINED_POS) : axis(axis), pos(pos){ // axis(axis),
	}
	*/

	inline
	Alignment(const Alignment & ac) : axis(ac.axis), pos(ac.pos){
	}

	template <typename AX2, AlignBase::Axis A2>
	inline
	Alignment(const Alignment<AX2,A2> & align) : pos(align.pos){ // axis(ac.axis),
		axis = align.axis; // error if const
	}

	virtual inline
	bool isSet() const {
		return (axis != AlignBase::Axis::UNDEFINED_AXIS) && (pos != AlignBase::Pos::UNDEFINED_POS);
	}

	virtual inline
	void reset(){
		axis = AlignBase::Axis::UNDEFINED_AXIS;
		pos  = AlignBase::Pos::UNDEFINED_POS;
	}


	template <typename AX2, AlignBase::Axis A2>
	inline
	bool operator==(const Alignment<AX2,A2> & align) const {
		return (align.axis == axis) && (align.pos == pos);
		// return compare(ad) == 0;
	}


};





template <typename AX, AlignBase::Axis V>
inline
std::ostream & operator<<(std::ostream &ostr, const Alignment<AX,V> & align){
	//return ostr << align.axis << '_' << align.pos; // enums resolved above
	return ostr << align.axis << '_' << align.pos;
}




//struct Alignment2;

/// User-friendly programming interface for alignment considering two elements.
/**
 *   Designed to be contained by SVG elements.
 *   Considers two elements, or more specifically, the bounding boxes of two elements.
 *
 *
 */
struct AlignSVG { // : protected Align {


	virtual
	~AlignSVG(){};

	typedef Alignment<const AlignBase::Axis, AlignBase::Axis::HORZ> HorzAlign;

	/// Alias for {HORZ:MIN}
	static
	const HorzAlign LEFT; // ,Coord::MIN

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


	enum Topol {
		INSIDE = 0,
		OUTSIDE = 1,
		UNDEFINED_TOPOL = 2,
	};


	/// Low-level, "atomic" setter of alignment for OBJECT itself or its ANCHOR object.
	/*
	 *  Notice that for an alignment to be complete, this function should be called twice:
	 *  setting partial alignment for both owner=OBJECT and owner=ANCHOR .
	 *
	 *  \tparam OBJ - enum type \c Owner  or string
	 *  \tparam A - enum type \c Axis or string
	 *  \tparam V - enum type \c Alignment or string
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
	void setAlign(const AlignBase::Axis & axis, const AlignBase::Pos & pos, Topol topol=Topol::INSIDE){
		modifyAlign(ANCHOR, axis, pos);
		modifyAlign(OBJECT, axis, (topol==INSIDE) ? pos : AlignBase::flip(pos));
		updateAlign();
	}

	/// NEW High-level, user friendlier interface for setting the alignments for both OBJECT itself and its ANCHOR object.
	/*
	 *  \tparam T - enum type \c Topol or string
	 *  \tparam A - enum type \c Alignment or string
	 *  \param topol  - \c INSIDE or \c OUTSIDE
	 *  \param align  - \c LEFT|CENTER|RIGHT or \c TOP|MIDDLE|BOTTOM
	template <typename T, typename AX, AlignBase::Axis A>
	void setAlign(const Alignment<AX,A> & align, const T & topol){
		const Topol & t = EnumDict<AlignSVG::Topol>::getValue(topol, false);
		// const Alignment<> & a = EnumDict<Alignment<> >::getValue(align, false);
		setAlign(align.axis, align.pos, t);
	}
	 */

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
		setAlign(align1.axis, align1.pos, AlignSVG::Topol::INSIDE);
		setAlign(align2.axis, align2.pos, AlignSVG::Topol::INSIDE);
	}

	/// Compiler trap: unimplemented for two of same kind: either \c HorzAlign or \c VertAlign twice.
	template <typename AX, AlignBase::Axis A>
	void setAlign(const Alignment<AX,A> & align1, const Alignment<AX,A> & align2);

	// Convenience: set both horz and vert alignments (INSIDE)
	/*
	void setAlign(const VertAlign & valign, const HorzAlign & halign){
		setAlign(halign.axis, halign.pos, AlignSVG::Topol::INSIDE);
		setAlign(valign.axis, valign.pos, AlignSVG::Topol::INSIDE);
	}
	 */


	// typedef Alignment<const AlignBase::Axis, AlignBase::Axis::HORZ> HorzAlign;
	/*
	template <AlignBase::Axis AX, typename ...T>
	void setAlign(const Alignment<const AlignBase::Axis, AX> & align, const T... args){
		if (align.pos == AlignBase::FILL){
			// Makes sense only for OBJECT, as it changed. ANCHOR is always unchanged.
			setAlign(AlignSVG::OBJECT, align.axis, align.pos);
		}
		else {
			setAlign(align.axis, align.pos, args...);
		}
	}
	*/

	/*
	 *  Template supports empty arg list.
	 *

	template <typename ...T>
	void setAlign(const HorzAlign & align, const T... args){
		if (align.pos == AlignBase::FILL){
			// Makes sense only for OBJECT, as it changed. ANCHOR is always unchanged.
			setAlign(AlignSVG::OBJECT, align.axis, align.pos);
		}
		else {
			setAlign(align.axis, align.pos, args...);
		}
	}

	template <typename ...T>
	void setAlign(const VertAlign & align, const T...  args){
		setAlign(align.axis, align.pos, args...);
	}
	 */




	/// High-level, user friendlier interface for setting the alignments for both OBJECT itself and its ANCHOR object.
	/*
	 *  \tparam T - enum type \c Topol or string: \c INSIDE or \c OUTSIDE .
	 *  \param align  - Horizontal or vertical Alignment: \c LEFT|CENTER|RIGHT or \c TOP|MIDDLE|BOTTOM
	 *  \param topol  - \c INSIDE or \c OUTSIDE
	 */
	template <typename T>
	void setAlign(const std::string & align, const T & topol){
		const Alignment<> & a = EnumDict<Alignment<> >::getValue(align, false);
		const Topol & t = EnumDict<AlignSVG::Topol>::getValue(topol, false);
		//const Alignment<> & a = EnumDict<Alignment<> >::getValue(align, false);
		setAlign(a.axis, a.pos, t);
	}

	/// High-level, user friendly interface for setting the alignments for both OBJECT itself and its ANCHOR object.

	/// Handler for command line or configuration file arguments
	void setAlign(const std::string & align);
	// Note: no mixed type, ANCHOR:LEFT

	/// Returns true, if any setting is set...
	bool isAligned() const;

	void resetAlign();

	/// Return alignment setting of an object along horizontal or vertical axis  .
	/*
	 *  \tparam P - enum type Owner \c REF or \c OBJ , or respective string.
	 *  \tparam A - enum type axis_t \c HORZ or \c VERT , or respective string.
	 *  \param pos - target object \c OBJ or referred anchor object \c REF
	 *  \param axis - horizontal \c HORZ or vertical \c AXIS .
	 */
	template <typename P, typename A>
	AlignBase::Pos & getAlign(const P & pos, const A & axis);


	/// Return alignment setting of an object along horizontal or vertical axis  .
	/*
	 *  \tparam P - enum type Owner \c REF or \c OBJ , or respective string.
	 *  \tparam A - enum type axis_t \c HORZ or \c VERT , or respective string.
	 *  \param pos - target object \c OBJ or referred anchor object \c REF
	 *  \param axis - horizontal \c HORZ or vertical \c AXIS .
	 *
	 */
	template <typename P, typename A>
	const AlignBase::Pos & getAlign(const P & pos, const A & axis) const;

	/**
	 *  For some reason, toStream() conflicts with: drain::NodeXML<T>::toStream(std::ostream&, const T&, const string&, int)
	 */
	void confToStream(std::ostream & ostr) const;

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

	/// Change alignment configuration without updating the alignStr.
	template <typename P, typename A, typename V>
	void modifyAlign(const P & owner, const A & axis,  const V &value){
		getAlign(owner, axis) = EnumDict<AlignBase::Pos>::getValue(value, false);
	}

	virtual inline
	void updateAlign(){};


	typedef std::vector<AlignBase::Pos> align_vect_t;
	typedef std::vector<align_vect_t > align_conf_t;

	align_conf_t alignments = align_conf_t(2, align_vect_t(2, AlignBase::Pos::UNDEFINED_POS));

	/// alignments[OBJECT][VERT] = alignments[OBJECT|ANCHOR][HORZ|VERT] = [MIN|MID|MAX]


};


inline
std::ostream & operator<<(std::ostream &ostr, const AlignSVG & align){
	//return ostr << align.axis << '_' << align.pos; // enums resolved above
	align.confToStream(ostr);
	return ostr; //  << "UNDER CONSTR...";  // RESOLVE!
}


template <>
const drain::EnumDict<AlignSVG::Owner>::dict_t drain::EnumDict<AlignSVG::Owner>::dict;
DRAIN_ENUM_OSTREAM(AlignSVG::Owner);

template <>
const drain::EnumDict<AlignSVG::Topol>::dict_t drain::EnumDict<AlignSVG::Topol>::dict;
DRAIN_ENUM_OSTREAM(AlignSVG::Topol);

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

DRAIN_TYPENAME(AlignSVG::HorzAlign);

DRAIN_TYPENAME(AlignSVG::VertAlign);

/// Dictionary of horizontal (LEFT,CENTER,RIGHT) flags
template <>
const drain::EnumDict<AlignSVG::HorzAlign>::dict_t  drain::EnumDict<AlignSVG::HorzAlign>::dict;
// DRAIN_ENUM_OSTREAM(AlignSVG::HorzAlign);

/// Dictionary of vertical (TOP,MIDDLE,BOTTOM) flags
template <>
const drain::EnumDict<AlignSVG::VertAlign>::dict_t  drain::EnumDict<AlignSVG::VertAlign>::dict;
// DRAIN_ENUM_OSTREAM(AlignSVG::VertAlign);


/// Dictionary combining horizontal (LEFT,CENTER,RIGHT) and vertical (TOP,MIDDLE,BOTTOM) flags
template <>
const drain::EnumDict<Alignment<> >::dict_t  drain::EnumDict<Alignment<> >::dict;




template <typename OBJ, typename A>
AlignBase::Pos & AlignSVG::getAlign(const OBJ & owner, const A & axis){
	const AlignSVG::Owner p = EnumDict<AlignSVG::Owner>::getValue(owner, false); // raise error
	const AlignBase::Axis a = EnumDict<AlignBase::Axis>::getValue(axis, false); // raise error
	return alignments[p][a];
}

template <typename OBJ, typename A>
const AlignBase::Pos & AlignSVG::getAlign(const OBJ & owner, const A & axis) const {
	const AlignSVG::Owner p  = EnumDict<AlignSVG::Owner>::getValue(owner, false); // raise error
	const AlignBase::Axis a  = EnumDict<AlignBase::Axis>::getValue(axis, false); // raise error
	return alignments[p][a];
}






/// "Alternative" \e partial alignment configuration for single object (OBJECT or its ANCHOR).
/**
 *  Essentially, a triplet (topol, axis, coord) .ie of types <Topol,Axis,Coord>
 *
 *  Designed for command line use.
 *
 */
template <typename AX = AlignBase::Axis, AlignBase::Axis A = AlignBase::Axis::UNDEFINED_AXIS> // , Align::Coord POS = Align::Coord::UNDEFINED_POS>
struct CompleteAlignment : public Alignment<AX,A> {

	AlignSVG::Topol topol = AlignSVG::Topol::INSIDE; // or undef?

	/// Constructor not setting Axis.
	template <class ...TT>
	CompleteAlignment(const TT... args) : Alignment<AX,A>() {
		set(args...);
	}
	//CompleteAlignment(AlignSVG::Topol topol=AlignSVG::Topol::INSIDE, AlignBase::Pos pos=AlignBase::Pos::MIN) : Alignment<AX,A>(pos), topol(topol){

	virtual inline
	bool isSet() const {
		return (this->axis != AlignBase::Axis::UNDEFINED_AXIS) &&
				(this->pos != AlignBase::Pos::UNDEFINED_POS) &&
				(topol != AlignSVG::Topol::UNDEFINED_TOPOL);
		// return Alignment<AX,A>::isSet() && (topol != AlignSVG::Topol::UNDEFINED_TOPOL);
	}


	virtual inline
	void reset(){
		//this->axis  = AlignBase::Axis::UNDEFINED_AXIS;
		Alignment<AX,A>::reset();
		// this->pos   = AlignBase::Pos::UNDEFINED_POS;
		topol = AlignSVG::Topol::UNDEFINED_TOPOL;
	}
	/// Constructor supporting setting of Axis.
	/*
	inline
	CompleteAlignment(AlignSVG::Topol topol, AlignBase::Axis axis, AlignBase::Pos pos=AlignBase::Pos::MIN) : Alignment<AX,A>(axis, pos), topol(topol){
	}
	*/

	// template <class ...TT>
	/*
	inline
	void set(const CompleteAlignment & align){ // , const TT... args){
		this->topol = align.topol;
		this->axis  = align.axis;
		this->pos   = align.pos;
		// Could continue: set(args...);
	}
	*/

	template <typename AX2, AlignBase::Axis A2, class ...TT>
	void set(const Alignment<AX2,A2> & align, const TT... args){
		this->axis = align.axis;
		this->pos  = align.pos;
		set(args...);
	}

	template <class ...TT>
	void set(AlignSVG::Topol topol, const TT... args){
		this->topol = topol;
		set(args...);
	}

	template <class ...TT>
	void set(AlignBase::Axis axis, const TT... args){
		this->axis = axis;
		set(args...);
	}

	template <class ...TT>
	void set(AlignBase::Pos coord, const TT... args){
		this->pos = coord;
		set(args...);
	}

	template <class ...TT>
	void set(const std::string & key, const TT... args){
		if (EnumDict<AlignSVG::Topol>::setValue(key, topol)){
			// ok
		}
		else if (EnumDict<Alignment<> >::setValue(key, *this)){ // RIGHT or?
			// ok
		}
		else if (EnumDict<AlignBase::Axis>::setValue(key, this->axis)){
			// ok
		}
		else if (EnumDict<AlignBase::Pos>::setValue(key, this->pos)){
			// ok
		}
		else {
			// Advice: keys
			throw std::runtime_error(drain::StringBuilder<>("key '", key, "' not found. Appeared in: ", args...));
		}

		set(args...);
	}

	template <class ...TT>
	void set(const char *key, const TT... args){
		set(std::string(key), args...);
	}


protected:

	inline
	void set(){
	}

};


template <typename AX, AlignBase::Axis A>
std::ostream & operator<<(std::ostream &ostr, const CompleteAlignment<AX,A> & ad){
	return ostr << ad.topol << '_' << ad.axis << ':' << ad.pos;
}


/// Higher level controller for setting alignments.
/**
 *  Also applied by PanelSVG
 */
class LayoutSVG {

public:


	//enum Axis {HORZ=0, VERT=1};  // must be indexed! for vect_ UNDEFINED_AXIS=0,
	typedef drain::EnumFlagger<drain::SingleFlagger<AlignBase::Axis> > AxisFlagger;
	AxisFlagger orientation = AlignBase::HORZ;

	enum Direction {
		UNDEFINED_DIRECTION=0,
		INCR = 1,
		DECR = 2,
	};
	typedef drain::EnumFlagger<drain::SingleFlagger<Direction> > DirectionFlagger;
	DirectionFlagger direction = INCR;

	// Experimental CSS classes
	enum GroupType {
		HEADER,
		ALIGN_FRAME,
		// ALI GNED,  // needed? "anchor" attrib and getAlign() should work
		ABSOLUTE, // "do not align (me or descendants) (future option)"
		FLOAT,       // = element does not affect alignment of other elems
	};


	inline
	LayoutSVG(AlignBase::Axis v=AlignBase::HORZ, Direction d=INCR) : orientation(v), direction(d) {
	}

	inline
	LayoutSVG(const LayoutSVG & layout) : orientation(layout.orientation), direction(layout.direction){
	}

	template <typename V>
	inline
	void setOrientation(const V &v){
		orientation.set(EnumDict<AlignBase::Axis>::getValue(v));
	};

	template <typename D>
	inline
	void setDirection(const D & d){
		direction.set(EnumDict<LayoutSVG::Direction>::getValue(d));
	};

	/// Set direction and orientation
	/**
	 *
	 */
	template <typename D, typename V>
	inline
	void set(const D & d, const V &v){
		direction.set(EnumDict<LayoutSVG::Direction>::getValue(d));
		orientation.set(EnumDict<AlignBase::Axis>::getValue(v));
	};


	/*
	static inline
	Direction flip(Direction ...){
		return ...
	};
	*/



};



template <>
const EnumDict<LayoutSVG::Direction>::dict_t  drain::EnumDict<LayoutSVG::Direction>::dict;
DRAIN_ENUM_OSTREAM(LayoutSVG::Direction);

template<>
const EnumDict<LayoutSVG::GroupType>::dict_t EnumDict<LayoutSVG::GroupType>::dict;
DRAIN_ENUM_OSTREAM(LayoutSVG::GroupType);




/// Adapter designed for NodeSVG
struct AlignAdapterSVG : public AlignSVG {

	/// Mark one of the elements of this object (SVG or G) as a decisive position
	inline
	void setAlignAnchor(const std::string & pathElem){
		anchorHorz = pathElem;
		anchorVert = pathElem;
		updateAlign();
	}

	inline
	void setAlignAnchorHorz(const std::string & pathElem){
		anchorHorz = pathElem;
		updateAlign();
	}

	inline
	void setAlignAnchorVert(const std::string & pathElem){
		anchorVert = pathElem;
		updateAlign();
	}

	///
	inline
	const std::string & getAlignAnchorHorz() const {
		return anchorHorz;
	}

	inline
	const std::string & getAlignAnchorVert() const {
		return anchorVert;
	}

	inline
	const std::string & getAlignStr() const {
		return alignStr;
	}

	inline virtual
	~AlignAdapterSVG(){};

protected:

	virtual inline
	void updateAlign() override {
		updateAlignStr();
	}

	std::string alignStr;

	std::string anchorHorz;
	std::string anchorVert;

	void updateAlignStr();

};






}  // image::

}  // drain::


DRAIN_ENUM_OSTREAM(drain::image::AlignBase::Axis);
DRAIN_ENUM_OSTREAM(drain::image::AlignBase::Pos);

DRAIN_ENUM_OSTREAM(drain::image::AlignSVG::Owner);

DRAIN_ENUM_OSTREAM(drain::image::LayoutSVG::Direction);


#endif // DRAIN_ALIGN_SVG_H_

