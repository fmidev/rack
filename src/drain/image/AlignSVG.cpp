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
const EnumDict<AlignSVG2::axis_t>::dict_t EnumDict<AlignSVG2::axis_t>::dict = {
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG2::axis_t, HORZ),
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG2::axis_t, VERT),
};

template<>
const EnumDict<AlignSVG2::pos_t>::dict_t EnumDict<AlignSVG2::pos_t>::dict = {
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG2::pos_t, OBJ),
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG2::pos_t, REF),
};

template<>
const EnumDict<AlignSVG2::value_t>::dict_t EnumDict<AlignSVG2::value_t>::dict = {
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG2::value_t, UNDEFINED),
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG2::value_t, MAX),
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG2::value_t, MID),
	DRAIN_ENUM_ENTRY(drain::image::AlignSVG2::value_t, MIN),
	// {drain::image::svg::MAX, "MAX"}
};



void AlignConfSVG::clearAlign(){
	for (align_vect_t & a: alignments){
		for (value_t & v: a){
			v = UNDEFINED;
		}
	}
};

bool AlignConfSVG::isAligned() const {
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


void AlignSVG2::updateAlignStr(){
	std::stringstream sstr;
	char sep=0;
	for (AlignSVG2::pos_t p: {AlignSVG2::OBJ, AlignSVG2::REF}){
		for (AlignSVG2::axis_t a: {AlignSVG2::HORZ, AlignSVG2::VERT}){
			const AlignSVG2::value_t & v = getAlign(p, a);
			if (v != AlignSVG2::UNDEFINED){
				if (sep)
					sstr << sep;
				else
					sep=' ';
				sstr << EnumDict<AlignSVG2::pos_t>::dict.getKey(p) << ':' << EnumDict<AlignSVG2::axis_t>::dict.getKey(a) << '-' << EnumDict<AlignSVG2::value_t>::dict.getKey(v);
				//std::cerr << __FUNCTION__ << ':' << EnumDict<AlignSVG2::pos_t>::dict.getKey(p) << '_' << EnumDict<AlignSVG2::axis_t>::dict.getKey(a) << '_' << EnumDict<AlignSVG2::value_t>::dict.getKey(v) << '_' << (int)v << '\n';
			}
		}
	}
	// std::string s = sstr.str();
	align = sstr.str();
}


template <>
const drain::EnumDict<LayoutSVG::Orientation>::dict_t  drain::EnumDict<LayoutSVG::Orientation>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Orientation, HORZ),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Orientation, VERT),
};

template <>
const drain::EnumDict<LayoutSVG::Direction>::dict_t  drain::EnumDict<LayoutSVG::Direction>::dict = {
		// {"INCR", drain::image::PanelConfSVG::INCR},
		// {"DECR", drain::image::PanelConfSVG::DECR}
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Direction, INCR),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Direction, DECR)
};


/*
std::ostream & NodeSVG::toStream(std::ostream &ostr, const TreeSVG & tree){
	NodeXML::toStream(ostr, tree);
	return ostr;
}
*/

/*
static
void LayoutSVG::getAlignmentConf(LayoutSVG::Direction d, LayoutSVG::Orientation v, AlignConfSVG & alignConf){
	alignConf.setAlignOutside(d==LayoutSVG::HORZ ? AlignConfSVG::HORZ : AlignConfSVG::VERT, v==LayoutSVG::INCR ? AlignConfSVG::MAX : AlignConfSVG::MIN);
}
*/

}  // image::

}  // drain::


