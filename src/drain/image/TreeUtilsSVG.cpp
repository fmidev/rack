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

//PanelConfSVG::OrientationFlagger TreeUtilsSVG::defaultOrientation(PanelConfSVG::HORZ);

template <>
const drain::EnumDict<PanelConfSVG::Orientation>::dict_t  drain::EnumDict<PanelConfSVG::Orientation>::dict = {
		{"HORZ", drain::image::PanelConfSVG::HORZ},
		{"VERT", drain::image::PanelConfSVG::VERT}
};

// PanelConfSVG::DirectionFlagger   TreeUtilsSVG::irection(PanelConfSVG::INCR);

template <>
const drain::EnumDict<PanelConfSVG::Direction>::dict_t  drain::EnumDict<PanelConfSVG::Direction>::dict = {
		{"INCR", drain::image::PanelConfSVG::INCR},
		{"DECR", drain::image::PanelConfSVG::DECR}
};

template <>
const drain::EnumDict<PanelConfSVG::Legend>::dict_t  drain::EnumDict<PanelConfSVG::Legend>::dict = {
		{"NO", drain::image::PanelConfSVG::NO_LEGEND},
		{"LEFT", drain::image::PanelConfSVG::LEFT},
		{"RIGHT", drain::image::PanelConfSVG::RIGHT},
		{"DUPLEX", drain::image::PanelConfSVG::LEFT | drain::image::PanelConfSVG::RIGHT},
		//{"EMBED", drain::image::PanelConfSVG::EMBED},
};


//PanelConfSVG::LegendFlagger PanelConfSVG::legend(drain::image::PanelConfSVG::RIGHT); // (PanelConfSVG::Legend::LEFT, PanelConfSVG::Legend::EMBED);

/*
PanelConfSVG::PanelConfSVG() : legend(drain::image::PanelConfSVG::RIGHT){
};
*/


//std::string TreeUtilsSVG::defaultGroupName("main");

//std::string TreeUtilsSVG::defaultTitle("");

PanelConfSVG TreeUtilsSVG::defaultConf;


// drain::Rectangle<int> & bbox
void TreeUtilsSVG::determineBBox(TreeSVG & group, drain::Frame2D<int> & frame, PanelConfSVG::Orientation orientation){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (orientation == PanelConfSVG::UNDEFINED_ORIENTATION){
		orientation = defaultConf.orientation;
		if (orientation == PanelConfSVG::UNDEFINED_ORIENTATION){
			orientation = PanelConfSVG::HORZ;
		}
	}

	// Loop 1: detect collective width and height
	frame.set(0, 0);
	//frame.height = 0;
	for (const auto & entry : group.getChildren()){ //
		const drain::image::TreeSVG & elem = entry.second;
		if ((elem->getType() == svg::IMAGE) || (elem->getType() == svg::RECT)){
			if (!elem->hasClass("FLOAT")){
				if (orientation == PanelConfSVG::HORZ){
					frame.width  += elem->get("width");
					frame.height  = std::max(frame.height, elem->get("height", 0));
					mout.warn("updated frame HORZ: ", frame.tuple());
				}
				else {
					frame.width   = std::max(frame.width, elem->get("width", 0));
					frame.height += elem->get("height");
					mout.warn("updated frame VERT: ", frame.tuple());
				}
			}
		}
		// recursion?
	}

	group->set("frame", frame.tuple());

	// bbox.set(0, 0, width, height);
}

// drain::Rectangle<int> & bbox
void TreeUtilsSVG::align(TreeSVG & group, const drain::Frame2D<int> & frame, const drain::Point2D<int> & start, PanelConfSVG::Orientation orientation, PanelConfSVG::Direction direction){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("aligning elems of group ", group->get("name", "?"));

	if (orientation == PanelConfSVG::UNDEFINED_ORIENTATION){
		orientation = defaultConf.orientation;
		if (orientation == PanelConfSVG::UNDEFINED_ORIENTATION){
			orientation = PanelConfSVG::HORZ;
		}
	}

	if (direction == PanelConfSVG::UNDEFINED_DIRECTION){
		direction = defaultConf.direction;
		if (direction == PanelConfSVG::UNDEFINED_DIRECTION){
			direction = PanelConfSVG::INCR;
		}
	}

	// Loop: stack horizontally or vertically.
	Point2D<int> pos(0,0);

	if (direction==PanelConfSVG::DECR){

		if (orientation==PanelConfSVG::HORZ){
			pos.x = frame.width;
		}
		else if (orientation==PanelConfSVG::VERT){
			pos.y = frame.height;
		}
	}
	/* readability?
	pos.x = ((orientation==PanelConfSVG::VERT) || (direction==PanelConfSVG::INCR)) ? 0 : frame.width;
	pos.y = ((orientation==PanelConfSVG::HORZ) || (direction==PanelConfSVG::INCR)) ? 0 : frame.height;
	*/

	pos.x += start.x;
	pos.y += start.y;

	Point2D<int> offset;
	// int widthPrev = 0;
	// int offset.y = 0;

	for (auto & entry : group.getChildren()){ //

		TreeSVG & elem = entry.second;
		NodeSVG::elem_t t = elem->getType();

		if (elem->hasClass("FIXED"))
				continue;

		// elem->addClass(FlagResolver::getKeys(drain::EnumDict<PanelConfSVG::Orientation>::dict, orientation, ' '));
		//elem->addClass(PanelConfSVG::OrientationFlagger::getValueNEW("mika"));
		elem->addClass(PanelConfSVG::OrientationFlagger::getKeysNEW2(orientation, ' '));

		if ((t == svg::IMAGE) || (t == svg::RECT) || (t == svg::TEXT)){
			// mout.attention("  elem ", elem->get("name", "?"), ": init pos", pos);

			/*
			if (elem->hasClass("header")){ // or "maxWidth"   or rect?
				elem->set("width",  frame.width);
			}
			*/

			if (!elem->hasClass("FLOAT")){

				if (direction==PanelConfSVG::INCR){
					if (orientation==PanelConfSVG::HORZ)
						pos.x += offset.x;
					else
						pos.y += offset.y;
				}

				offset.x = elem->get("width", 0);
				offset.y = elem->get("height",0);

				if (direction==PanelConfSVG::DECR){
					if (orientation==PanelConfSVG::HORZ)
						pos.x -= offset.x;
					else
						pos.y -= offset.y;
				}
			}

			if (t == svg::TEXT){
				elem->set("x", pos.x);
				elem->set("y", pos.y + 20); // fonts
			}
			else {
				elem->set("x", pos.x);
				elem->set("y", pos.y);
				elem->set("_offset", offset.tuple());
				// elem->set("_offset", offset);
			}

		}


	}


}

/**
 *
 */
/*

int  AttributeCheckerXML::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path){

	TreeSVG & current = tree(path);

	TreeSVG::node_data_t::map_t & attributes = current->getAttributes();
	for (const std::string & key: attributes.getKeyList()){
		std::string keyChecked(key);
		bool CHANGED = false;
		for (char &c: keyChecked){
			if ( ((c>='a') && (c<='z')) || ((c>='A') && (c<='Z')) || ((c>='0') && (c<='9')) || (c=='_') || (c=='-')){
				// ok
			}
			else {
				CHANGED = true;
				c = '_';
			}
		}
		if (CHANGED){
			attributes[keyChecked] = attributes[key];
			attributes.erase(key);
		}
	};

	return 0;

}
	*/



}  // image::

}  // drain::
