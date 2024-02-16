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
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#ifndef DRAIN_TREE_UTILS_SVG_H_
#define DRAIN_TREE_UTILS_SVG_H_

//#include <drain/util/Rectangle.h>

#include <drain/util/Frame.h>
#include <drain/util/Flags.h>
#include "TreeSVG.h"

namespace drain {

namespace image {

struct TreeUtilsSVG {

	enum Orientation {UNDEFINED_ORIENTATION=0, HORZ, VERT};
	typedef drain::EnumFlagger<drain::SingleFlagger<Orientation> > OrientationFlagger;
	static OrientationFlagger defaultOrientation;

	enum Direction {UNDEFINED_DIRECTION=0, INCR, DECR};
	typedef drain::EnumFlagger<drain::SingleFlagger<Direction> > DirectionFlagger;
	static DirectionFlagger defaultDirection;

	/// SVG file may contain several "modules", for example rows or columns of IMAGE:s. This is the name of the current module, contained in a GROUP.
	static std::string defaultGroupName;

	/// Computes the width and height for a bounding box covering non-overlappin IMAGE and RECT elements.
	static
	void determineBBox(TreeSVG & group, drain::Frame2D<int> & frame, Orientation orientation=UNDEFINED_ORIENTATION);

	/// Stack IMAGE and RECT elements within the frame (width x height)
	static
	void align(TreeSVG & group, drain::Frame2D<int> & frame, Orientation orientation=UNDEFINED_ORIENTATION, Direction direction=UNDEFINED_DIRECTION);



};

template <>
const drain::EnumDict<TreeUtilsSVG::Orientation>::dict_t  drain::EnumDict<TreeUtilsSVG::Orientation>::dict;

template <>
const drain::EnumDict<TreeUtilsSVG::Direction>::dict_t  drain::EnumDict<TreeUtilsSVG::Direction>::dict;


//static
//NodeXML<BaseSVG::tag_t>::tags;
/*
template <>
TreeSVG & TreeSVG::operator=(const std::initializer_list<std::pair<const char *,const drain::Variable> > & l){
	// data.set(l);
	return *this;
}
*/



}  // image::

}  // drain::




#endif // TREESVG_H_

