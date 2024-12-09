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
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#include "AlignSVG.h"

namespace drain {

// DRAIN_TYPENAME_DEF(image::NodeSVG);

namespace image {



template <>
const drain::EnumDict<Align::Position>::dict_t drain::EnumDict<Align::Position>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::Align::Position, MAX),
		DRAIN_ENUM_ENTRY(drain::image::Align::Position, MID),
		DRAIN_ENUM_ENTRY(drain::image::Align::Position, MIN),
};

template <>
const drain::EnumDict<Align::Axis>::dict_t drain::EnumDict<Align::Axis>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::Align::Axis, HORZ),
		DRAIN_ENUM_ENTRY(drain::image::Align::Axis, VERT),
};

template <>
const drain::EnumDict<AlignSVG::Owner>::dict_t drain::EnumDict<AlignSVG::Owner>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG::Owner, OBJECT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG::Owner, ANCHOR),
};

template <>
const drain::EnumDict<AlignSVG::HorzAlign>::dict_t  drain::EnumDict<AlignSVG::HorzAlign>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG::HorzAlign, LEFT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG::HorzAlign, CENTER),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG::HorzAlign, RIGHT),
};

template <>
const drain::EnumDict<AlignSVG::VertAlign>::dict_t  drain::EnumDict<AlignSVG::VertAlign>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG::VertAlign, TOP),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG::VertAlign, MIDDLE),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG::VertAlign, BOTTOM),
};


template<>
const EnumDict<LayoutSVG::GroupType>::dict_t EnumDict<LayoutSVG::GroupType>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, HEADER),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, ALIGN_SCOPE),
		//DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, ALIGNED),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, FLOAT),
};

/*
template <>
const drain::EnumDict<LayoutSVG::Axis>::dict_t  drain::EnumDict<LayoutSVG::Axis>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Axis, HORZ),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Axis, VERT),
};
*/


template <>
const drain::EnumDict<LayoutSVG::Direction>::dict_t  drain::EnumDict<LayoutSVG::Direction>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Direction, INCR),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Direction, DECR)
};

template <>
const drain::EnumDict<AlignSVG::Topol>::dict_t drain::EnumDict<AlignSVG::Topol>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG::Topol, INSIDE),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG::Topol, OUTSIDE),
};


/*
Align::Position AlignSVG::getAlignPos(Owner owner, Axis axis) const{

	bitvect_t v = combineAlign((int)owner, (int)axis);

	if (isAlignSet(v, Position::MID)){ // both bits: = MIN|MAX
		return Position::MID;
	}
	else if (isAlignSet(v, Position::MIN)){
		return Position::MIN;
	}
	else if (isAlignSet(v, Position::MAX)){
		return Position::MAX;
	}
	else {
		return Position::UNDEFINED_POS;
	}
}
*/

/*
static
void LayoutSVG::getAlignmentConf(LayoutSVG::Direction d, LayoutSVG::Axis v, AlignSVG & alignConf){
	alignConf.setAlignOutside(d==LayoutSVG::HORZ ? AlignSVG::HORZ : AlignSVG::VERT, v==LayoutSVG::INCR ? AlignSVG::MAX : AlignSVG::MIN);
}
*/


/*
template<>
const EnumDict<AlignAdapterSVG::axis_t>::dict_t EnumDict<AlignAdapterSVG::axis_t>::dict = {
	DRAIN_ENUM_ENTRY(drain::image::AlignAdapterSVG::axis_t, HORZ),
	DRAIN_ENUM_ENTRY(drain::image::AlignAdapterSVG::axis_t, VERT),
};
*/

/*
template<>
const EnumDict<AlignSVG::Owner>::dict_t EnumDict<AlignSVG::Owner>::dict = {
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG::Owner, OBJECT),
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG::Owner, ANCHOR),
};

template<>
const EnumDict<Align::Position>::dict_t EnumDict<Align::Position>::dict = {
	DRAIN_ENUM_ENTRY(drain::image::Align::Position, UNDEFINED_POS),
	DRAIN_ENUM_ENTRY(drain::image::Align::Position, MAX),
	DRAIN_ENUM_ENTRY(drain::image::Align::Position, MID),
	DRAIN_ENUM_ENTRY(drain::image::Align::Position, MIN),
	// {drain::image::svg::MAX, "MAX"}
};
*/


/*
void AlignSVG::setAlign(const AlignSVG & conf){
	for (AlignSVG::Owner p: {AlignSVG::OBJ, AlignSVG::REF}){
		for (LayoutSVG::Axis a: {LayoutSVG::Axis::HORZ, LayoutSVG::Axis::VERT}){
			setAlign(p, a, conf.getAlign(p, a));
		}
	}
}
*/


void AlignSVG::resetAlign(){
	for (align_vect_t & a: alignments){
		for (Position & v: a){
			v = Position::UNDEFINED_POS;
		}
	}
};

bool AlignSVG::isAligned() const {
	for (const align_vect_t & a: alignments){
		for (const Position & v: a){
			if (v != Position::UNDEFINED_POS){
				return true;
			}
		}
	}
	return false;
};


void AlignAdapterSVG::updateAlignStr(){

	std::stringstream sstr;
	char sep=0;

	alignment = 0;

	int bitShift=0;
	for (AlignSVG::Owner owner: {Owner::OBJECT, Owner::ANCHOR}){
		bitShift = (owner == Owner::OBJECT) ? 0 : 4;

		for (Align::Axis axis: {Align::Axis::HORZ, Align::Axis::VERT}){

			bitShift += (axis == Axis::HORZ) ? 0 : 2;

			const Align::Position & pos = getAlign(owner, axis);

			if (pos != Align::UNDEFINED_POS){
				if (sep)
					sstr << sep;
				else
					sep=' ';
				sstr << owner << ':' << axis << '-' << pos;
				//std::cerr << __FUNCTION__ << ':' << EnumDict<AlignAdapterSVG::Owner>::dict.getKey(p) << '_' << EnumDict<AlignAdapterSVG::axis_t>::dict.getKey(a) << '_' << EnumDict<AlignAdapterSVG::Position>::dict.getKey(v) << '_' << (int)v << '\n';
			}

			alignment |= (((bitvect_t)pos)<<bitShift);
		}
	}

	// std::string s = sstr.str();
	alignStr = sstr.str();

}



/*
void AlignAdapterSVG::updateAlignStr(){

	std::stringstream sstr;
	char sep=0;

	for (AlignSVG::Owner p: {AlignSVG::OBJ, AlignSVG::REF}){
		for (LayoutSVG::Axis a: {LayoutSVG::Axis::HORZ, LayoutSVG::Axis::VERT}){

			const AlignAdapterSVG::Position & v = getAlign(p, a);
			if (v != AlignSVG::UNDEFINED){
				if (sep)
					sstr << sep;
				else
					sep=' ';
				sstr << EnumDict<AlignSVG::Owner>::dict.getKey(p) << ':' << EnumDict<LayoutSVG::Axis>::dict.getKey(a) << '-' << EnumDict<AlignSVG::Position>::dict.getKey(v);
				//std::cerr << __FUNCTION__ << ':' << EnumDict<AlignAdapterSVG::Owner>::dict.getKey(p) << '_' << EnumDict<AlignAdapterSVG::axis_t>::dict.getKey(a) << '_' << EnumDict<AlignAdapterSVG::Position>::dict.getKey(v) << '_' << (int)v << '\n';
			}
		}
	}
	// std::string s = sstr.str();
	align = sstr.str();
}
*/



}  // image::

}  // drain::


