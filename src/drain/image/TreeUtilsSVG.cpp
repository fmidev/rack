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


#include <drain/util/Point.h>
#include "TreeUtilsSVG.h"

namespace drain {

namespace image {


template <>
const drain::EnumDict<PanelConfSVG::Orientation>::dict_t  drain::EnumDict<PanelConfSVG::Orientation>::dict = {
		{"HORZ", drain::image::PanelConfSVG::HORZ},
		{"VERT", drain::image::PanelConfSVG::VERT}
};

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




PanelConfSVG TreeUtilsSVG::defaultConf;

bool TreeUtilsSVG::getRect(TreeSVG & group, Box<double> & rect){

	rect.setLocation(group->get("x", 0), group->get("y", 0));
	rect.setWidth(group->get("width", 0));
	rect.setHeight(group->get("height", 0));

	return (rect.getWidth() != 0.0) && (rect.getHeight() != 0.0);

}

void TreeUtilsSVG::getBoundingFrame(TreeSVG & group, Frame2D<int> & frame, PanelConfSVG::Orientation orientation){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (orientation == PanelConfSVG::UNDEFINED_ORIENTATION){
		orientation = defaultConf.orientation;
		if (orientation == PanelConfSVG::UNDEFINED_ORIENTATION){
			orientation = PanelConfSVG::HORZ;
		}
	}

	NodeSVG::path_list_t paths;
	// TODO / FIX: set of allowed tags, to get them in order of appearance.
	NodeSVG::findByTags(group, {svg::IMAGE,svg::RECT} , paths);
	//NodeSVG::findByTag(group, svg::RECT,  paths); // also SVG ? (legends)

	frame.set(0, 0);

	for (const NodeSVG::path_t  & p: paths){

		const TreeSVG & elem = group(p);

		if (!elem->hasClass("FLOAT")){
			if (orientation == PanelConfSVG::HORZ){
				frame.width  += elem->get("width");
				frame.height  = std::max(frame.height, elem->get("height", 0));
				// mout.warn("updated frame HORZ: ", frame.tuple());
			}
			else {
				frame.width   = std::max(frame.width, elem->get("width", 0));
				frame.height += elem->get("height");
				// mout.warn("updated frame VERT: ", frame.tuple());
			}
		}
	}

	/*
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
	}
	*/

	group->set("data-frame", frame.tuple());

	if (mout.isDebug()){

	}

	// bbox.set(0, 0, width, height);
}

// drain::Rectangle<int> & bbox
void TreeUtilsSVG::alignSequence(TreeSVG & group, const drain::Frame2D<int> & frame, const drain::Point2D<int> & start, PanelConfSVG::Orientation orientation, PanelConfSVG::Direction direction){

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
	// Position of the current object
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

	// Position of the current object
	Point2D<int> offset;

	NodeSVG::path_list_t paths;

	NodeSVG::findByTags(group, {svg::IMAGE,svg::RECT,svg::TEXT}, paths);

	bool first=true;
	for (const NodeSVG::path_t  & p: paths){

		TreeSVG & elem = group(p);

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

			if (first && elem->hasClass("EXPERIMENTAL-MARGINAL")){

				// TODO: see above; for clarity all pos.itions could be inited here
				if (direction==PanelConfSVG::INCR){
					if (orientation==PanelConfSVG::HORZ){
						pos.x -= elem->get("width", 0);
					}
					else if (orientation==PanelConfSVG::VERT){
						pos.y -= elem->get("height", 0);
					}
				}
				/*
				if (direction==PanelConfSVG::DECR){
					if (orientation==PanelConfSVG::HORZ){
						pos.x = frame.width;
					}
					else if (orientation==PanelConfSVG::VERT){
						pos.y = frame.height;
					}
				}
				*/
				first = false;
			}


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
				if (mout.isDebug()){
					elem->set("_offset", offset.tuple());
				}
			}

		}

	}


}

void TreeUtilsSVG::alignText(TreeSVG & group){

	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::image::NodeSVG::path_list_t pathList;

	//mout.note("koe: ", group->getTag(), " name: ", group->get("name", "?"));

	drain::image::NodeSVG::findByClass(group, "FLOAT", pathList);

	const drain::Point2D<double> textOffset(5.0, 5.0);

	for (const drain::image::NodeSVG::path_t & path: pathList){

		TreeSVG & elem =  group(path);
		if (elem -> typeIs(NodeSVG::TEXT)){

			// mout.note("realign: ", path);

			const std::string ref = elem->get("ref", "");
			if (ref.empty()){
				mout.warn("'ref' attribute missing for TEXT.FLOAT elem ", elem->get("name",""), " at ", path);
			}

			drain::image::NodeSVG::path_t p;

			if (NodeSVG::findById(group, ref, p)){

				// mout.note("for TEXT.FLOAT(", path, ") found IMAGE/RECT '", ref, "'");

				TreeSVG & frame = group(p);
				Box<double> box;

				if (TreeUtilsSVG::getRect(frame, box)){

					// start | middle | end

					// Book keeping by parent element for rows.
					std::string locationLabel = "titles";

					if (elem->hasClass("MIDDLE")){
						locationLabel += "_M";
					}
					else if (elem->hasClass("BOTTOM")){
						locationLabel += "_B";
					}
					else { // Default: elem->hasClass("TOP")
						locationLabel += "_T";
					}

					if (elem->hasClass("CENTER")){
						elem->set("x", box.x + 0.5*box.width);
						elem->setStyle("text-anchor", "middle");
						locationLabel += "_C";
					}
					else if (elem->hasClass("RIGHT")){
						elem->set("x",  box.x + box.width - textOffset.x);
						elem->setStyle("text-anchor", "end");
						locationLabel += "_R";
					}
					else { // Default: elem->hasClass("LEFT")
						elem->set("x", box.x + textOffset.x);
						elem->setStyle("text-anchor", "start");
						locationLabel += "_L";
					}

					const int index = frame->get(locationLabel, 0);
					const int fontSize = elem->style.get("font-size", 30); // what about "30em" ?

					if (elem->hasClass("MIDDLE")){
						elem->set("y", box.y + 0.75*box.height + textOffset.y + fontSize*index); // FIX: should be SUM of invidual row widths
					}
					else if (elem->hasClass("BOTTOM")){
						elem->set("y", box.y + box.height - textOffset.y - fontSize*index); // FIX: should be SUM of invidual row widths
					}
					else { // Default: elem->hasClass("TOP")
						elem->set("y", box.y + textOffset.y + fontSize*(index+1)); // FIX: should be SUM of invidual row widths
					}

					/// Book keeping
					frame->set(locationLabel, index+1);

				}
				else {
					mout.warn("width and/or height missing for referred IMAGE/RECT '", ref, "' at path=", p);
				}

			}
			else {
				mout.warn("could not find referred IMAGE/RECT object with id='", ref, "', required by elem '", elem->get("name","?"), "' at ", path);
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
