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

	enum Position {
		UNDEFINED_POS = 0,
		MIN = 1, // 0b01010101,
		MAX = 2, // 0b10101010,
		MID = 3, //0b11111111,
		// ABSOLUTE?
	};

	enum Axis {
		// UNDEFINED_AXIS = 0,
		HORZ      = 0, // 0b00110011,
		VERT      = 1, // b11001100,
	};
	//typedef drain::EnumDict<Axis>::dict_t axis_edict_t;


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

	//typedef drain::EnumDict<Position>::dict_t pos_dict_t;

	static inline
	Position flip(Position v){
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
const drain::EnumDict<Align::Position>::dict_t drain::EnumDict<Align::Position>::dict;
DRAIN_ENUM_OSTREAM(Align::Position);


struct AlignConf {


protected:


};


struct AlignSVG : protected Align {

	typedef int bitvect_t;  // or int_t ?

	virtual
	~AlignSVG(){};


	enum HorzAlign {
		UNDEFINED_HORZ=0,
		LEFT   = 1, // Axis::HORZ & Position::MIN,
		RIGHT  = 2, // Axis::HORZ & Position::MAX,
		CENTER = 3, // Axis::HORZ & Position::MID,
		// ABSOLUTE?
	};
	//typedef drain::EnumDict<HorzAlign>::dict_t horz_dict_t;

	enum VertAlign {
		UNDEFINED_VERT=0,
		TOP    = 1, // Axis::VERT & Position::MIN,
		BOTTOM = 2, // Axis::VERT & Position::MAX,
		MIDDLE = 3, // Axis::VERT & Position::MID,
		// ABSOLUTE?
	};
	typedef drain::EnumDict<VertAlign>::dict_t vert_dict_t;

	enum Owner {
		OBJECT = 0, // 0b00001111,
		ANCHOR = 1, // 0b11110000,
	};
	// typedef drain::EnumDict<Owner>::dict_t vert_dict_t;

	bitvect_t alignment = 0;

	enum Topol {
		INSIDE = 0,
		OUTSIDE = 1,
	};

	/// High-level alignment function.
	template <Topol T>
	inline
	void setAlign(const AlignSVG::HorzAlign & ha, const AlignSVG::VertAlign & va){
		setAlign<T>(ha);
		setAlign<T>(va);
	}

	/// High-level alignment function.
	/**
	 *   Locks object and anchor.
	 */
	template <Topol T>
	inline
	void setAlign(const AlignSVG::HorzAlign & ha){

		// const HorzAlign horzAlign = EnumDict<HorzAlign>::getValue(ha);
		// AlignSVG::Position & horzRef = getAlign(Owner::REF, LayoutSVG::HORZ);
		// AlignSVG::Position & horzObj = getAlign(Owner::OBJ, LayoutSVG::HORZ);

		switch (EnumDict<AlignSVG::HorzAlign>::getValue(ha)){
			case HorzAlign::LEFT:
				setAlign(ANCHOR, HORZ, MIN);
				setAlign(OBJECT, HORZ, (T==Topol::INSIDE) ? MIN : MAX); // Inside(Align::HORZ, Align::MIN);
				break;
			case HorzAlign::CENTER:
				setAlign(ANCHOR, HORZ, MID);
				setAlign(OBJECT, HORZ, MID); // Inside(Align::HORZ, Align::MIN);
				// setAlignInside(Align::HORZ, Align::MID);
				break;
			case HorzAlign::RIGHT:
				break;
				setAlign(ANCHOR, HORZ, MAX);
				setAlign(OBJECT, HORZ, (T==Topol::INSIDE) ? MAX : MIN); // Inside(Align::HORZ, Align::MIN);
				// setAlignInside(Align::HORZ, Align::MAX);
				break;
			default:
				break;
		}
	}

	/// High-level alignment function.
	/**
	 *   Locks object and anchor.
	 */
	template <Topol T>
	inline
	void setAlign(const VertAlign & va){

		switch (EnumDict<VertAlign>::getValue(va)){
			case VertAlign::TOP:
				setAlign(ANCHOR, VERT, MIN);
				setAlign(OBJECT, VERT, (T==Topol::INSIDE) ? MIN : MAX); // Inside(Align::HORZ, Align::MIN);
				// setAlignInside(Align::VERT, Align::MIN);
				break;
			case VertAlign::MIDDLE:
				setAlign(ANCHOR, VERT, MID);
				setAlign(OBJECT, VERT, MID); // Inside(Align::HORZ, Align::MIN);
				// setAlignInside(Align::VERT, Align::MID);
				break;
			case VertAlign::BOTTOM:
				setAlign(ANCHOR, VERT, MAX);
				setAlign(OBJECT, VERT, (T==Topol::INSIDE) ? MAX : MIN); // Inside(Align::HORZ, Align::MIN);
				// setAlignInside(Align::VERT, Align::MAX);
				break;
			default:
				break;
		}
		// getAlign(Owner::REF, LayoutSVG::VERT) = Align::MIN;
		// getAlign(Owner::OBJ, LayoutSVG::VERT) = Align::MAX;

		// updateAlign();
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

	/// Set a single alignment setting.
	/*
	 *  \tparam P - enum type \c Position  or string
	 *  \tparam A - enum type \c Axis or string
	 *  \tparam V - enum type \c Alignment or string
	 *  \param pos   - enum value \c OBJ or \c REF
	 *  \param axis  - enum value \c HORZ or \c VERT
	 *  \param value - enum value \c MAX , \c MID , or \c MIN (or string)
	 */
	template <typename P, typename A, typename V>
	void setAlign(const P & owner, const A & axis,  const V &value){
		getAlign(owner, axis) = EnumDict<Position>::getValue(value);
		updateAlign();
	}


	/// Return alignment setting of an object along horizontal or vertical axis  .
	/*
	 *  \tparam P - enum type Owner \c REF or \c OBJ , or respective string.
	 *  \tparam A - enum type axis_t \c HORZ or \c VERT , or respective string.
	 *  \param pos - target object \c OBJ or referred anchor object \c REF
	 *  \param axis - horizontal \c HORZ or vertical \c AXIS .
	 */
	template <typename P, typename A>
	Position & getAlign(const P & pos, const A & axis);


	/// Return alignment setting of an object along horizontal or vertical axis  .
	/*
	 *  \tparam P - enum type Owner \c REF or \c OBJ , or respective string.
	 *  \tparam A - enum type axis_t \c HORZ or \c VERT , or respective string.
	 *  \param pos - target object \c OBJ or referred anchor object \c REF
	 *  \param axis - horizontal \c HORZ or vertical \c AXIS .
	 *
	 */
	template <typename P, typename A>
	const Position & getAlign(const P & pos, const A & axis) const;


	/*
	inline
	const Position & getAlign(Owner owner, Axis axis) const {
		return alignments[owner][axis];
	}

	inline
	Position & getAlign(Owner owner, Axis axis) {
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

	typedef std::vector<Align::Position> align_vect_t;
	typedef std::vector<align_vect_t > align_conf_t;

	/// alignments[OBJECT][VERT] = alignments[OBJECT|ANCHOR][HORZ|VERT] = [MIN|MID|MAX]
	align_conf_t alignments = align_conf_t(2, align_vect_t(2, Align::Position::UNDEFINED_POS));


	/*
	template <typename ...T>
	void setAlign(const T... args){
		alignment |= combineAlign(args...);
		//alignment = 0;
		updateAlign();
	};

	// For staff use only... for avoiding contradictory HORZ,MIN,MAX
	template <typename ...TT>
	static
	bitvect_t combineAlign(bitvect_t arg, const TT... args) {
		return arg & combineAlign(args...);
	}

	static inline
	bitvect_t combineAlign(){
		return ~bitvect_t(0);
	}
	*/

	// void updateAlignStr();

};

template <>
const drain::EnumDict<AlignSVG::Owner>::dict_t drain::EnumDict<AlignSVG::Owner>::dict;
DRAIN_ENUM_OSTREAM(AlignSVG::Owner);

template <>
const drain::EnumDict<AlignSVG::Topol>::dict_t drain::EnumDict<AlignSVG::Topol>::dict;
DRAIN_ENUM_OSTREAM(AlignSVG::Topol);


template <typename P, typename A>
Align::Position & AlignSVG::getAlign(const P & owner, const A & axis){
	const AlignSVG::Owner p = EnumDict<AlignSVG::Owner>::getValue(owner);
	const Align::Axis a     = EnumDict<Align::Axis>::getValue(axis);
	return alignments[p][a];
}

template <typename P, typename A>
const Align::Position & AlignSVG::getAlign(const P & owner, const A & axis) const {
	const AlignSVG::Owner p  = EnumDict<AlignSVG::Owner>::getValue(owner);
	const Align::Axis a      = EnumDict<Align::Axis>::getValue(axis);
	return alignments[p][a];
}

//template <>
// const drain::EnumDict<AlignSVG::Direction>::dict_t  drain::EnumDict<AlignSVG::Direction>::dict;




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
		INCR,
		DECR
	};
	typedef drain::EnumFlagger<drain::SingleFlagger<Direction> > DirectionFlagger;
	DirectionFlagger direction = INCR;

	// Experimental
	enum GroupType {
		HEADER,
		ALIGN_SCOPE,
		FLOAT,       // = element does not affect alignment of other elems
		// ALIGNED,  // needed? If anchor set,
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

/*
template <>
const drain::EnumDict<image::LayoutSVG::Axis>::dict_t  drain::EnumDict<image::LayoutSVG::Axis>::dict;
*/
template<>
const EnumDict<LayoutSVG::GroupType>::dict_t EnumDict<LayoutSVG::GroupType>::dict;


template <>
const drain::EnumDict<image::LayoutSVG::Direction>::dict_t  drain::EnumDict<image::LayoutSVG::Direction>::dict;




/*
/// Specific instructions for implementing a layout.
class AlignSVG_OLD {

public:

	inline
	AlignSVG_OLD(){};

	inline
	AlignSVG_OLD(const AlignSVG_OLD & conf) : alignments(conf.alignments){};

	inline virtual
	~AlignSVG_OLD(){};

	/// Horizontal position (inside or outside reference object).
	enum HorzAlign {
		LEFT,
		CENTER,
		RIGHT,
	};

	/// Vertical position (inside or outside reference object).
	enum VertAlign {
		TOP,
		MIDDLE,
		BOTTOM,
	};


	/// Switch for referring to SVG element itself or an anchor object.
	enum Owner {
		OBJ=0,
		REF=1,
	};

	/// Switch for horizontal or vertical position.
	enum Position {
		UNDEFINED=0,
		MIN,
		MID,
		MAX,
		// ABSOLUTE?
	}

	// template <typename H>
	inline
	void alignInside(const HorzAlign & ha, const VertAlign & va){
		alignInside(ha);
		alignInside(va);
	}

	inline
	void alignInside(const HorzAlign & ha){

		// const HorzAlign horzAlign = EnumDict<HorzAlign>::getValue(ha);
		// AlignSVG::Position & horzRef = getAlign(Owner::REF, LayoutSVG::HORZ);
		// AlignSVG::Position & horzObj = getAlign(Owner::OBJ, LayoutSVG::HORZ);

		switch (EnumDict<HorzAlign>::getValue(ha)){
			case HorzAlign::LEFT:
				setAlignInside(Align::HORZ, Align::MIN);
				break;
			case HorzAlign::CENTER:
				setAlignInside(Align::HORZ, Align::MID);
				break;
			case HorzAlign::RIGHT:
				setAlignInside(Align::HORZ, Align::MAX);
				break;
			default:
				break;
		}
	}

	// template <typename H>
	inline
	void alignInside(const VertAlign & va){

		switch (EnumDict<VertAlign>::getValue(va)){
			case VertAlign::TOP:
				setAlignInside(Align::VERT, Align::MIN);
				break;
			case VertAlign::MIDDLE:
				setAlignInside(Align::VERT, Align::MID);
				break;
			case VertAlign::BOTTOM:
				setAlignInside(Align::VERT, Align::MAX);
				break;
			default:
				break;
		}
		// getAlign(Owner::REF, LayoutSVG::VERT) = Align::MIN;
		// getAlign(Owner::OBJ, LayoutSVG::VERT) = Align::MAX;

		// updateAlign();
	}

	/// Set a single alignment setting.
	 *  \tparam P - enum type \c Position  or string
	 *  \tparam A - enum type \c Axis or string
	 *  \tparam V - enum type \c Alignment or string
	 *  \param pos   - enum value \c OBJ or \c REF
	 *  \param axis  - enum value \c HORZ or \c VERT
	 *  \param value - enum value \c MAX , \c MID , or \c MIN (or string)
	template <typename P, typename A, typename V>
	void setAlign(const P & owner, const A & axis,  const V &value){
		getAlign(owner, axis) = EnumDict<Position>::getValue(value);
		updateAlign();
	}

	/// Set all alignment settings.
	void setAlign(const AlignSVG & conf);

	// todo hide and requestAl

	/// Align element inside anchor element.
	template <typename A,typename V>
	void setAlignInside(const A & axis,  const V &value){
		const Align::Position v = EnumDict<Align::Position>::getValue(value);
		getAlign(AlignSVG::Owner::ANCHOR, axis) = v;
		getAlign(AlignSVG::Owner::OBJECT, axis) = Align::flip(v);
		updateAlign();
	}


	/// Align element outside anchor element.
	template <typename A,typename V>
	void setAlignOutside(const A & axis,  const V &value){
		const Align::Position v = EnumDict<Align::Position>::getValue(value);
		getAlign(AlignSVG::Owner::ANCHOR, axis) = v;
		getAlign(AlignSVG::Owner::OBJECT, axis) = v;
		updateAlign();
	}

	inline
	void setMajorAlignment(Align::Axis v, LayoutSVG::Direction d){
		setAlignOutside(v==Align::HORZ ? Align::HORZ : Align::VERT, d==LayoutSVG::INCR ? Align::MAX : Align::MIN);
	}



	/// Return alignment setting of an object along horizontal or vertical axis  .
	**
	 *  \tparam P - enum type Owner \c REF or \c OBJ , or respective string.
	 *  \tparam A - enum type axis_t \c HORZ or \c VERT , or respective string.
	 *  \param pos - target object \c OBJ or referred anchor object \c REF
	 *  \param axis - horizontal \c HORZ or vertical \c AXIS .
	 *
	template <typename P, typename A>
	AlignSVG::Position & getAlign(const P & pos, const A & axis);

	/// Return alignment setting of an object along horizontal or vertical axis  .
	**
	 *  \tparam P - enum type Owner \c REF or \c OBJ , or respective string.
	 *  \tparam A - enum type axis_t \c HORZ or \c VERT , or respective string.
	 *  \param pos - target object \c OBJ or referred anchor object \c REF
	 *  \param axis - horizontal \c HORZ or vertical \c AXIS .
	 *
	template <typename P, typename A>
	const AlignSVG::Position & getAlign(const P & pos, const A & axis) const;

	/// Reset all alignments
	void clearAlign();

	/// Return true, if any of alignment requests has been set.
	bool isAligned() const;


protected:

	typedef std::vector<Align::Position> align_vect_t;
	typedef std::vector<align_vect_t > align_conf_t;

	// Alignment "matrix": {ORIG,REF} × {HORZ,VERT} = [MAX|MID|MIN|UNDEFINED]
	align_conf_t alignments = align_conf_t(2, align_vect_t(2, Align::Position::UNDEFINED_POS));

	virtual
	void updateAlign(){
	};

};
*/

/*
template <typename P, typename A>
Align::Position & AlignSVG_OLD::getAlign(const P & pos, const A & axis){
	const AlignSVG::Owner p   = EnumDict<AlignSVG::Owner>::getValue(pos);
	const Align::Axis a    = EnumDict<Align::Axis>::getValue(axis);
	return alignments[p][a];
}

template <typename P, typename A>
const Align::Position & AlignSVG_OLD::getAlign(const P & pos, const A & axis) const {
	const AlignSVG::Owner p  = EnumDict<AlignSVG::Owner>::getValue(pos);
	const Align::Axis a   = EnumDict<Align::Axis>::getValue(axis);
	return alignments[p][a];
}
*/




/// Adapter designed for NodeSVG
struct AlignAdapterSVG : public AlignSVG {

	/// Mark one of the elements of this object (SVG or G) as a decisive position
	inline
	void setAlignAnchor(const std::string & pathElem){
		anchor = pathElem;
		updateAlign();
	}

	inline
	const std::string & getAlignAnchor() const {
		return anchor;
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

	//std::string align;

	std::string anchor;



private:

	void updateAlignStr();


};






}  // image::

}  // drain::


DRAIN_ENUM_OSTREAM(drain::image::Align::Axis);
DRAIN_ENUM_OSTREAM(drain::image::Align::Position);

DRAIN_ENUM_OSTREAM(drain::image::AlignSVG::Owner);

DRAIN_ENUM_OSTREAM(drain::image::LayoutSVG::Direction);


#endif // DRAIN_ALIGN_SVG_H_

