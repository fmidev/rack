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

#include "LayoutSVG.h"

#include "Direction.h"

namespace drain {

DRAIN_ENUM_DICT(image::LayoutSVG::GroupType) = {
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, STACK_LAYOUT),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, ADAPTER),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, COMPOUND),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, FIXED),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, INDEPENDENT),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, NEUTRAL),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, CROP),
		// DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, HEADER),
		// DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::GroupType, ALIGN),
};

DRAIN_ENUM_DICT(image::LayoutSVG::Direction) = {
		DRAIN_ENUM_ENTRY(image::LayoutSVG::Direction, INCR),
		DRAIN_ENUM_ENTRY(image::LayoutSVG::Direction, DECR),
};

DRAIN_ENUM_DICT(image::LayoutSVG::DirectionHorz) = {
		DRAIN_ENUM_ENTRY(image::LayoutSVG::DirectionHorz, RIGHT),
		DRAIN_ENUM_ENTRY(image::LayoutSVG::DirectionHorz, LEFT),
};

DRAIN_ENUM_DICT(image::LayoutSVG::DirectionVert) = {
		DRAIN_ENUM_ENTRY(image::LayoutSVG::DirectionVert, DOWN),
		DRAIN_ENUM_ENTRY(image::LayoutSVG::DirectionVert, UP),

};


}  // drain::


