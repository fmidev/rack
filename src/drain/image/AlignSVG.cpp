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


template<>
const EnumDict<LayoutSVG::GroupType>::dict_t EnumDict<LayoutSVG::GroupType>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, HEADER),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, ALIGN_SCOPE),
		//DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, ALIGNED),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, FLOAT),
};

template <>
const drain::EnumDict<LayoutSVG::Axis>::dict_t  drain::EnumDict<LayoutSVG::Axis>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Axis, HORZ),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Axis, VERT),
};

template <>
const drain::EnumDict<LayoutSVG::Direction>::dict_t  drain::EnumDict<LayoutSVG::Direction>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Direction, INCR),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Direction, DECR)
};


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

template<>
const EnumDict<AlignSVG::Owner>::dict_t EnumDict<AlignSVG::Owner>::dict = {
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG::Owner, OBJ),
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG::Owner, REF),
};

template<>
const EnumDict<AlignSVG::value_t>::dict_t EnumDict<AlignSVG::value_t>::dict = {
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG::value_t, UNDEFINED),
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG::value_t, MAX),
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG::value_t, MID),
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG::value_t, MIN),
	// {drain::image::svg::MAX, "MAX"}
};


void AlignSVG::setAlign(const AlignSVG & conf){
	for (AlignSVG::Owner p: {AlignSVG::OBJ, AlignSVG::REF}){
		for (LayoutSVG::Axis a: {LayoutSVG::Axis::HORZ, LayoutSVG::Axis::VERT}){
			setAlign(p, a, conf.getAlign(p, a));
		}
	}
}


void AlignSVG::clearAlign(){
	for (align_vect_t & a: alignments){
		for (value_t & v: a){
			v = UNDEFINED;
		}
	}
};

bool AlignSVG::isAligned() const {
	// Could be also decided indirectly: if (!alignStr.empty())
	for (const align_vect_t & a: alignments){
		for (const value_t & v: a){
			if (v != UNDEFINED){
				return true;
			}
		}
	}
	return false;
};



void AlignAdapterSVG::updateAlignStr(){

	std::stringstream sstr;
	char sep=0;

	for (AlignSVG::Owner p: {AlignSVG::OBJ, AlignSVG::REF}){
		for (LayoutSVG::Axis a: {LayoutSVG::Axis::HORZ, LayoutSVG::Axis::VERT}){

			const AlignAdapterSVG::value_t & v = getAlign(p, a);
			if (v != AlignSVG::UNDEFINED){
				if (sep)
					sstr << sep;
				else
					sep=' ';
				sstr << EnumDict<AlignSVG::Owner>::dict.getKey(p) << ':' << EnumDict<LayoutSVG::Axis>::dict.getKey(a) << '-' << EnumDict<AlignSVG::value_t>::dict.getKey(v);
				//std::cerr << __FUNCTION__ << ':' << EnumDict<AlignAdapterSVG::Owner>::dict.getKey(p) << '_' << EnumDict<AlignAdapterSVG::axis_t>::dict.getKey(a) << '_' << EnumDict<AlignAdapterSVG::value_t>::dict.getKey(v) << '_' << (int)v << '\n';
			}
		}
	}
	// std::string s = sstr.str();
	align = sstr.str();
}




}  // image::

}  // drain::


