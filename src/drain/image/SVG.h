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

#ifndef DRAIN_SVG
#define DRAIN_SVG

#include <string>

//#include "drain/Enum.h"
#include "drain/util/XML.h"
#include "drain/util/Frame.h"
// #include "AlignSVG.h"


namespace drain {

namespace image {

struct svg {

	typedef float coord_t;

	enum tag_t {
		UNDEFINED=XML::UNDEFINED,
		COMMENT=XML::COMMENT,
		CTEXT=XML::CTEXT,
		SCRIPT=XML::SCRIPT,
		STYLE=XML::STYLE,
		STYLE_SELECT=XML::STYLE_SELECT,
		JAVASCRIPT_SCOPE=XML::JAVASCRIPT_SCOPE,
		SVG=10,
		CIRCLE, CLIP_PATH, DEFS, DESC, GROUP, IMAGE, LINE, LINEAR_GRADIENT, MASK, METADATA, PATH, POLYGON, RECT, TEXT, TITLE, TSPAN };
	// check CTEXT, maybe implement in XML

};

/*
class NodeSVG;

// typedef drain::UnorderedMultiTree<NodeSVG,false, NodeXML<>::path_t> TreeSVG;
typedef DRAIN_XML_TREE(NodeSVG) TreeSVG;

typedef SelectXML<svg::tag_t> SelectSVG;
*/

} // image::







namespace image {


class BBoxSVG : public drain::Box<svg::coord_t> {

public:

	inline
	BBoxSVG(svg::coord_t x=0, svg::coord_t y=0, svg::coord_t width=0, svg::coord_t height=0) : drain::Box<svg::coord_t>(x, y, width, height)  {
	}

	inline
	BBoxSVG(const BBoxSVG & bbox) : drain::Box<svg::coord_t>(bbox)  {
	}


	// New



	// Future option - also other units!
	/*
	bool x_PERCENTAGE = false;
	bool y_PERCENTAGE = false;
	bool width_PERCENTAGE = false;
	bool height_PERCENTAGE = false;
	*/
};





}  // image::

// template <>
// const Enum<image::svg::tag_t>::dict_t Enum<image::svg::tag_t>::dict;
DRAIN_ENUM_DICT(image::svg::tag_t);

DRAIN_ENUM_OSTREAM(image::svg::tag_t)


}  // drain::



#endif // DRAIN_COORDS_SVG

