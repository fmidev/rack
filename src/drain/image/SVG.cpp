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

#include "SVG.h"

namespace drain {





DRAIN_ENUM_DICT(image::svg::tag_t) = {
		{"UNDEFINED", image::svg::UNDEFINED},
		{"#",         image::svg::COMMENT},
		{"CTEXT",     image::svg::CTEXT},
		{"script",    image::svg::SCRIPT},
		{"style",     image::svg::STYLE},
		{"style_select", drain::image::svg::STYLE_SELECT},
		{"javascript_scope", drain::image::svg::JAVASCRIPT_SCOPE},
		// -- Actual SVG elems
		{"svg",       image::svg::SVG},
		{"circle",    image::svg::CIRCLE},
		{"clipPath", drain::image::svg::CLIP_PATH},
		{"defs", drain::image::svg::DEFS},
		{"desc", drain::image::svg::DESC},
		{"g", drain::image::svg::GROUP},
		{"image", drain::image::svg::IMAGE},
		{"line", drain::image::svg::LINE},
		{"linearGradient", drain::image::svg::LINEAR_GRADIENT},
		{"mask", drain::image::svg::MASK},
		{"metadata", drain::image::svg::METADATA},
		{"polygon", drain::image::svg::POLYGON},
		{"path", drain::image::svg::PATH},
		{"rect", drain::image::svg::RECT},
		{"style", drain::image::svg::STYLE}, // raise?
		{"text", drain::image::svg::TEXT},
		{"title", drain::image::svg::TITLE},
		{"tspan", drain::image::svg::TSPAN},
};

namespace image {

}  // image::


}  // drain::


