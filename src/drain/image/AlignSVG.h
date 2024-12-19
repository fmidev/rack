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


template <typename AX = AlignBase::Axis, AlignBase::Axis V = AlignBase::Axis::UNDEFINED_AXIS> // , Align::Coord POS = Align::Coord::UNDEFINED_POS>
struct AlignCoord {

	 /*struct AlignCoord : public AlignBase {
		Axis  axis;
		Pos pos;
		*/

	AX axis = V; // compiler error if different type.
	AlignBase::Pos pos = AlignBase::Pos::UNDEFINED_POS;

	inline
	AlignCoord(AlignBase::Pos pos = AlignBase::Pos::UNDEFINED_POS) : pos(pos){
	}

	inline
	AlignCoord(AlignBase::Axis axis, AlignBase::Pos pos = AlignBase::Pos::UNDEFINED_POS) : pos(pos){ // axis(axis),
		this->axis = axis;
	}

	inline
	AlignCoord(const AlignCoord & ac) : axis(ac.axis), pos(ac.pos){
	}

	template <typename AX2, AlignBase::Axis A2>
	inline
	AlignCoord(const AlignCoord<AX2,A2> & ac) : pos(ac.pos){ // axis(ac.axis),
		axis = ac.axis; // error if const
	}

	/*
	inline
	AlignCoord(Axis axis = UNDEFINED_AXIS, Pos pos = Pos::UNDEFINED_POS) : axis(axis), pos(pos) {
	}

	inline
	AlignCoord(const AlignCoord & align) : axis(align.axis), pos(align.pos) {
	}
	*/

	inline
	bool operator==(const AlignCoord & align) const {
		return (align.axis == axis) && (align.pos == pos);
		// return compare(ad) == 0;
	}

protected:




};



template <>
const drain::EnumDict<AlignBase::Axis>::dict_t drain::EnumDict<AlignBase::Axis>::dict;
DRAIN_ENUM_OSTREAM(AlignBase::Axis);

template <>
const drain::EnumDict<AlignBase::Pos>::dict_t drain::EnumDict<AlignBase::Pos>::dict;
DRAIN_ENUM_OSTREAM(AlignBase::Pos);



template <typename AX, AlignBase::Axis V>
inline
std::ostream & operator<<(std::ostream &ostr, const AlignCoord<AX,V> & align){
	//return ostr << align.axis << '_' << align.pos; // enums resolved above
	return ostr << (int)align.axis << '_' << (int)align.pos;  // RESOLVE!
}

/// NEW experimental, modular constructive






//typedef const Alignment<AlignCoord::Axis::HORZ> HorzAlign; // unused?
// typedef Alignment<AlignCoord::Axis::HORZ> HorzAlign; // unused?
typedef AlignCoord<const AlignBase::Axis, AlignBase::Axis::HORZ> HorzAlign;
DRAIN_TYPENAME(HorzAlign);

// typedef const Alignment<AlignCoord::Axis::VERT> VertAlign; // unused?
//typedef Alignment<AlignCoord::Axis::VERT> VertAlign; // unused?
typedef AlignCoord<const AlignBase::Axis, AlignBase::Axis::VERT> VertAlign;
DRAIN_TYPENAME(VertAlign);


template <>
const drain::EnumDict<HorzAlign>::dict_t  drain::EnumDict<HorzAlign>::dict;

template <>
const drain::EnumDict<VertAlign>::dict_t  drain::EnumDict<VertAlign>::dict;


// NEW
/**
 *   \tparam AX - axis init value - still non-const value, so allows changing it...
 *
 *   Alignment<Align::Axis::HORZ>
 *   Alignment<Align::Axis::VERT>
 */
template <typename AX = AlignBase::Axis> // instead of const Axis        , Align::Coord POS = Align::Coord::UNDEFINED_POS>
struct Alignment : public AlignCoord<AX> {

	inline
	Alignment(AlignBase::Pos pos = AlignBase::Pos::UNDEFINED_POS) : AlignCoord<AX>(pos){
	}

	inline
	Alignment(const Alignment & align) : AlignCoord<AX>(align.pos){
	}

protected:

	inline
	Alignment(AlignBase::Axis axis, AlignBase::Pos pos = AlignBase::Pos::UNDEFINED_POS) : AlignCoord<AX>(pos){
	}


};


// struct AlignSVG;

template <>
const drain::EnumDict<AlignCoord<> >::dict_t  drain::EnumDict<AlignCoord<> >::dict;


struct Alignment2;

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


	/// Alias for {HORZ:MIN}
	static
	const HorzAlign LEFT; // ,Coord::MIN

	/// Alias for {HORZ:MID}
	static
	const HorzAlign CENTER;

	/// Alias for {HORZ:MAX}
	static
	const HorzAlign RIGHT;

	static
	const HorzAlign UNDEFINED_HORZ;

	// ----------------------

	/// Alias for {VERT:MIN}
	static
	const VertAlign TOP;

	/// Alias for {VERT:MID}
	static
	const VertAlign MIDDLE;

	/// Alias for {VERT:MAX}
	static
	const VertAlign BOTTOM;

	static
	const VertAlign UNDEFINED_VERT;


	enum Owner {
		OBJECT = 0, // 0b00001111,
		ANCHOR = 1, // 0b11110000,
	};
	// typedef drain::EnumDict<Owner>::dict_t vert_dict_t;


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
	inline
	void setAlign(const OBJ & owner, const A & axis,  const V &value){
		modifyAlign(owner, axis, value);
		updateAlign();
	}

	/// High-level, user friendlier interface for setting the alignments for both OBJECT itself and its ANCHOR object.
	/*
	 *  \tparam T - enum type \c Topol or string
	 *  \tparam A - enum type \c AlignCoord or string
	 *  \param topol  - \c INSIDE or \c OUTSIDE
	 *  \param align  - \c LEFT|CENTER|RIGHT or \c TOP|MIDDLE|BOTTOM
	 */
	template <typename T, typename AX, AlignBase::Axis A>
	void setAlign(const T & topol, const AlignCoord<AX,A> & align){
		const Topol & t = EnumDict<AlignSVG::Topol>::getValue(topol, false);
		// const Alignment<> & a = EnumDict<Alignment<> >::getValue(align, false);
		setAlign(t, align.axis, align.pos);
	}

	template <typename T>
	void setAlign(const T & topol, const std::string & align){
		const Topol & t = EnumDict<AlignSVG::Topol>::getValue(topol, false);
		//const Alignment<> & a = EnumDict<Alignment<> >::getValue(align, false);
		const AlignCoord<> & a = EnumDict<AlignCoord<> >::getValue(align, false);
		setAlign(t, a.axis, a.pos);
	}

	/// High-level, user friendly interface for setting the alignments for both OBJECT itself and its ANCHOR object.
	/*
	 *  \param pos   - enum value \c OBJ or \c REF
	 *  \param axis  - enum value \c HORZ or \c VERT
	inline
	void setAlign(Topol topol, const AlignCoord & pos){
		setAlign(topol, pos.axis, pos.pos);
	}
	 */

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
	void setAlign(Topol topol, const AlignBase::Axis & axis, const AlignBase::Pos & pos){
		modifyAlign(ANCHOR, axis, pos);
		modifyAlign(OBJECT, axis, (topol==INSIDE) ? pos : AlignBase::flip(pos));
		updateAlign();
	}

	/// Handler for command line or configuration file arguments
	void setAlign(const std::string & align);


	// Note: no mixed type, ANCHOR:LEFT





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

	typedef int bitvect_t;  // or int_t ?

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

	/// alignments[OBJECT][VERT] = alignments[OBJECT|ANCHOR][HORZ|VERT] = [MIN|MID|MAX]
	align_conf_t alignments = align_conf_t(2, align_vect_t(2, AlignBase::Pos::UNDEFINED_POS));


};

template <>
const drain::EnumDict<AlignSVG::Owner>::dict_t drain::EnumDict<AlignSVG::Owner>::dict;
DRAIN_ENUM_OSTREAM(AlignSVG::Owner);

template <>
const drain::EnumDict<AlignSVG::Topol>::dict_t drain::EnumDict<AlignSVG::Topol>::dict;
DRAIN_ENUM_OSTREAM(AlignSVG::Topol);

inline
std::ostream & operator<<(std::ostream &ostr, const AlignSVG & align){
	//return ostr << align.axis << '_' << align.pos; // enums resolved above
	align.confToStream(ostr);
	return ostr; //  << "UNDER CONSTR...";  // RESOLVE!
}


template <typename P, typename A>
AlignBase::Pos & AlignSVG::getAlign(const P & owner, const A & axis){
	const AlignSVG::Owner p = EnumDict<AlignSVG::Owner>::getValue(owner, false); // raise error
	const AlignBase::Axis a = EnumDict<AlignBase::Axis>::getValue(axis, false); // raise error
	return alignments[p][a];
}

template <typename P, typename A>
const AlignBase::Pos & AlignSVG::getAlign(const P & owner, const A & axis) const {
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
struct Alignment2 : public Alignment<> {

	AlignSVG::Topol topol;

	inline
	Alignment2(AlignSVG::Topol topol=AlignSVG::Topol::INSIDE, AlignBase::Axis axis=AlignBase::Axis::HORZ, AlignBase::Pos pos=AlignBase::Pos::MIN) : Alignment<>(axis, pos), topol(topol){
	}

	// template <class ...TT>
	inline
	void set(const Alignment2 & align){ // , const TT... args){
		this->topol = align.topol;
		this->axis  = align.axis;
		this->pos   = align.pos;
		// Could continue: set(args...);
	}

	template <class ...TT>
	void set(const AlignCoord & apos, const TT... args){
		this->axis  = apos.axis;
		this->pos   = apos.pos;
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
		/*
		else if (EnumDict<Alignment<> >::setValue(key, *this)){ // RIGHT or?
			// ok
		}
		*/
		else if (EnumDict<AlignBase::Axis>::setValue(key, axis)){
			// ok
		}
		else if (EnumDict<AlignBase::Pos>::setValue(key, pos)){
			// ok
		}
		else {
			// Advice: keys
			throw std::runtime_error(drain::StringBuilder<>("key '", key, "' not found"));
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


inline
std::ostream & operator<<(std::ostream &ostr, const Alignment2 & ad){
	return ostr << ad.topol << '_' << ad.axis << ':' << ad.pos;  // RESOLVE!
	//return ostr << (int)ad.axis << '_' << (int)ad.pos;  // RESOLVE!
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

