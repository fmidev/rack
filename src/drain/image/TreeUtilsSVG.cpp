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


#include <drain/util/Output.h> // Debugging
#include <drain/util/Point.h>
#include "TreeUtilsSVG.h"

namespace drain {

namespace image {

/*
#define ENUM_PAIR(NSPACE, ARG) {#ARG, NSPACE::ARG}
#define APPLY_ENUM_ENTRY(NSPACE, FIRST, ...)  ENUM_PAIR(NSPACE, FIRST)
#define IS_EMPTY(...) _IS_EMPTY_IMPL(__VA_ARGS__, 1, 0)
#define _IS_EMPTY_IMPL(_1, _2, N, ...) N
*/

/*
template <>
const drain::EnumDict<LayoutSVG::Orientation>::dict_t  drain::EnumDict<LayoutSVG::Orientation>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Orientation, HORZ),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Orientation, VERT),
};

template <>
const drain::EnumDict<LayoutSVG::Direction>::dict_t  drain::EnumDict<LayoutSVG::Direction>::dict = {
		// {"INCR", drain::image::LayoutSVG::INCR},
		// {"DECR", drain::image::LayoutSVG::DECR}
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Direction, INCR),
		DRAIN_ENUM_ENTRY(drain::image::LayoutSVG::Direction, DECR)
};
*/

template <>
const drain::EnumDict<PanelConfSVG::Legend>::dict_t  drain::EnumDict<PanelConfSVG::Legend>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Legend, NO_LEGEND),
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Legend, LEFT),
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Legend, RIGHT),
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Legend, DUPLEX),
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Legend, EMBED)
		// {"NO", drain::image::PanelConfSVG::NO_LEGEND},
		// {"LEFT", drain::image::PanelConfSVG::LEFT},
		// {"RIGHT", drain::image::PanelConfSVG::RIGHT},
		// {"DUPLEX", drain::image::PanelConfSVG::LEFT | drain::image::PanelConfSVG::RIGHT},
		// {"EMBED", drain::image::PanelConfSVG::EMBED},
};




PanelConfSVG TreeUtilsSVG::defaultConf;

/*
bool TreeUtilsSVG::getElementBounds(const TreeSVG & elem, Box<double> & rect){

	rect.setLocation(elem->get("x", 0), elem->get("y", 0));
	rect.setWidth(elem->get("width", 0));
	rect.setHeight(elem->get("height", 0));

	return (rect.getWidth() != 0.0) && (rect.getHeight() != 0.0);

}
*/

bool TreeUtilsSVG::computeBoundingBox(const TreeSVG & elem, Box<NodeSVG::coord_t> & box){

	if (elem->typeIs(svg::SVG) || elem->typeIs(svg::GROUP)){
		for (const TreeSVG::pair_t & entry: elem){
			computeBoundingBox(entry.second, box); // recursion
		}
	}
	else {
		if (box.empty()){
			box = elem->getBoundingBox();
		}
		else {
			// drain::Logger mout(__FILE__, __FUNCTION__);
			//mout.pending("Updating bbox ", box, " with ", elem->getBoundingBox());
			box.expand(elem->getBoundingBox());
		}
	}

	return (box.getWidth() != 0.0) && (box.getHeight() != 0.0);

}

/** Stacks all the non-FLOAT elements horizontally or vertically, computing their collective width or height.
 *
 */
void TreeUtilsSVG::getBoundingFrame(const TreeSVG & group, Frame2D<int> & frame, Align::Axis orientation){

	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.unimplemented(__FUNCTION__);

	/*
	if (orientation == LayoutSVG::UNDEFINED_ORIENTATION){
		orientation = defaultConf.layout.orientation;
		if (orientation == LayoutSVG::UNDEFINED_ORIENTATION){
			orientation = LayoutSVG::HORZ;
		}
	}

	NodeSVG::path_list_t paths;
	// TODO / DONE(?): set of allowed tags, to get them in order of appearance.
	NodeSVG::findByTags(group, {svg::IMAGE,svg::RECT} , paths);
	// Consider also svg::SVG ? (eg. legends)
	// Consider also svg::TXT (for x, y)

	frame.set(0, 0);

	for (const NodeSVG::path_t  & p: paths){

		const TreeSVG & elem = group(p);

		if (!elem->hasClass(Align::FLOAT)){
			if (orientation == LayoutSVG::HORZ){
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
	*/

	// if (mout.isDebug()){}
	// group->set("data-frame", frame.tuple());
	// bbox.set(0, 0, width, height);
}





//
/**  Retrieves and traverses elements of ALIGN_GROUP, and (re)aligns elements inside them as panels.
 *
 */


// Does not touch any width and height
void TreeUtilsSVG::realignObject(const Box<svg::coord_t> & anchorBox, TreeSVG & object){ // <- or a Frame?

	Logger mout(__FILE__, __FUNCTION__);

	static const std::string TEXT_ANCHOR("text-anchor");

	const bool TEXT = object->typeIs(NodeSVG::TEXT);

	const Box<svg::coord_t> & box = object->getBoundingBox();

	// ASSUME REF_RIGHT (and LEFT)
	Point2D<svg::coord_t> location(anchorBox.x + anchorBox.width, anchorBox.y);


	// Initial position is at the anchor (REF) element.
	Align::Position pos;
	switch (pos = object->getAlign(AlignSVG::Owner::ANCHOR, Align::Axis::HORZ)){
	case Align::Position::MAX:
		location.x = anchorBox.x + anchorBox.width;
		break;
	case Align::Position::MID:
		location.x = anchorBox.x + anchorBox.width/2;
		break;
	case Align::Position::MIN:
		location.x = anchorBox.x;
		break;
	case Align::Position::UNDEFINED_POS:
		break;
	default:
		mout.unimplemented<LOG_ERR>("Align::Position");
	}
	mout.attention("Align::ANCHOR-HORZ ", pos);

	switch (pos = object->getAlign(AlignSVG::Owner::ANCHOR, Align::Axis::VERT)){
	case Align::Position::MAX:
		location.y = anchorBox.y + anchorBox.height;
		break;
	case Align::Position::MID:
		location.y = anchorBox.y + anchorBox.height/2;
		break;
	case Align::Position::MIN:
		location.y = anchorBox.y;
		break;
	case Align::Position::UNDEFINED_POS:
		break;
	default:
		mout.unimplemented<LOG_ERR>("Align::Position");
	}
	mout.attention("Align::ANCHOR-VERT ", pos);


	mout.special("Adjusting location (", location, ") with element refPoint");

	switch (pos = object->getAlign(AlignSVG::Owner::OBJECT, Align::Axis::HORZ)){
	case Align::Position::MAX:
		// location.x += 0;
		if (TEXT){
			//object->setStyle(TEXT_ANCHOR, "middle");
			object->setStyle(TEXT_ANCHOR, "start"); // Default value
		}
		break;
	case Align::Position::MID:
		location.x -= box.width/2;
		if (TEXT){
			object->setStyle(TEXT_ANCHOR, "middle");
		}
		break;
	case Align::Position::MIN:
		location.x -= box.width;
		if (TEXT){
			// object->setStyle(TEXT_ANCHOR, "middle");
			object->setStyle(TEXT_ANCHOR, "end");
		}
		break;
	case Align::Position::UNDEFINED_POS: // or absolute
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG::Position");
	}
	mout.attention("Align::OBJECT-HORZ ", pos);

	if (TEXT){
		if (box.height > 0){
			location.y += box.height;
		}
		else {
			location.y -= object->style.get("font-size", 0.0);
			mout.experimental("Vertical adjust by explicit font-size=", object->style["font-size"]);
		}
	}


	switch (pos = object->getAlign(AlignSVG::Owner::OBJECT, Align::Axis::VERT)){
	case Align::Position::MAX:
		/*
		if (TEXT){
			if (box.height==0){
				location.y -= object->style.get("font-size", 0.0)/2.0;
				mout.experimental("Vertical adjust by explicit font-size=", object->style["font-size"], " /2.0");
			}
		}
		*/
		break;
	case Align::Position::MID:
		location.y -= box.height/2;
		/*
		if (TEXT){
			if (box.height==0){
				location.y -= object->style.get("font-size", 0.0)/2.0;
				mout.experimental("Vertical adjust by explicit font-size=", object->style["font-size"], " /2.0");
			}
		}
		*/
		break;
	case Align::Position::MIN:
		location.y -= box.height;
		/*
		if (TEXT){
			if (box.height==0){
				location.y -= object->style.get("font-size", 0.0);
				mout.experimental("Vertical adjust by explicit font-size=", object->style["font-size"]);
			}
		}
		*/
		break;
	case Align::Position::UNDEFINED_POS:
		break;
	default:
		mout.unimplemented<LOG_ERR>("Align::Position");
	}
	mout.attention("Align::OBJECT-VERT ", pos);


	mout.special("Final location:", location);

	// For clarity
	// Point2D<svg::coord_t> offset(location.x - anchorBox.x, location.y - anchorBox.y);
	Point2D<svg::coord_t> offset(location.x - box.x, location.y - box.y);
	translateAll(object, offset);

	// mout.accept()
	// object->setPosition(...)
	// makes attribs also in group <g> visible?
	// elem->set("x", box.x);
	// elem->set("y", box.y);


}



/**   Traverse the whole thing, at least an ALIGN_GROUP...
 *
 *    Recursion: before aligning an element, align its contents.
 *
 */

/*
const NodeSVG::path_elem_t & getAnchor(const TreeSVG & node){

	Logger mout(__FILE__, __FUNCTION__);


	return node->getAlignAnchor(); // !!


	std::list<NodeSVG::path_elem_t> anchorLegs;
	if (NodeSVG::findByClass(node, ANCHOR, anchorLegs)){
		mout.accept<LOG_NOTICE>("Aligning ID=", node->getId(), " with ANCHOR ->/", anchorLegs.front(),"/ ", anchor.data);
		if (anchorLegs.size() > 1){
			mout.warn("group ID=", node->getId(), " contained several (", anchorLegs.size(),") elements, using first at ./", anchorLegs.front());
		}
		return anchorLegs.front();
	}
	else {
		static const NodeSVG::path_elem_t dummy;
		return dummy;
	}
}
*/

void TreeUtilsSVG::superAlign(TreeSVG & object, Align::Axis orientation, LayoutSVG::Direction direction, const Point2D<svg::coord_t> & offset){

	Logger mout(__FILE__, __FUNCTION__);

	switch (object->getType()){
	case NodeXML<NodeSVG::tag_t>::STYLE:
	case NodeSVG::DESC:
	case NodeSVG::METADATA:
	case NodeSVG::TITLE:
		return;
	default:
		break;
	}

	// Depth-first
	for (TreeSVG::pair_t & entry: object){
		superAlign(entry.second);
	}

	BBoxSVG & bboxHost = object->getBoundingBox();

	// If an ANCHOR elem is set, RELATIVE objects are aligned with respect to it.
	const NodeSVG::path_elem_t & anchor = object->getAlignAnchor();
	if (!anchor.empty()){

		TreeSVG & anchorElem = object[anchor];

		// Reset for now; translate the bundle later.
		anchorElem->setLocation(0,0);

		const BBoxSVG & abox = anchorElem->getBoundingBox();
		bboxHost.expand(abox);

		for (TreeSVG::pair_t & entry: object){

			if (entry.first != anchor){
				if (!entry.second->isAligned()){
					mout.pending("No alignment for: ", entry.second.data, " using default (...?)");
					// entry.second->setA
					// AlignConfSVG alignConf;
					// LayoutSVG::getAlignmentConf(orientation, direction, alignConf); // TODO: fnct arg
					// entry.second->setMajorAlignment(orientation, direction);
					if (orientation == Align::HORZ){
						entry.second->setAlign<AlignSVG::OUTSIDE>(direction==LayoutSVG::INCR ? AlignSVG::LEFT : AlignSVG::RIGHT);
					}
					else {
						entry.second->setAlign<AlignSVG::OUTSIDE>(direction==LayoutSVG::INCR ? AlignSVG::TOP : AlignSVG::BOTTOM);
					}
					// setAlignOutside(v==Align::HORZ ? Align::HORZ : Align::VERT, d==LayoutSVG::INCR ? Align::MAX : Align::MIN);
				}
				TreeUtilsSVG::realignObject(abox, entry.second);
			}
			else {
				mout.reject<LOG_DEBUG>("Skipping anchor elem [", anchor, "] of ", object->getId());
			}
			/*
			if (entry.second->hasClass(RELATIVE)){
				TreeUtilsSVG::realignObject(abox, entry.second);
			}*/
			bboxHost.expand(entry.second->getBoundingBox());
		}

	}
	else { // Stack HORZ/VERT + INCR/DECR

		// mout.accept<LOG_NOTICE>("Aligning... ID=", object->getId(), " using stacking ");
		mout.accept<LOG_NOTICE>("Aligning... ", object.data, " using stacking ");

		BBoxSVG bbox;
		bbox.setLocation(offset); // ~ essentially a point (width==height==0).

		for (TreeSVG::pair_t & entry: object){

			if (entry.second->hasClass(LayoutSVG::FLOAT)){
				mout.pending(" ... FLOATing: ", entry.second.data);
				if (!entry.second->isAligned()){
					entry.second->setAlign<AlignSVG::INSIDE>(AlignSVG::HorzAlign::CENTER); // check
					entry.second->setAlign<AlignSVG::INSIDE>(AlignSVG::VertAlign::MIDDLE); // check
				}
				TreeUtilsSVG::realignObject(bbox, entry.second);
			}
			else {
				// mout.pending(" ... non-FLOATing: ", entry.second.data);

				if (!entry.second->isAligned()){ // Apply defaults

					if (orientation == (entry.second->hasClass(LayoutSVG::ALIGN_SCOPE) ? Align::VERT : Align::HORZ)){
						entry.second->setAlign<AlignSVG::OUTSIDE>(direction==LayoutSVG::INCR ? AlignSVG::LEFT : AlignSVG::RIGHT);
					}
					else {
						entry.second->setAlign<AlignSVG::OUTSIDE>(direction==LayoutSVG::INCR ? AlignSVG::TOP : AlignSVG::BOTTOM);
					}
					mout.pending("No alignment for: ", entry.second.data, " using default (...?)");
				}
				else {
					mout.accept<LOG_NOTICE>("Previously set align: ", entry.second.data);
				}

				TreeUtilsSVG::realignObject(bbox, entry.second);
				bbox = entry.second->getBoundingBox(); // copy
				bboxHost.expand(bbox);
				mout.accept<LOG_NOTICE>(" ... ", bbox, " now, after: ", entry.second.data);
			}



			// NOTE: use align class: (or?)
			/*
			{
				// HORZ - INCR
				bbox.x += b.width;
				bbox.y += b.height;
			}
			bbox.width  = b.width;
			bbox.height = b.height;
			*/
		}



	}


}


/**  Iteratively traverses elements of ALIGN_GROUP, and (re)aligns elements inside them as panels.
 *
 */
void TreeUtilsSVG::translateAll(TreeSVG & elem, const Point2D<svg::coord_t> & offset){

	// For many elements (like group, "G"), this property is hidden hence not appearing in attributes.
	BBoxSVG & bbox = elem->getBoundingBox();
	bbox.x += offset.x;
	bbox.y += offset.y;

	for (auto & entry: elem){
		translateAll(entry.second, offset); // recursion
	}

}

//
/**  Retrieves all the elements of ALIGN class and (re)set their positions.
 *
 */



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
