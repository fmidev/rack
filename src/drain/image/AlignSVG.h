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

class AlignConfSVG {

public:

	inline
	AlignConfSVG(){};

	inline
	AlignConfSVG(const AlignConfSVG & conf) : alignments(conf.alignments){};

	inline virtual
	~AlignConfSVG(){};

	enum pos_t {
		OBJ=0,
		REF=1,
	};

	enum axis_t {
		HORZ=0,
		VERT=1,
	};

	enum value_t {
		UNDEFINED=0,
		MIN,
		MID,
		MAX,
		// ABSOLUTE?
	};

	inline
	value_t flip(value_t v){
		switch (v){
		case MAX:
			return MIN;
		case MIN:
			return MAX;
		default:
			return v;
		}
	};
	/*
	typedef EnumFlagger<SingleFlagger<AlignSVG2::value_t> > Alignment;
	typedef EnumFlagger<SingleFlagger<AlignSVG2::axis_t> > Axis;
	typedef EnumFlagger<SingleFlagger<AlignSVG2::pos_t> > Position;
	*/

	template <typename P, typename A,typename V>
	void setAlign(const P & pos, const A & axis,  const V &value){
		getAlign(pos, axis) = EnumDict<value_t>::getValue(value);
		updateAlign();
	}

	/// Align element inside anchor element.
	/**
	 *
	 *  Consider additional enums: LEFT, RIGHT and <IN>, <OUT>
	 *
	 */
	template <typename A,typename V>
	void setAlignInside(const A & axis,  const V &value){
		const value_t v = EnumDict<value_t>::getValue(value);
		getAlign(pos_t::REF, axis) = v;
		getAlign(pos_t::OBJ, axis) = flip(v);
		updateAlign();
	}

	/// Align element outside anchor element.
	/**
	 *  Consider additional enums: LEFT, RIGHT and <IN>, <OUT>
	 */
	template <typename A,typename V>
	void setAlignOutside(const A & axis,  const V &value){
		const value_t v = EnumDict<value_t>::getValue(value);
		getAlign(pos_t::REF, axis) = v;
		getAlign(pos_t::OBJ, axis) = v;
		updateAlign();
	}

	/// Return alignment setting of an object along horizontal or vertical axis  .
	/**
	 *  \tparam P - enum type pos_t \c REF or \c OBJ , or respective string.
	 *  \tparam A - enum type axis_t \c HORZ or \c VERT , or respective string.
	 *  \param pos - target object \c OBJ or referred anchor object \c REF
	 *  \param axis - horizontal \c HORZ or vertical \c AXIS .
	 */
	template <typename P, typename A>
	value_t & getAlign(const P & pos, const A & axis);

	/// Return alignment setting of an object along horizontal or vertical axis  .
	/**
	 *  \tparam P - enum type pos_t \c REF or \c OBJ , or respective string.
	 *  \tparam A - enum type axis_t \c HORZ or \c VERT , or respective string.
	 *  \param pos - target object \c OBJ or referred anchor object \c REF
	 *  \param axis - horizontal \c HORZ or vertical \c AXIS .
	 */
	template <typename P, typename A>
	const value_t & getAlign(const P & pos, const A & axis) const;

	/// Reset all alignments
	void clearAlign();

	/// Return true, if any of alignment requests has been set.
	bool isAligned() const;

protected:

	typedef std::vector<value_t> align_vect_t;
	typedef std::vector<align_vect_t > align_conf_t;

	// Alignment "matrix": {ORIG,REF} × {HORZ,VERT} = [MAX|MID|MIN|UNDEFINED]
	align_conf_t alignments = align_conf_t(2, align_vect_t(2, value_t::UNDEFINED));

	virtual
	void updateAlign(){
	};

	/*
	template <typename V>
	static inline
	const V & getValue(const std::string &key){
		return EnumDict<V>::dict.getValue(key);
	}

	template <typename V>
	static inline
	const V & getValue(const char &key){
		return EnumDict<V>::dict.getValue(key);
	}

	template <typename V>
	static inline
	const V & getValue(const V &value){
		return value;
	}
	*/

};


template <typename P, typename A>
AlignConfSVG::value_t & AlignConfSVG::getAlign(const P & pos, const A & axis){
	const pos_t p   = EnumDict<pos_t>::getValue(pos);
	const axis_t a  = EnumDict<axis_t>::getValue(axis);
	return alignments[p][a];
}

template <typename P, typename A>
const AlignConfSVG::value_t & AlignConfSVG::getAlign(const P & pos, const A & axis) const {
	const pos_t p   = EnumDict<pos_t>::getValue(pos);
	const axis_t a  = EnumDict<axis_t>::getValue(axis);
	return alignments[p][a];
}





/// Adapter designed for NodeSVG
struct AlignSVG2 : public AlignConfSVG {

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

protected:

	inline virtual
	~AlignSVG2(){};


	std::string align;

	std::string anchor;

	virtual
	void updateAlign(){
		updateAlignStr();
	};

private:

	void updateAlignStr();


};




template<>
const EnumDict<AlignSVG2::value_t>::dict_t EnumDict<AlignSVG2::value_t>::dict;

/// Higher level controller for setting alignments.
/**
 *  Also applied by PanelSVG
 */
class LayoutSVG {

public:

	enum Orientation {UNDEFINED_ORIENTATION=0, HORZ, VERT};
	typedef drain::EnumFlagger<drain::SingleFlagger<Orientation> > OrientationFlagger;
	OrientationFlagger orientation = HORZ;

	enum Direction {UNDEFINED_DIRECTION=0, INCR, DECR};
	typedef drain::EnumFlagger<drain::SingleFlagger<Direction> > DirectionFlagger;
	DirectionFlagger direction = INCR;

	inline
	LayoutSVG(Orientation v=HORZ, Direction d=INCR) : orientation(v), direction(d) {
	}

	inline
	LayoutSVG(const LayoutSVG & layout) : orientation(layout.orientation), direction(layout.direction){
	}

	template <typename V>
	inline
	void setOrientation(const V &v){
		orientation.set(EnumDict<Orientation>::getValue(v));
	};

	template <typename D>
	inline
	void setDirection(const D & d){
		direction.set(EnumDict<Direction>::getValue(d));
	};

	/// Set direction and orientation
	/**
	 *
	 */
	template <typename D, typename V>
	inline
	void set(const D & d, const V &v){
		direction.set(EnumDict<Orientation>::getValue(d));
		orientation.set(EnumDict<Orientation>::getValue(v));
	};

	static inline
	void getAlignmentConf(Orientation v, Direction d, AlignConfSVG & alignConf){
		alignConf.setAlignOutside(v==LayoutSVG::HORZ ? AlignConfSVG::HORZ : AlignConfSVG::VERT, d==LayoutSVG::INCR ? AlignConfSVG::MAX : AlignConfSVG::MIN);
	}

};

template <>
const drain::EnumDict<image::LayoutSVG::Orientation>::dict_t  drain::EnumDict<image::LayoutSVG::Orientation>::dict;

template <>
const drain::EnumDict<image::LayoutSVG::Direction>::dict_t  drain::EnumDict<image::LayoutSVG::Direction>::dict;

}  // image::

}  // drain::



#endif // DRAIN_ALIGN_SVG_H_

