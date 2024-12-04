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
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.pending("Updating bbox ", box, " with ", elem->getBoundingBox());
			box.expand(elem->getBoundingBox());
			/*
			const Box<NodeSVG::coord_t> & b = elem->getBoundingBox();
			const Point2D<NodeSVG::coord_t> pMax(std::max(box.x+box.width,  b.x+b.width), std::max(box.y+box.height, b.y+b.height));
			box.x = std::min(box.x, b.x);
			box.y = std::min(box.y, b.y);
			box.width  = pMax.x - box.x;
			box.height = pMax.y - box.y;
			*/
		}
	}

	return (box.getWidth() != 0.0) && (box.getHeight() != 0.0);

}

/** Stacks all the non-FLOAT elements horizontally or vertically, computing their collective width or height.
 *
 */
void TreeUtilsSVG::getBoundingFrame(const TreeSVG & group, Frame2D<int> & frame, LayoutSVG::Orientation orientation){

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

		if (!elem->hasClass(AlignSVG::FLOAT)){
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

void TreeUtilsSVG::alignSequenceOLD(TreeSVG & group, const drain::Frame2D<int> & frame, const drain::Point2D<int> & start, LayoutSVG::Orientation orientation, LayoutSVG::Direction direction){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("aligning elems of group ", group->get("name", "?"));
	mout.unimplemented(__FUNCTION__);

		/*
	if (orientation == LayoutSVG::UNDEFINED_ORIENTATION){
		orientation = TreeUtilsSVG::defaultConf.layout.orientation;
		if (orientation == LayoutSVG::UNDEFINED_ORIENTATION){
			orientation = LayoutSVG::HORZ;
		}
	}

	if (direction == LayoutSVG::UNDEFINED_DIRECTION){
		direction = TreeUtilsSVG::defaultConf.layout.direction;
		if (direction == LayoutSVG::UNDEFINED_DIRECTION){
			direction = LayoutSVG::INCR;
		}
	}

	// Loop: stack horizontally or vertically.
	// Position of the current object
	Point2D<int> pos(0,0);

	if (direction==LayoutSVG::DECR){
		if (orientation==LayoutSVG::HORZ){
			pos.x = frame.width;
		}
		else if (orientation==LayoutSVG::VERT){
			pos.y = frame.height;
		}
	}

	pos.x += start.x;
	pos.y += start.y;

	// Position offset for the current object
	Point2D<int> offset;

	NodeSVG::path_list_t paths;

	NodeSVG::findByTags(group, {svg::IMAGE,svg::RECT,svg::TEXT}, paths);

	//bool first=true;
	for (const NodeSVG::path_t  & p: paths){

		TreeSVG & elem = group(p);

		NodeSVG::elem_t t = elem->getType();

		if (elem->hasClass("FIXED")) // currently not used...
			continue;

		if (elem->hasClass(ALIGN)){ // currently not used...
			drain::FlexibleVariable & host = elem->get(attr_FRAME_REFERENCE);
			if (host.empty()){
				elem->set("host", group->getId()); // rAise
			}
			continue;
		}

		// elem->addClass(orientation);

		if ((t == svg::IMAGE) || (t == svg::RECT) || (t == svg::TEXT)){


			if (!elem->hasClass(AlignSVG::FLOAT)){ // FLOAT = on top

				if (direction==LayoutSVG::INCR){
					if (orientation==LayoutSVG::HORZ)
						pos.x += offset.x;
					else
						pos.y += offset.y;
				}

				offset.x = elem->get("width", 0);
				offset.y = elem->get("height",0);

				if (direction==LayoutSVG::DECR){
					if (orientation==LayoutSVG::HORZ)
						pos.x -= offset.x;
					else
						pos.y -= offset.y;
				}
			}

			if (t == svg::TEXT){
				elem->set("x", pos.x);
				elem->set("y", pos.y + 20); // TODO fonts
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

	*/
}


/// Marker for...
const std::string TreeUtilsSVG::attr_FRAME_REFERENCE("data-frame-ref");


// #define  DRAIN_ENUM_NAMESPACE drain::image::AlignSVG
/*
template <>
const drain::EnumDict<AlignSVG_FOO>::dict_t  drain::EnumDict<AlignSVG_FOO>::dict = {

		//ENUM_ENTRY(drain::image::AlignSVG, PANEL),

		DRAIN_ENUM_ENTRY(drain::image::AlignSVG_FOO, ALIGN_GROUP),
		///
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG_FOO, LEFT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, RIGHT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, CENTER),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, TOP),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, BOTTOM),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, MIDDLE),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, REF_LEFT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, REF_RIGHT),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, REF_CENTER),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, REF_TOP),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, REF_BOTTOM),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, REF_MIDDLE),
		///
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, FLOAT), // (probably deprecating)
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, ALIGN), // (probably deprecating)
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, PANEL), //
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, ANCHOR), //
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, RELATIVE), //
};
 */


void TreeUtilsSVG::markAligned(const TreeSVG & elem, TreeSVG & alignedElem){
	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.unimplemented<LOG_ERR>();
	/*
	alignedElem->addClass(AlignSVG::FLOAT);
	alignedElem->addClass(AlignSVG::ALIGN);
	alignedElem->set(attr_FRAME_REFERENCE, elem->getId());
	*/
}


void TreeUtilsSVG::alignText(TreeSVG & group){

	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.unimplemented<LOG_ERR>();
	/*
	drain::image::NodeSVG::path_list_t pathList;

	//mout.note("koe: ", group->getTag(), " name: ", group->get("name", "?"));

	// drain::image::NodeSVG::findByClass(group, cls_FLOAT, pathList);

	drain::image::NodeSVG::findByClass(group, AlignSVG::FLOAT, pathList);

	// mout.accept<LOG_WARNING>("yes accept: ", drain::EnumDict<AlignSVG>::dict.getKey(FLOAT));

	const drain::Point2D<double> textOffset(5.0, 5.0);

	for (const drain::image::NodeSVG::path_t & path: pathList){

		// mout.reject<LOG_WARNING>("yes accept: ", path);


		TreeSVG & elem =  group(path);
		if (elem -> typeIs(NodeSVG::TEXT)){

			// mout.note("realign: ", path);

			const std::string ref = elem->get(attr_FRAME_REFERENCE, "");
			if (ref.empty()){
				mout.warn("'ref' attribute missing for TEXT.FLOAT elem ", elem->get("name",""), " at ", path);
			}

			drain::image::NodeSVG::path_t p;

			if (NodeSVG::findById(group, ref, p)){

				// mout.note("for TEXT.FLOAT(", path, ") found IMAGE/RECT '", ref, "'");

				TreeSVG & frame = group(p);
				Box<svg::coord_t> box = frame->getBoundingBox();

				if (!box.empty()){

					// start | middle | end

					// Book keeping by parent element for rows.
					std::string locationLabel = "data-titles";

					if (elem->hasClass(AlignSVG::MIDDLE)){
						locationLabel += "_m";
					}
					else if (elem->hasClass(AlignSVG::BOTTOM)){
						locationLabel += "_b";
					}
					else { // Default: elem->hasClass("TOP")
						locationLabel += "_t";
					}

					if (elem->hasClass(AlignSVG::CENTER)){
						elem->set("x", box.x + 0.5*box.width);
						elem->setStyle("text-anchor", "middle");
						locationLabel += "_c";
					}
					else if (elem->hasClass(AlignSVG::RIGHT)){
						elem->set("x",  box.x + box.width - textOffset.x);
						elem->setStyle("text-anchor", "end");
						locationLabel += "_r";
					}
					else { // Default: elem->hasClass("LEFT")
						elem->set("x", box.x + textOffset.x);
						elem->setStyle("text-anchor", "start");
						locationLabel += "_l";
					}

					const int index = frame->get(locationLabel, 0);
					const int fontSize = elem->style.get("font-size", 30); // what about "30em" ?

					if (elem->hasClass(AlignSVG::MIDDLE)){
						elem->set("y", box.y + 0.75*box.height + textOffset.y + fontSize*index); // FIX: should be SUM of invidual row widths
					}
					else if (elem->hasClass(AlignSVG::BOTTOM)){
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
	*/
}


// void alignPanels(TreeSVG & alignGroup, drain::Point2D<double> &pos);

//
/**  Retrieves and traverses elements of ALIGN_GROUP, and (re)aligns elements inside them as panels.
 *
 */
void TreeUtilsSVG::alignDomains(TreeSVG & group){

	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.unimplemented<LOG_ERR>();

	drain::image::NodeSVG::path_list_t paths;
	/*
	drain::image::NodeSVG::findByClass(group, ALIGN_GROUP, paths);
	// mout.accept<LOG_WARNING>("yes accept: ", drain::EnumDict<AlignSVG>::dict.getKey(FLOAT));
	// const drain::Point2D<double> textOffset(5.0, 5.0);

	Point2D<double> pos = {0.0, 0.0};

	for (const drain::image::NodeSVG::path_t & path: paths){
		mout.attention("Aligning ALIGN_GROUP at ", path);
		alignPanels(group(path), pos);
	}
	*/

}

// Does not touch any width and height
void TreeUtilsSVG::realignObject(const Box<svg::coord_t> & anchorBox, TreeSVG & object){ // <- or a Frame?

	Logger mout(__FILE__, __FUNCTION__);

	static const std::string TEXT_ANCHOR("text-anchor");

	const bool TEXT = object->typeIs(NodeSVG::TEXT);

	const Box<svg::coord_t> & box = object->getBoundingBox();

	// ASSUME REF_RIGHT (and LEFT)
	Point2D<svg::coord_t> location(anchorBox.x + anchorBox.width, anchorBox.y);


	// Initial position is at the anchor (REF) element.
	switch (object->getAlign(AlignSVG2::pos_t::REF, AlignSVG2::axis_t::HORZ)){
	case AlignSVG2::value_t::MAX:
		location.x = anchorBox.x + anchorBox.width;
		break;
	case AlignSVG2::value_t::MID:
		location.x = anchorBox.x + anchorBox.width/2;
		break;
	case AlignSVG2::value_t::MIN:
		location.x = anchorBox.x;
		break;
	case AlignSVG2::value_t::UNDEFINED:
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG2::value_t");
	}

	switch (object->getAlign(AlignSVG2::pos_t::REF, AlignSVG2::axis_t::VERT)){
	case AlignSVG2::value_t::MAX:
		location.y = anchorBox.y + anchorBox.height;
		break;
	case AlignSVG2::value_t::MID:
		location.y = anchorBox.y + anchorBox.height/2;
		break;
	case AlignSVG2::value_t::MIN:
		location.y = anchorBox.y;
		break;
	case AlignSVG2::value_t::UNDEFINED:
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG2::value_t");
	}


	mout.special("Initial adjust, location:", location);

	switch (object->getAlign(AlignSVG2::pos_t::OBJ, AlignSVG2::axis_t::HORZ)){
	case AlignSVG2::value_t::MAX:
		// location.x += 0;
		if (TEXT){
			object->setStyle(TEXT_ANCHOR, "start"); // Default value
		}
		break;
	case AlignSVG2::value_t::MID:
		location.x -= box.width/2;
		if (TEXT){
			object->setStyle(TEXT_ANCHOR, "middle");
		}
		break;
	case AlignSVG2::value_t::MIN:
		location.x -= box.width;
		if (TEXT){
			object->setStyle(TEXT_ANCHOR, "end");
		}
		break;
	case AlignSVG2::value_t::UNDEFINED: // or absolute
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG2::value_t");
	}

	switch (object->getAlign(AlignSVG2::pos_t::OBJ, AlignSVG2::axis_t::VERT)){
	case AlignSVG2::value_t::MAX:
		break;
	case AlignSVG2::value_t::MID:
		location.y -= box.height/2;
		break;
	case AlignSVG2::value_t::MIN:
		location.y -= box.height;
		break;
	case AlignSVG2::value_t::UNDEFINED:
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG2::value_t");
	}


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

void TreeUtilsSVG::superAlign(TreeSVG & object, const Point2D<svg::coord_t> & offset){

	Logger mout(__FILE__, __FUNCTION__);

	// Depth-first
	for (TreeSVG::pair_t & entry: object){
		superAlign(entry.second);
	}

	BBoxSVG & bboxHost = object->getBoundingBox();

	// If an ANCHOR elem is found among children, RELATIVE objects are aligned with respect to it.
	const NodeSVG::path_elem_t & anchor = object->getAlignAnchor();
	if (!anchor.empty()){

		TreeSVG & anchorElem = object[anchor];

		// Reset for now; translate the bundle later.
		anchorElem->setLocation(0,0);

		const BBoxSVG & abox = anchorElem->getBoundingBox();
		bboxHost.expand(abox);

		for (TreeSVG::pair_t & entry: object){

			if (entry.first != anchor){
				if (entry.second->isAligned()){
					TreeUtilsSVG::realignObject(abox, entry.second);
				}
				else {
					mout.pending("No alignment for: ", entry.second.data);
				}
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

		mout.accept<LOG_NOTICE>("Aligning... ID=", object->getId(), " using stacking ");

		BBoxSVG bbox;
		bbox.setLocation(offset); // ~ essentially a point (width==height==0).

		for (TreeSVG::pair_t & entry: object){
			TreeUtilsSVG::realignObject(bbox, entry.second);
			//const Frame2D<svg::coord_t> & frame = entry.second->getBoundingBox();
			// const BBoxSVG & b = entry.second->getBoundingBox();
			bbox = entry.second->getBoundingBox(); // copy
			bboxHost.expand(bbox);

			mout.accept<LOG_NOTICE>(bbox, " now, after: ", entry.second.data);

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


void TreeUtilsSVG::alignPanels(TreeSVG & alignGroup, Point2D<double> &startPos){ // <- or a Frame?

	Logger mout(__FILE__, __FUNCTION__);
	mout.unimplemented<LOG_ERR>();

	/*
	NodeSVG::path_list_t panelPaths;
	NodeSVG::findByClass(alignGroup, PANEL, panelPaths); // TODO: (non)recursive NodeSVG::findByClass with pathElem ?!

	// std::list<NodeSVG::path_elem_t> panelPaths;
	// NodeSVG::findChildrenByClass(alignGroup, cls, result);

	double maxHeight = 0;
	for (const NodeSVG::path_t & pp: panelPaths){
		mout.attention("           --> PANEL at ", pp);

		TreeSVG & panelGroup = alignGroup(pp);

		NodeSVG::path_list_t anchorPaths;
		NodeSVG::findByClass(panelGroup, ANCHOR, anchorPaths);
		if (anchorPaths.empty()){
			mout.reject<LOG_WARNING>("PANEL group at ", pp, " ID=", panelGroup->getId(), " contained no ANCHOR element ");
			continue;
		}
		if (anchorPaths.size() > 1){
			mout.warn("PANEL group has more than one (", anchorPaths.size(),") ANCHOR element, panel: ", pp);
		}
		TreeSVG & panelAnchor = panelGroup(anchorPaths.front());
		panelAnchor->set("x", startPos.x);
		panelAnchor->set("y", startPos.y);
		const Box<NodeSVG::coord_t> anchorBox = panelAnchor->getBoundingBox(); // COPY   // could be: box(pos.x, pos.y, ... )
		//TreeUtilsSVG::getElementBounds(panelAnchor, anchorBox);

		// Finally, align relative elements
		NodeSVG::path_list_t paths;
		NodeSVG::findByClass(panelGroup, RELATIVE, paths);
		for (const NodeSVG::path_t & path: paths){
			TreeSVG & elem = panelGroup(path);
			TreeUtilsSVG::realignObject(anchorBox, elem);
		} // for each RELATIVE elem

		Box<svg::coord_t> panelBox;
		TreeUtilsSVG::computeBoundingBox(panelGroup, panelBox);
		TreeSVG & debugFrame = panelGroup["debugFrame"](svg::RECT);
		debugFrame->setBoundingBox(panelBox);
		debugFrame->setStyle("fill:none; stroke:red; stroke-width:3; stroke-dasharray:5,5");

		// SUPER DEBUG
		{
			TreeSVG tmp(svg::SVG);
			tmp->setBoundingBox(panelBox); // panelAnchor->getBoundingBox());
			TreeSVG & group = tmp["main"](svg::GROUP);
			group.data = panelGroup.data; //->set(panelGroup->getAttributes().getMap());
			//group->classList = panelGroup->classList;
			// pp.str()
			group.swap(panelGroup);
			drain::Output output(panelGroup->getId() + ".svg");
			NodeSVG::docToStream(output.getStream(), tmp);
			panelGroup.swap(group);
		}

		// TENTATIVE/ fix using orientation, and translate?
		maxHeight = std::max(maxHeight, panelAnchor->get("height", 0.0));
		startPos.x += (10+panelAnchor->get("width", 0));
		startPos.y += 10;
		// Frame2D<int> panelFrame;
		//TreeUtilsSVG::getBoundingFrame(panelGroup, panelFrame, LayoutSVG::HORZ);


	} // for each PANEL elem

	// TENTATIVE/ fix using orientation, and translate?
	startPos.x += 10;
	startPos.y += (10+maxHeight);
	*/
}

/**  Retrieves and traverses elements of ALIGN_GROUP, and (re)aligns elements inside them as panels.
 *
 */
void TreeUtilsSVG::translateAll(TreeSVG & elem, const Point2D<svg::coord_t> & offset){


	/*
	NodeSVG::map_t & attr = elem->getAttributes();
	if (attr.hasKey("x") &&  attr.hasKey("y")){
		attr["x"] = attr.get("x",0.0) + offset.x;
		attr["y"] = attr.get("y",0.0) + offset.y;
	}
	*/
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
