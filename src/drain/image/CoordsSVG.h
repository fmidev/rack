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

#ifndef DRAIN_COORDS_SVG
#define DRAIN_COORDS_SVG

#include <string>

// #include "drain/Enum.h"
// #include "drain/util/XML.h"
// #include "drain/util/Frame.h"
#include "Alignment.h"
#include "TreeSVG.h"
#include "AlignSVG.h"


namespace drain {

namespace image {


/// SVG classes marking requests of relative horizontal and vertical alignment.
/**
 *
 */
class CoordSpanBase {

public:

	// Starting coordinate (x or y).
	svg::coord_t pos = 0;  // BBoxSVG::undefined;

	// Width or height
	svg::coord_t span = 0; // BBoxSVG::undefined;

	inline
	CoordSpanBase(svg::coord_t pos = 0, svg::coord_t span = 0) : pos(pos), span(span) {
	}

	inline
	CoordSpanBase(const CoordSpanBase & cspan) : pos(cspan.pos), span(cspan.span) {
	}

	inline
	bool isDefined() const {
		return ! (std::isnan(pos) || std::isnan(span));
	}

	bool getPosition(AlignBase::Pos alignLoc, svg::coord_t & coord) const;

};


// template <>
// const drain::Enum<AlignSVG_FOO>::dict_t  drain::Enum<AlignSVG_FOO>::dict;

template <AlignBase::Axis AX>
struct CoordSpan : public CoordSpanBase {

	inline
	CoordSpan(svg::coord_t pos = 0, svg::coord_t span = 0) : CoordSpanBase(pos, span) {
	}

	inline
	CoordSpan(const CoordSpan & cspan) : CoordSpanBase(cspan.pos, cspan.span) {
	}


	// ? void getTranslatedCoordSpan(const BBoxSVG & bbox);
	void copyFrom(const NodeSVG & node);

	void copyFrom(const BBoxSVG & bbox);

};



template <AlignBase::Axis AX>
std::ostream & operator<<(std::ostream & ostr, const CoordSpan<AX> &span){
	ostr << "span" << AX << "=" << span.pos << "(+" << span.span <<  ')';
	return ostr;
}

} // image::

}  // drain::

// template <drain::image::AlignBase::Axis AX>
// std::ostream & operator<<(std::ostream & ostr, const drain::image::CoordSpan<AX> &span);



#endif // DRAIN_COORDS_SVG

