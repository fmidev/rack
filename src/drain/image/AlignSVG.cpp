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
const drain::EnumDict<AlignBase::Pos>::dict_t drain::EnumDict<AlignBase::Pos>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Pos, UNDEFINED_POS),
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Pos, MAX),
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Pos, MID),
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Pos, MIN),
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Pos, FILL),
};

template <>
const drain::EnumDict<AlignBase::Axis>::dict_t drain::EnumDict<AlignBase::Axis>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Axis, HORZ),
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Axis, VERT),
};

// -----------------------------------------------------------------------------------------------------------

template <>
const drain::EnumDict<AlignSVG::Owner>::dict_t drain::EnumDict<AlignSVG::Owner>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG::Owner, OBJECT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG::Owner, ANCHOR),
};


const HorzAlign AlignSVG::RIGHT(AlignBase::Pos::MAX);
const HorzAlign AlignSVG::CENTER(AlignBase::Pos::MID);
const HorzAlign AlignSVG::LEFT(AlignBase::Pos::MIN);
const HorzAlign AlignSVG::UNDEFINED_HORZ(AlignBase::Pos::UNDEFINED_POS);

template <>
const drain::EnumDict<HorzAlign>::dict_t  drain::EnumDict<HorzAlign>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, LEFT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, CENTER),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, RIGHT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, UNDEFINED_HORZ),
};

const VertAlign AlignSVG::BOTTOM(AlignBase::Pos::MAX);
const VertAlign AlignSVG::MIDDLE(AlignBase::Pos::MID);
const VertAlign AlignSVG::TOP(AlignBase::Pos::MIN);
const VertAlign AlignSVG::UNDEFINED_VERT(AlignBase::Pos::UNDEFINED_POS);

template <>
const drain::EnumDict<VertAlign>::dict_t  drain::EnumDict<VertAlign>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, TOP),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, MIDDLE),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, BOTTOM),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, UNDEFINED_VERT),
};


/// Combines both EnumDict<Alignment<> >:

template <>
const drain::EnumDict<AlignCoord<> >::dict_t  drain::EnumDict<AlignCoord<> >::dict = {
		/*
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, LEFT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, CENTER),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, RIGHT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, UNDEFINED_HORZ),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, TOP),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, MIDDLE),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, BOTTOM),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, UNDEFINED_VERT),
		*/
};
//EnumDict<Alignment<> >


AlignCoord<> test(drain::image::AlignSVG::LEFT);
AlignCoord<> test2 = drain::image::AlignSVG::LEFT;

/*
template <>
const drain::EnumDict<AlignSVG>::dict_t  drain::EnumDict<AlignSVG>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, LEFT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, CENTER),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, RIGHT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, UNDEFINED_HORZ),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, TOP),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, MIDDLE),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, BOTTOM),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, UNDEFINED_VERT),
};
*/
/// Handler for command line or configuration file arguments
void AlignSVG::setAlign(const std::string & align){

	std::list<std::string> args;
	drain::StringTools::split(align, args, ','); // todo: separators?

	for (const std::string & arg: args){

		std::vector<std::string> conf;
		drain::StringTools::split(arg, conf, ':');

		switch (conf.size()){
		case 3:
			setAlign(conf[0], conf[1], conf[2]);
			break;
		case 2:
			setAlign(conf[0], conf[1]);
			break;
		default:
			drain::Logger mout(__FUNCTION__, __FILE__);
			mout.advice<LOG_NOTICE>("Use 3 args, example: OBJECT:HORZ:MIN,ANCHOR:VERT:MAX");
			mout.advice<LOG_NOTICE>("use 2 args, example: OUTSIDE:LEFT,INSIDE:TOP");
			mout.error("Could not parse: ", arg);
		}
	}

}


void AlignSVG::confToStream(std::ostream & ostr) const {

	// std::stringstream sstr;
	char sep=0;

	alignment = 0;

	int bitShift=0;
	for (AlignSVG::Owner owner: {Owner::OBJECT, Owner::ANCHOR}){
		bitShift = (owner == Owner::OBJECT) ? 0 : 4;

		for (AlignBase::Axis axis: {AlignBase::Axis::HORZ, AlignBase::Axis::VERT}){

			bitShift += (axis == AlignBase::Axis::HORZ) ? 0 : 2;

			const AlignBase::Pos & pos = getAlign(owner, axis);

			if (pos != AlignBase::UNDEFINED_POS){
				if (sep)
					ostr << sep;
				else
					sep=' ';
				ostr << owner << ':' << axis << '-' << pos;
				//std::cerr << __FUNCTION__ << ':' << EnumDict<AlignAdapterSVG::Owner>::dict.getKey(p) << '_' << EnumDict<AlignAdapterSVG::axis_t>::dict.getKey(a) << '_' << EnumDict<AlignAdapterSVG::Coord>::dict.getKey(v) << '_' << (int)v << '\n';
			}

			alignment |= (((bitvect_t)pos)<<bitShift);
		}
	}


}

// -----------------------------------------------------------------------------------------------------------

template<>
const EnumDict<LayoutSVG::GroupType>::dict_t EnumDict<LayoutSVG::GroupType>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, HEADER),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, ALIGN_FRAME),
		// DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, ALI GNED),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, ABSOLUTE),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, FLOAT),
};

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



void AlignSVG::resetAlign(){
	for (align_vect_t & a: alignments){
		for (AlignBase::Pos & v: a){
			v = AlignBase::Pos::UNDEFINED_POS;
		}
	}
};

bool AlignSVG::isAligned() const {
	for (const align_vect_t & a: alignments){
		for (const AlignBase::Pos & v: a){
			if (v != AlignBase::Pos::UNDEFINED_POS){
				return true;
			}
		}
	}
	return false;
};





void AlignAdapterSVG::updateAlignStr(){

	std::stringstream sstr;
	confToStream(sstr);
	alignStr = sstr.str();

	/*
	char sep=0;

	alignment = 0;

	int bitShift=0;
	for (AlignSVG::Owner owner: {Owner::OBJECT, Owner::ANCHOR}){
		bitShift = (owner == Owner::OBJECT) ? 0 : 4;

		for (Align::Axis axis: {Align::Axis::HORZ, Align::Axis::VERT}){

			bitShift += (axis == Axis::HORZ) ? 0 : 2;

			const Align::Coord & pos = getAlign(owner, axis);

			if (pos != Align::UNDEFINED_POS){
				if (sep)
					sstr << sep;
				else
					sep=' ';
				sstr << owner << ':' << axis << '-' << pos;
				//std::cerr << __FUNCTION__ << ':' << EnumDict<AlignAdapterSVG::Owner>::dict.getKey(p) << '_' << EnumDict<AlignAdapterSVG::axis_t>::dict.getKey(a) << '_' << EnumDict<AlignAdapterSVG::Coord>::dict.getKey(v) << '_' << (int)v << '\n';
			}

			alignment |= (((bitvect_t)pos)<<bitShift);
		}
	}

	// std::string s = sstr.str();
	alignStr = sstr.str();
	 *
	 */

}



/*
void AlignAdapterSVG::updateAlignStr(){

	std::stringstream sstr;
	char sep=0;

	for (AlignSVG::Owner p: {AlignSVG::OBJ, AlignSVG::REF}){
		for (LayoutSVG::Axis a: {LayoutSVG::Axis::HORZ, LayoutSVG::Axis::VERT}){

			const AlignAdapterSVG::Coord & v = getAlign(p, a);
			if (v != AlignSVG::UNDEFINED){
				if (sep)
					sstr << sep;
				else
					sep=' ';
				sstr << EnumDict<AlignSVG::Owner>::dict.getKey(p) << ':' << EnumDict<LayoutSVG::Axis>::dict.getKey(a) << '-' << EnumDict<AlignSVG::Coord>::dict.getKey(v);
				//std::cerr << __FUNCTION__ << ':' << EnumDict<AlignAdapterSVG::Owner>::dict.getKey(p) << '_' << EnumDict<AlignAdapterSVG::axis_t>::dict.getKey(a) << '_' << EnumDict<AlignAdapterSVG::Coord>::dict.getKey(v) << '_' << (int)v << '\n';
			}
		}
	}
	// std::string s = sstr.str();
	align = sstr.str();
}
*/



}  // image::

}  // drain::


