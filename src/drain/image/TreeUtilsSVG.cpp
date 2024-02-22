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

#include <drain/util/Point.h>
#include "TreeUtilsSVG.h"

namespace drain {

namespace image {

TreeUtilsSVG::OrientationFlagger TreeUtilsSVG::defaultOrientation(TreeUtilsSVG::HORZ);

template <>
const drain::EnumDict<TreeUtilsSVG::Orientation>::dict_t  drain::EnumDict<TreeUtilsSVG::Orientation>::dict = {
		{"HORZ", drain::image::TreeUtilsSVG::HORZ},
		{"VERT", drain::image::TreeUtilsSVG::VERT}
};

TreeUtilsSVG::DirectionFlagger   TreeUtilsSVG::defaultDirection(TreeUtilsSVG::INCR);

template <>
const drain::EnumDict<TreeUtilsSVG::Direction>::dict_t  drain::EnumDict<TreeUtilsSVG::Direction>::dict = {
		{"INCR", drain::image::TreeUtilsSVG::INCR},
		{"DECR", drain::image::TreeUtilsSVG::DECR}
};

std::string TreeUtilsSVG::defaultGroupName("main");


// drain::Rectangle<int> & bbox
void TreeUtilsSVG::determineBBox(TreeSVG & group, drain::Frame2D<int> & frame, Orientation orientation){

	if (orientation == UNDEFINED_ORIENTATION){
		orientation = TreeUtilsSVG::defaultOrientation;
	}

	// Loop 1: detect collective width and height
	frame.set(0, 0);
	//frame.height = 0;
	for (const auto & entry : group.getChildren()){ //
		const drain::image::TreeSVG & elem = entry.second;
		if ((elem->getType() == svg::IMAGE) || (elem->getType() == svg::RECT)){
			if (!elem->hasClass("FLOAT")){
				if (orientation == HORZ){
					frame.width  += elem->get("width");
					frame.height  = std::max(frame.height, elem->get("height", 0));
				}
				else {
					frame.width = std::max(frame.width, elem->get("width", 0));
					frame.height += elem->get("height");
				}
			}
		}
		// recursion?
	}

	// bbox.set(0, 0, width, height);
}

// drain::Rectangle<int> & bbox
void TreeUtilsSVG::align(TreeSVG & group, const drain::Frame2D<int> & frame, const drain::Point2D<int> & start, Orientation orientation, Direction direction){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.attention("aligning elems of ", group->get("name", "?"));

	if (orientation == UNDEFINED_ORIENTATION){
		orientation = TreeUtilsSVG::defaultOrientation;
	}

	if (direction == UNDEFINED_DIRECTION){
		direction = TreeUtilsSVG::defaultDirection;
	}

	// Loop: stack horizontally or vertically.
	Point2D<int> pos;
	pos.x = ((orientation==VERT) || (direction==INCR)) ? 0 : frame.width;
	pos.y = ((orientation==HORZ) || (direction==INCR)) ? 0 : frame.height;

	pos.x += start.x;
	pos.y += start.y;

	int w = 0;
	int h = 0;

	for (auto & entry : group.getChildren()){ //

		TreeSVG & elem = entry.second;
		NodeSVG::elem_t t = elem->getType();

		if (elem->hasClass("FIXED"))
			continue;

		if ((t == svg::IMAGE) || (t == svg::RECT) || (t == svg::TEXT)){
			// mout.attention("  elem ", elem->get("name", "?"), ": init pos", pos);


			if (!elem->hasClass("FLOAT")){
				if (direction==INCR){
					if (orientation==HORZ)
						pos.x += w;
					else
						pos.y += h;
				}

				w = elem->get("width", 0);
				h = elem->get("height",0);

				if (direction==DECR){
					if (orientation==HORZ)
						pos.x -= w;
					else
						pos.y -= h;
				}
			}

			if (t == svg::TEXT){
				elem->set("x", pos.x);
				elem->set("y", pos.y + 20); // fonts
			}
			else {
				elem->set("x", pos.x);
				elem->set("y", pos.y);
			}


		}
	}


}


}  // image::

}  // drain::
