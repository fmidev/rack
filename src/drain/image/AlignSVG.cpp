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


const AlignSVG::HorzAlign AlignSVG::RIGHT(AlignBase::Pos::MAX);
const AlignSVG::HorzAlign AlignSVG::CENTER(AlignBase::Pos::MID);
const AlignSVG::HorzAlign AlignSVG::LEFT(AlignBase::Pos::MIN);
const AlignSVG::HorzAlign AlignSVG::HORZ_FILL(AlignBase::Pos::FILL);
const AlignSVG::HorzAlign AlignSVG::UNDEFINED_HORZ(AlignBase::Pos::UNDEFINED_POS);

template <>
const drain::EnumDict<AlignSVG::HorzAlign>::dict_t  drain::EnumDict<AlignSVG::HorzAlign>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, LEFT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, CENTER),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, RIGHT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, HORZ_FILL),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, UNDEFINED_HORZ),
};

const AlignSVG::VertAlign AlignSVG::BOTTOM(AlignBase::Pos::MAX);
const AlignSVG::VertAlign AlignSVG::MIDDLE(AlignBase::Pos::MID);
const AlignSVG::VertAlign AlignSVG::TOP(AlignBase::Pos::MIN);
const AlignSVG::VertAlign AlignSVG::VERT_FILL(AlignBase::Pos::FILL);
const AlignSVG::VertAlign AlignSVG::UNDEFINED_VERT(AlignBase::Pos::UNDEFINED_POS);

template <>
const drain::EnumDict<AlignSVG::VertAlign>::dict_t  drain::EnumDict<AlignSVG::VertAlign>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, TOP),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, MIDDLE),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, BOTTOM),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, VERT_FILL),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, UNDEFINED_VERT),
};

DRAIN_TYPENAME_DEF(AlignSVG::HorzAlign);
DRAIN_TYPENAME_DEF(AlignSVG::VertAlign);



template <>
const drain::EnumDict<Alignment<> >::dict_t  drain::EnumDict<Alignment<> >::dict = {
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, LEFT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, CENTER),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, RIGHT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, HORZ_FILL),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, UNDEFINED_HORZ),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, TOP),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, MIDDLE),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, BOTTOM),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, VERT_FILL),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, UNDEFINED_VERT),
};


// Alignment<> test(drain::image::AlignSVG::LEFT);
// Alignment<> test2 = drain::image::AlignSVG::LEFT;

/// Handler for command line or configuration file arguments
void AlignSVG::setAlign(const std::string & align){

	drain::Logger mout(__FUNCTION__, __FILE__);

	std::list<std::string> args;
	drain::StringTools::split(align, args, ','); // todo: separators?


	for (const std::string & arg: args){

		mout.special("Now: ", arg);

		std::vector<std::string> conf;
		drain::StringTools::split(arg, conf, ':');

		switch (conf.size()){
		case 3:
			mout.attention("Setting: ", conf[0], conf[1], conf[2]);
			setAlign(conf[0], conf[1], conf[2]);
			break;
		case 2:
			mout.attention("Setting: ", conf[0], conf[1]);
			setAlign(conf[0], conf[1]);
			break;
		// case 1:
			//mout.attention("Setting: ", conf[0]);
			//setAlign(EnumDict<Alignment<> >::getValue(conf[0], false).pos);
			//setAlign(conf[0], conf[1]);
			//break;
		default:
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

	for (AlignBase::Axis axis: {AlignBase::Axis::HORZ, AlignBase::Axis::VERT}){

		bitShift += (axis == AlignBase::Axis::HORZ) ? 0 : 2;

		//sep=0;
		//sep=' ';

		for (AlignSVG::Owner owner: {Owner::ANCHOR, Owner::OBJECT}){

			bitShift = (owner == Owner::OBJECT) ? 0 : 4;

			const AlignBase::Pos & pos = getAlign(owner, axis);

			if (pos != AlignBase::UNDEFINED_POS){

				if (sep){
					ostr << sep;
					//sep = ' ';
				}
				else {
					//sep='.';
				}
				sep='.';

				if (owner == Owner::ANCHOR){
					// ostr << '@';
				}
				else {
					//ostr << '.';
				}

				const std::string & key = drain::EnumDict<Alignment<> >::dict.getKey(Alignment<>(axis, pos), true);
				if (!key.empty()){
					ostr << key;
				}
				else {
					ostr << axis << '-' << pos;
				}

				//std::cerr << __FUNCTION__ << ':' << EnumDict<AlignAdapterSVG::Owner>::dict.getKey(p) << '_' << EnumDict<AlignAdapterSVG::axis_t>::dict.getKey(a) << '_' << EnumDict<AlignAdapterSVG::Coord>::dict.getKey(v) << '_' << (int)v << '\n';
			}

			alignment |= (((bitvect_t)pos)<<bitShift);
		}

		sep=' ';
	}


}


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

}


}  // image::

}  // drain::


