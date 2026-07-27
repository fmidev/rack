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
 *      Author: mpeura
 */

#include <drain/Log.h>
#include <drain/StringTools.h>
#include <drain/image/AlignAnchorSVG.h>

namespace drain {


namespace image {





/** Future option: re-map to normalized coordinates: MIN=0.0, MID=0.5, MAX=1.0
 *  FILL must be left as a flag.
 *
 */
DRAIN_ENUM_DICT(AlignBase::Pos) = {
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Pos, UNDEFINED_POS),
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Pos, MAX),
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Pos, MID),
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Pos, MIN),
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Pos, FILL),
};

DRAIN_ENUM_DICT(AlignBase::Axis) = {
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Axis, HORZ),
		DRAIN_ENUM_ENTRY(drain::image::AlignBase::Axis, VERT),
};

// -----------------------------------------------------------------------------------------------------------

DRAIN_ENUM_DICT(image::MutualAlign::Topol) = {
		DRAIN_ENUM_ENTRY(image::MutualAlign::Topol, INSIDE),
		DRAIN_ENUM_ENTRY(image::MutualAlign::Topol, OUTSIDE),
};


/*
const AlignSVG::HorzAlign AlignSVG::RIGHT(AlignBase::Pos::MAX);
const AlignSVG::HorzAlign AlignSVG::CENTER(AlignBase::Pos::MID);
const AlignSVG::HorzAlign AlignSVG::LEFT(AlignBase::Pos::MIN);
const AlignSVG::HorzAlign AlignSVG::HORZ_FILL(AlignBase::Pos::FILL);
const AlignSVG::HorzAlign AlignSVG::UNDEFINED_HORZ(AlignBase::Pos::UNDEFINED_POS);


const AlignSVG::VertAlign AlignSVG::BOTTOM(AlignBase::Pos::MAX);
const AlignSVG::VertAlign AlignSVG::MIDDLE(AlignBase::Pos::MID);
const AlignSVG::VertAlign AlignSVG::TOP(AlignBase::Pos::MIN);
const AlignSVG::VertAlign AlignSVG::VERT_FILL(AlignBase::Pos::FILL);
const AlignSVG::VertAlign AlignSVG::UNDEFINED_VERT(AlignBase::Pos::UNDEFINED_POS);
*/



}  // image::



// Alignment<> test(drain::image::AlignSVG::LEFT);
// Alignment<> test2 = drain::image::AlignSVG::LEFT;



}  // drain::


