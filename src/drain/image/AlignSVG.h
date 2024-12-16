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


	static inline
	Axis flip(Axis axis){
		return (axis == HORZ) ? VERT : HORZ;
		/*
		switch (axis){
		case HORZ:
			return VERT;
		case VERT:
			return HORZ;
		default:
			return UNDEFINED_AXIS;
		}
		*/
		// return orientation==HORZ ? VERT : HORZ;
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


/*
struct AlignConf {
protected:
};
*/

/// NEW experimental, modular constructive

struct AlignPos {

	//const
	Align::Axis  axis;

	//const
	Align::Coord pos;

	inline
	AlignPos(Align::Axis axis = Align::Axis::UNDEFINED_AXIS, Align::Coord pos = Align::Coord::UNDEFINED_POS) : axis(axis), pos(pos) {
	}

	inline
	AlignPos(const AlignPos & aligner) : axis(aligner.axis), pos(aligner.pos) {
	}


	inline
	bool operator==(const AlignPos & ad) const {
		return (ad.axis == axis) && (ad.pos == pos);
		// return compare(ad) == 0;
	}

	// In future, there might be need for testing alignments with less/greater-than ops (ANCHOR:LEFT + OBJECT:RIGHT < ANCHOR:LEFT + OBJECT:LEFT)
	/*
	inline
	int compare(const AlignPos & ad) const {
		if (ad.axis == axis){
			if (ad.pos == pos){
				return 0;
			}
			else {
				return 1; // TODO: MIN < MAX ->  -1
			}
		}
		else {
			return 1;
		}
	}
	*/

};

inline
std::ostream & operator<<(std::ostream &ostr, const AlignPos & ad){
	return ostr << ad.axis << '_' << ad.pos;  // RESOLVE!
	//return ostr << (int)ad.axis << '_' << (int)ad.pos;  // RESOLVE!
}

/*
template <>
const drain::EnumDict<AlignPos>::dict_t drain::EnumDict<AlignPos>::dict;
*/


// NEW
/**
 *   Alignment<Align::Axis::HORZ>
 */
template <Align::Axis AX>
struct Alignment :public AlignPos {

	inline
	Alignment(Align::Coord pos = Align::Coord::UNDEFINED_POS) : AlignPos(AX, pos){
	}

};

typedef Alignment<Align::Axis::HORZ> HorzAlign2;
DRAIN_TYPENAME(HorzAlign2);
typedef Alignment<Align::Axis::VERT> VertAlign2;
DRAIN_TYPENAME(VertAlign2);

template <>
const drain::EnumDict<Alignment<Align::Axis::HORZ> >::dict_t  drain::EnumDict<Alignment<Align::Axis::HORZ> >::dict;

template <>
const drain::EnumDict<Alignment<Align::Axis::VERT> >::dict_t  drain::EnumDict<Alignment<Align::Axis::VERT> >::dict;

// NEW
/*
template <Align::Axis AX, Align::Coord POS>
struct ConstAlign :public Alignment<AX> {

	inline
	ConstAlign() : Alignment<AX>(POS){
	}

};
*/




struct AlignSVG : protected Align {

	typedef int bitvect_t;  // or int_t ?

	virtual
	~AlignSVG(){};

	static
	const Alignment<Align::Axis::HORZ> RIGHT;

	static
	const Alignment<Align::Axis::HORZ> CENTER;

	static
	const Alignment<Align::Axis::HORZ> LEFT;

	static
	const Alignment<Align::Axis::HORZ> UNDEFINED_HORZ;

	static
	const Alignment<Align::Axis::VERT> BOTTOM;

	static
	const Alignment<Align::Axis::VERT> MIDDLE;

	static
	const Alignment<Align::Axis::VERT> TOP;

	static
	const Alignment<Align::Axis::VERT> UNDEFINED_VERT;

	/*
	enum HorzAlign {
		UNDEFINED_HORZ=0,
		LEFT   = 1, // Axis::HORZ & Coord::MIN,
		RIGHT  = 2, // Axis::HORZ & Coord::MAX,
		CENTER = 3, // Axis::HORZ & Coord::MID,
		// ABSOLUTE?
	};
	//typedef drain::EnumDict<HorzAlign>::dict_t horz_dict_t;

	enum VertAlign {
		UNDEFINED_VERT=0,
		TOP    = 1, // Axis::VERT & Coord::MIN,
		BOTTOM = 2, // Axis::VERT & Coord::MAX,
		MIDDLE = 3, // Axis::VERT & Coord::MID,
		// ABSOLUTE?
	};
	typedef drain::EnumDict<VertAlign>::dict_t vert_dict_t;
	*/

	enum Owner {
		OBJECT = 0, // 0b00001111,
		ANCHOR = 1, // 0b11110000,
	};
	// typedef drain::EnumDict<Owner>::dict_t vert_dict_t;

	bitvect_t alignment = 0;

	enum Topol {
		INSIDE = 0,
		OUTSIDE = 1,
		UNDEFINED_TOPOL = 2,
	};


	/*
	inline
	void setAlign(Topol topol, const HorzAlign & ha){

		switch (EnumDict<AlignSVG::HorzAlign>::getValue(ha)){
			case HorzAlign::LEFT:
				setAlign(topol, HORZ, MIN);
				// setAlign(ANCHOR, HORZ, MIN);
				// setAlign(OBJECT, HORZ, (topol==Topol::INSIDE) ? MIN : MAX); // Inside(Align::HORZ, Align::MIN);
				break;
			case HorzAlign::CENTER:
				setAlign(topol, HORZ, MID);
				// setAlign(ANCHOR, HORZ, MID);
				// setAlign(OBJECT, HORZ, MID); // Inside(Align::HORZ, Align::MIN);
				break;
			case HorzAlign::RIGHT:
				setAlign(ANCHOR, HORZ, MAX);
				// setAlign(ANCHOR, HORZ, MAX);
				// setAlign(OBJECT, HORZ, (topol==Topol::INSIDE) ? MAX : MIN); // Inside(Align::HORZ, Align::MIN);
				break;
			default:
				break;
		}
	}
	*/

	/// Low-level, atomic setter of alignment.
	//  Used by file-svg.cpp
	inline
	void setAlign(Topol topol, const Align::Axis & direction, const Align::Coord & position){
		setAlign(ANCHOR, direction, position);
		setAlign(OBJECT, direction, (topol==INSIDE) ? position : Align::flip(position));
	}

	inline
	void setAlign(Topol topol, const AlignPos & pos){
		setAlign(topol, pos.axis, pos.pos);
		// setAlign(OBJECT, pos.axis, (topol==INSIDE) ? position : Align::flip(position));
	}


	/*
	inline
	void setAlign(Topol topol, const VertAlign & va){

		switch (EnumDict<VertAlign>::getValue(va)){
			case VertAlign::TOP:
				setAlign(topol, VERT, MIN);
				// setAlign(ANCHOR, VERT, MIN);
				// setAlign(OBJECT, VERT, (topol==Topol::INSIDE) ? MIN : MAX); // Inside(Align::HORZ, Align::MIN);
				break;
			case VertAlign::MIDDLE:
				setAlign(topol, VERT, MID);
				// setAlign(ANCHOR, VERT, MID);
				// setAlign(OBJECT, VERT, MID); // Inside(Align::HORZ, Align::MIN);
				break;
			case VertAlign::BOTTOM:
				setAlign(topol, VERT, MAX);
				// setAlign(ANCHOR, VERT, MAX);
				// setAlign(OBJECT, VERT, (topol==Topol::INSIDE) ? MAX : MIN); // Inside(Align::HORZ, Align::MIN);
				break;
			default:
				break;
		}
	}
	*/

	/// Set a single alignment setting.
	/*
	 *  \tparam P - enum type \c Coord  or string
	 *  \tparam A - enum type \c Axis or string
	 *  \tparam V - enum type \c Alignment or string
	 *  \param pos   - enum value \c OBJ or \c REF
	 *  \param axis  - enum value \c HORZ or \c VERT
	 *  \param value - enum value \c MAX , \c MID , or \c MIN (or string)
	 */
	template <typename P, typename A, typename V>
	void setAlign(const P & owner, const A & axis,  const V &value){
		getAlign(owner, axis) = EnumDict<Coord>::getValue(value);
		updateAlign();
	}


	/*
	inline
	void resetAlign(){
		alignment = 0;
		updateAlign();
	};
	*/
	void resetAlign();


	/*
	template <typename ...TT>
	void setAlign(Owner arg, const TT... args){
		// First reset all settings for this owner
		// alignment &= ~arg;
		alignment |= (arg & combineAlign(args...));
		updateAlign();
	};

	/// Adds bits
	template <typename ...TT>
	void setAlign(HorzAlign arg, const TT... args){
		// First reset all HORZ settings
		// alignment &= ~arg;
		alignment |= (arg & combineAlign(args...));
		updateAlign();
	};

	/// Adds bits
	template <typename ...TT>
	void setAlign(VertAlign arg, const TT... args){
		// First reset all VERT settings
		// alignment &= ~arg;
		alignment |= (arg & combineAlign(args...));
		updateAlign();
	};

	inline
	bool isAligned() const {
		// return (alignment != 0);
	}
	*/

	bool isAligned() const;



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


	/*
	inline
	const Coord & getAlign(Owner owner, Axis axis) const {
		return alignments[owner][axis];
	}

	inline
	Coord & getAlign(Owner owner, Axis axis) {
		return alignments[owner][axis];
	}
	*/

	virtual inline
	void updateAlign(){};

	/*
	/// Checks if at least some combination of (ANCHOR|OBJECT) × (HORZ|VERT) × (LEFT|OBJECT),
	template <typename ...TT>
	bool isAlignSet(const TT... args) const {
		bitvect_t v = combineAlign(args...);
		return (alignment & v) == v;
	}

	*/


protected:

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


template <typename P, typename A>
Align::Coord & AlignSVG::getAlign(const P & owner, const A & axis){
	const AlignSVG::Owner p = EnumDict<AlignSVG::Owner>::getValue(owner);
	const Align::Axis a     = EnumDict<Align::Axis>::getValue(axis);
	return alignments[p][a];
}

template <typename P, typename A>
const Align::Coord & AlignSVG::getAlign(const P & owner, const A & axis) const {
	const AlignSVG::Owner p  = EnumDict<AlignSVG::Owner>::getValue(owner);
	const Align::Axis a      = EnumDict<Align::Axis>::getValue(axis);
	return alignments[p][a];
}

//template <>
// const drain::EnumDict<AlignSVG::Direction>::dict_t  drain::EnumDict<AlignSVG::Direction>::dict;







// For two element.
/**
 *   Considers two elements, or more specifcally, the bounding boxes of two elements.
 *
 */
struct Alignment2 :public AlignPos {

	AlignSVG::Topol topol;

	inline
	Alignment2(AlignSVG::Topol topol=AlignSVG::Topol::INSIDE, Align::Axis axis=Align::Axis::HORZ, Align::Coord pos=Align::Coord::MIN) : AlignPos(axis, pos), topol(topol){
	}

	// Consider HorzAlign2

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

	/*
	template <class T, class TT>
	void set(const T &arg, const TT... args){
		EnumDict<T>::getValue(owner);
		set(args...);
	}
	*/

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

