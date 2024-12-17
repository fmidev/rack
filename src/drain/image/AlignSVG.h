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
struct Align {

public:

	enum Coord {
		UNDEFINED_POS = 0,
		MIN = 1, // 0b01010101,
		MAX = 2, // 0b10101010,
		MID = 3, // 0b11111111,
		// ABSOLUTE?
	};

	enum Axis {
		// Index, must contain 0 and 1 for HORZ and VERT
		HORZ      = 0, // 0b00110011,
		VERT      = 1, // b11001100,
		UNDEFINED_AXIS = 2,
	};

	//const
	//Align::
	Axis  axis;

	//const
	// Align::
	Coord pos;

	inline
	Align(Axis axis = UNDEFINED_AXIS, Coord pos = Coord::UNDEFINED_POS) : axis(axis), pos(pos) {
	}

	inline
	Align(const Align & align) : axis(align.axis), pos(align.pos) {
	}

	inline
	bool operator==(const Align & align) const {
		return (align.axis == axis) && (align.pos == pos);
		// return compare(ad) == 0;
	}


	static inline
	Axis flip(Axis axis){
		//return (axis == HORZ) ? VERT : HORZ;
		switch (axis){
		case HORZ:
			return VERT;
		case VERT:
			return HORZ;
		default:
			return UNDEFINED_AXIS;
		}
	};

	//typedef drain::EnumDict<Coord>::dict_t pos_dict_t;

	static inline
	Coord flip(Coord v){
		switch (v){
		case MAX:
			return MIN;
		case MIN:
			return MAX;
		case MID:
			return MID;
		default:
			return UNDEFINED_POS;
		}
	};

};



template <>
const drain::EnumDict<Align::Axis>::dict_t drain::EnumDict<Align::Axis>::dict;
DRAIN_ENUM_OSTREAM(Align::Axis);

template <>
const drain::EnumDict<Align::Coord>::dict_t drain::EnumDict<Align::Coord>::dict;
DRAIN_ENUM_OSTREAM(Align::Coord);

inline
std::ostream & operator<<(std::ostream &ostr, const Align & align){
	//return ostr << align.axis << '_' << align.pos; // enums resolved above
	return ostr << (int)align.axis << '_' << (int)align.pos;  // RESOLVE!
}

/// NEW experimental, modular constructive





// NEW
/**
 *   \tparam AX - axis init value - still non-const value, so allows changing it...
 *
 *   Alignment<Align::Axis::HORZ>
 *   Alignment<Align::Axis::VERT>
 */
template <Align::Axis AX> // , Align::Coord POS = Align::Coord::UNDEFINED_POS>
struct Alignment :public Align {

	inline
	Alignment(Align::Coord pos = Align::Coord::UNDEFINED_POS) : Align(AX, pos){
	}

};

typedef const Alignment<Align::Axis::HORZ> HorzAlign; // unused?
DRAIN_TYPENAME(HorzAlign);
typedef const Alignment<Align::Axis::VERT> VertAlign; // unused?
DRAIN_TYPENAME(VertAlign);

template <>
const drain::EnumDict<Alignment<Align::Axis::HORZ> >::dict_t  drain::EnumDict<Alignment<Align::Axis::HORZ> >::dict;

template <>
const drain::EnumDict<Alignment<Align::Axis::VERT> >::dict_t  drain::EnumDict<Alignment<Align::Axis::VERT> >::dict;

template <>
const drain::EnumDict<Align>::dict_t  drain::EnumDict<Align>::dict;


/// User-friendly programming interface for alignment considering two elements.
/**
 *   Considers two elements, or more specifically, the bounding boxes of two elements.
 *
 */
struct AlignSVG : protected Align {

	typedef int bitvect_t;  // or int_t ?

	virtual
	~AlignSVG(){};

	static
	const Alignment<Axis::HORZ> LEFT; // ,Coord::MIN

	static
	const Alignment<Axis::HORZ> CENTER;

	static
	const Alignment<Axis::HORZ> RIGHT;

	static
	const Alignment<Align::Axis::HORZ> UNDEFINED_HORZ;

	// ----------------------

	static
	const Alignment<Align::Axis::VERT> TOP;

	static
	const Alignment<Align::Axis::VERT> MIDDLE;

	static
	const Alignment<Align::Axis::VERT> BOTTOM;

	static
	const Alignment<Align::Axis::VERT> UNDEFINED_VERT;


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

	/// Lowish-level, atomic setter of alignment.
	//  Used by file-svg.cpp
	inline
	void setAlign(Topol topol, const Align::Axis & direction, const Align::Coord & position){
		modifyAlign(ANCHOR, direction, position);
		modifyAlign(OBJECT, direction, (topol==INSIDE) ? position : Align::flip(position));
	}

	inline
	void setAlign(Topol topol, const Align & pos){
		setAlign(topol, pos.axis, pos.pos);
		// setAlign(OBJECT, pos.axis, (topol==INSIDE) ? position : Align::flip(position));
	}

	/// Set a single alignment setting. High-level.
	/*
	 *  \tparam T - enum type \c Topol or string
	 *  \tparam A - enum type \c Align or string Alignment
	 *  \param topol   - enum value \c INSIDE or \c OUTSIDE
	 *  \param align  - enum value \c LEFT|CENTER|RIGHT or \c TOP|MIDDLE|BOTTOM
	 */
	template <typename T, typename A>
	void setAlign(const T & topol, const A & align){
		const Topol & t = EnumDict<AlignSVG::Topol>::getValue(topol, false);
		const Align & a = EnumDict<Align>::getValue(align, false);
		setAlign(t, a.axis, a.pos);
	}
	// Note: no mixed type, ANCHOR:LEFT


	/// Set a single alignment setting. Low-level, ie. axis and value separately.
	/*
	 *  \tparam P - enum type \c Coord  or string
	 *  \tparam A - enum type \c Axis or string
	 *  \tparam V - enum type \c Alignment or string
	 *  \param pos   - enum value \c OBJ or \c REF
	 *  \param axis  - enum value \c HORZ or \c VERT
	 *  \param value - enum value \c MAX , \c MID , or \c MIN (or string)
	 */
	template <typename P, typename A, typename V>
	inline
	void setAlign(const P & owner, const A & axis,  const V &value){
		modifyAlign(owner, axis, value);
		updateAlign();
	}



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
	Coord & getAlign(const P & pos, const A & axis);


	/// Return alignment setting of an object along horizontal or vertical axis  .
	/*
	 *  \tparam P - enum type Owner \c REF or \c OBJ , or respective string.
	 *  \tparam A - enum type axis_t \c HORZ or \c VERT , or respective string.
	 *  \param pos - target object \c OBJ or referred anchor object \c REF
	 *  \param axis - horizontal \c HORZ or vertical \c AXIS .
	 *
	 */
	template <typename P, typename A>
	const Coord & getAlign(const P & pos, const A & axis) const;

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

	/// Change alignment configuration without updating the alignStr.
	template <typename P, typename A, typename V>
	void modifyAlign(const P & owner, const A & axis,  const V &value){
		getAlign(owner, axis) = EnumDict<Coord>::getValue(value, false);
	}

	mutable
	bitvect_t alignment = 0;

	virtual inline
	void updateAlign(){};

	typedef std::vector<Align::Coord> align_vect_t;
	typedef std::vector<align_vect_t > align_conf_t;

	/// alignments[OBJECT][VERT] = alignments[OBJECT|ANCHOR][HORZ|VERT] = [MIN|MID|MAX]
	align_conf_t alignments = align_conf_t(2, align_vect_t(2, Align::Coord::UNDEFINED_POS));


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
Align::Coord & AlignSVG::getAlign(const P & owner, const A & axis){
	const AlignSVG::Owner p = EnumDict<AlignSVG::Owner>::getValue(owner, false); // raise error
	const Align::Axis a     = EnumDict<Align::Axis>::getValue(axis, false); // raise error
	return alignments[p][a];
}

template <typename P, typename A>
const Align::Coord & AlignSVG::getAlign(const P & owner, const A & axis) const {
	const AlignSVG::Owner p  = EnumDict<AlignSVG::Owner>::getValue(owner, false); // raise error
	const Align::Axis a      = EnumDict<Align::Axis>::getValue(axis, false); // raise error
	return alignments[p][a];
}





// Rename AlignConf ?
/// For command line use.
struct Alignment2 :public Align {

	AlignSVG::Topol topol;

	inline
	Alignment2(AlignSVG::Topol topol=AlignSVG::Topol::INSIDE, Align::Axis axis=Align::Axis::HORZ, Align::Coord pos=Align::Coord::MIN) : Align(axis, pos), topol(topol){
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
	void set(const Align & apos, const TT... args){
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
	void set(Align::Axis axis, const TT... args){
		this->axis = axis;
		set(args...);
	}

	template <class ...TT>
	void set(Align::Coord coord, const TT... args){
		this->pos = coord;
		set(args...);
	}

	template <class ...TT>
	void set(const std::string & key, const TT... args){
		if (EnumDict<AlignSVG::Topol>::setValue(key, topol)){
			// ok
		}
		else if (EnumDict<Align>::setValue(key, *this)){
			// ok
		}
		else if (EnumDict<Align::Axis>::setValue(key, axis)){
			// ok
		}
		else if (EnumDict<Align::Coord>::setValue(key, pos)){
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
	typedef drain::EnumFlagger<drain::SingleFlagger<Align::Axis> > AxisFlagger;
	AxisFlagger orientation = Align::HORZ;

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
	LayoutSVG(Align::Axis v=Align::HORZ, Direction d=INCR) : orientation(v), direction(d) {
	}

	inline
	LayoutSVG(const LayoutSVG & layout) : orientation(layout.orientation), direction(layout.direction){
	}

	template <typename V>
	inline
	void setOrientation(const V &v){
		orientation.set(EnumDict<Align::Axis>::getValue(v));
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
		orientation.set(EnumDict<Align::Axis>::getValue(v));
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


DRAIN_ENUM_OSTREAM(drain::image::Align::Axis);
DRAIN_ENUM_OSTREAM(drain::image::Align::Coord);

DRAIN_ENUM_OSTREAM(drain::image::AlignSVG::Owner);

DRAIN_ENUM_OSTREAM(drain::image::LayoutSVG::Direction);


#endif // DRAIN_ALIGN_SVG_H_

