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

#ifndef DRAIN_ALIGNMENT1
#define DRAIN_ALIGNMENT1

#include <drain/Enum.h>
#include <string>

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


	/// supporting simultaneous HORZ|VERT
	/*
	enum Axis {
		// Index, must contain 0 and 1 for HORZ and VERT
		UNDEFINED_AXIS = 0,
		HORZ      = 1, // 0b00110011,
		VERT      = 2, // b11001100,
	};
	*/

	static inline
	Axis flip(Axis axis){
		switch (axis){
		case HORZ:
			return VERT;
		case VERT:
			return HORZ;
		default:
			return UNDEFINED_AXIS;
		}
	};

	//typedef drain::Enum<Coord>::dict_t pos_dict_t;

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

/// Container for Axis and Pos.
/**
 *   Explicit type parameters AX supports setting member \c axis a non-const or const value (HORZ or VERT).
 *
 *   \tparam AX - const or non-const Align::Axis
 *   \tparam  A - axis init value AlignBase::Axis::HORZ or AlignBase::Axis::VERT
 *
 *
 *   For \c const implementations, see AlignSVG
 *   Alignment<Align::Axis::HORZ>
 *   Alignment<Align::Axis::VERT>
 */
template <typename AX = AlignBase::Axis, AlignBase::Axis A = AlignBase::Axis::UNDEFINED_AXIS> //, typename POS = AlignBase::Pos> // , Align::Coord POS = Align::Coord::UNDEFINED_POS>
struct Alignment {

	// Align::Axis
	AX axis; // = V; // compiler error if different type?

	//AlignBase::Pos pos
	AlignBase::Pos pos = AlignBase::Pos::UNDEFINED_POS; // or middle?
	//POS pos = AlignBase::Pos::UNDEFINED_POS; // or middle?

	/// Default constructor
	inline
	Alignment(AlignBase::Pos pos = AlignBase::Pos::UNDEFINED_POS) : axis(A), pos(pos){
	}

	/// Copy constructor
	inline
	Alignment(const Alignment & ac) : axis(ac.axis), pos(ac.pos){
	}


	inline
	Alignment(AlignBase::Axis axis, AlignBase::Pos pos = AlignBase::Pos::UNDEFINED_POS): axis(axis), pos(pos){
	}

	template <typename AX2, AlignBase::Axis A2>
	inline
	Alignment(const Alignment<AX2,A2> & align) : pos(align.pos){ // axis(ac.axis),
		axis = align.axis; // error if const
	}

	/// Destructor.
	inline virtual
	~Alignment(){};


	inline
	const AlignBase::Axis & get(const AlignBase::Axis & defaultValue) const {
		if (axis != AlignBase::Axis::UNDEFINED_AXIS){
			return axis;
		}
		else {
			return defaultValue;
		}
	}

	inline
	const AlignBase::Pos & get(const AlignBase::Pos & defaultValue) const {
		if (pos != AlignBase::Pos::UNDEFINED_POS){
			return pos;
		}
		else {
			return defaultValue;
		}
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

	/*
	inline
	Alignment<AX,A> & operator=(Alignment<AX,A> & align){
		// axis = align.axis;
		pos  = align.pos;
		return *this;
	}
	*/




};




struct MutualAlign {

	enum Topol {
		INSIDE = 0,
		OUTSIDE = 1,
		UNDEFINED_TOPOL = 2,
	};


};


/// "Alternative" \e partial alignment configuration for single object. Partial means that either \c OBJECT itself or \c ANCHOR object is set.
/**
 *  Extends Alignment with topology, \c Topol (\c INSIDE or \c OUTSIDE ).
 *  Note: still for one axis only.
 *
 *  Essentially, CompleteAlignment is a triplet of types <Topol,Axis,Coordinate>, out of which Axis may be const.
 *
 *
 *  Designed to handle command line arguments, adjusting AlignSVG::HorzAlign and AlignSVG::VertAlign
 *
 *  \see AlignSVG::HorzAlign
 *  \see AlignSVG::VertAlign
 *  \see CompleteHorzAlign
 *  \see CompleteVertAlign
 *
 *
 */
template <typename AX = AlignBase::Axis, AlignBase::Axis A = AlignBase::Axis::UNDEFINED_AXIS> // , Align::Coord POS = Align::Coord::UNDEFINED_POS>
struct CompleteAlignment : public Alignment<AX,A> {

	MutualAlign::Topol topol = MutualAlign::Topol::INSIDE; // or undef?

	/// Constructor not setting Axis.
	template <class ...TT>
	CompleteAlignment(const TT... args) : Alignment<AX,A>() {
		set(args...);
	}

	inline
	~CompleteAlignment(){};

	virtual inline
	bool isSet() const {
		return (this->axis != AlignBase::Axis::UNDEFINED_AXIS) &&
				(this->pos != AlignBase::Pos::UNDEFINED_POS) &&
				(topol != MutualAlign::Topol::UNDEFINED_TOPOL);
		// return Alignment<AX,A>::isSet() && (topol != MutualAlign::Topol::UNDEFINED_TOPOL);
	}

	inline
	const MutualAlign::Topol & getOrDefault(const MutualAlign::Topol & defaultValue) const {
		if (topol != MutualAlign::Topol::UNDEFINED_TOPOL){
			return topol;
		}
		else {
			return defaultValue;
		}
	}


	// Sets all members to UNDEFINED state.
	virtual inline
	void reset(){
		Alignment<AX,A>::reset();
		topol = MutualAlign::Topol::UNDEFINED_TOPOL;
		// this->updateAlign();
	}

	/// Variant for same-AX-type, HORZ or VERT
	/**
	 *  Does not try to change axis...
	 */
	// template <typename AX2, AlignBase::Axis A2, class ...TT>
	//void set(const Alignment<const AX2,A2> & align, const TT... args){
	template <class ...TT>
	void set(const Alignment<AX,A> & align, const TT... args){
		// this->axis = align.axis;
		this->pos  = align.pos;
		set(args...);
	}

	/// Variant for non-const axis AX2.
	template <typename AX2, AlignBase::Axis A2, class ...TT>
	void set(const Alignment<AX2,A2> & align, const TT... args){
		this->axis = align.axis;
		this->pos  = align.pos;
		set(args...);
	}



	template <class ...TT>
	void set(MutualAlign::Topol topol, const TT... args){
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
		if (Enum<MutualAlign::Topol>::setValue(key, topol)){
			// ok
		}
		else if (Enum<Alignment<> >::setValue(key, *this)){ // RIGHT or?
			// ok
		}
		else if (Enum<AlignBase::Axis>::setValue(key, this->axis)){
			// ok
		}
		else if (Enum<AlignBase::Pos>::setValue(key, this->pos)){
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
		// this->updateAlign(); // ok?
	}

};


typedef CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::HORZ> CompleteHorzAlign;
typedef CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::VERT> CompleteVertAlign;


}  // image::

DRAIN_ENUM_DICT(image::AlignBase::Axis);
DRAIN_ENUM_DICT(image::AlignBase::Pos);
//DRAIN_ENUM_DICT(image::Alignment<>);
DRAIN_ENUM_DICT(image::MutualAlign::Topol);

DRAIN_ENUM_OSTREAM(drain::image::AlignBase::Axis);
DRAIN_ENUM_OSTREAM(drain::image::AlignBase::Pos);


template <typename AX, drain::image::AlignBase::Axis A>
inline
std::ostream & operator<<(std::ostream &ostr, const drain::image::Alignment<AX,A> & align){
	//return ostr << align.axis << '_' << align.pos; // enums resolved above
	return ostr << align.axis << '_' << align.pos;
}


//DRAIN_ENUM_DICT(drain::image::Alignment<>);
DRAIN_ENUM_OSTREAM(drain::image::MutualAlign::Topol);

template <typename AX, drain::image::AlignBase::Axis A>
std::ostream & operator<<(std::ostream &ostr, const drain::image::CompleteAlignment<AX,A> & ad){
	return ostr << ad.topol << '_' << ad.axis << ':' << ad.pos;
}

}  // drain::




// DRAIN_ENUM_OSTREAM(drain::image::Alignment<>);

// DRAIN_ENUM_OSTREAM(drain::image::AlignBase::Axis);
// DRAIN_ENUM_OSTREAM(drain::image::AlignBase::Pos);



#endif // DRAIN_ALIGNMENT

