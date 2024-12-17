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

/*
template <>
const drain::EnumDict<PanelConfSVG::Legend>::dict_t  drain::EnumDict<PanelConfSVG::Legend>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Legend, NO_LEGEND),
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Legend, LEFT),
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Legend, RIGHT),
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Legend, DUPLEX),
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Legend, EMBED)
};
*/




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


/// Does not "know" about top-level layout; only implements what has been requested for this object.
/**
 *   Consider also Horz:STRETCH = true
 *   Repositions object (x,y). Currently, does not touch any width and height.
 */
void TreeUtilsSVG::realignObject(const Box<svg::coord_t> & anchorBox, TreeSVG & object){ // <- or a Frame?
	realignObject(anchorBox, anchorBox, object);
}

/**
 *  \param anchorSpan - width or height of the achore rectangle.
 */
void TreeUtilsSVG::realignObject(Align::Axis axis, svg::coord_t anchorPos, svg::coord_t anchorSpan, TreeSVG & object, svg::coord_t & newPos){ // Point2D<svg::coord_t> & newLocation){

	Logger mout(__FILE__, __FUNCTION__);

	// svg::coord_t & newPos = newLocation.x; // FIX
	mout.debug("Adjusting location (", newPos, ") with ANCHOR's ref point");

	Align::Coord alignLoc;
	switch (alignLoc = object->getAlign(AlignSVG::Owner::ANCHOR, axis)){
	case Align::Coord::MIN:
		newPos = anchorPos;
		break;
	case Align::Coord::MID:
		newPos = anchorPos + anchorSpan/2;
		break;
	case Align::Coord::MAX:
		newPos = anchorPos + anchorSpan;
		break;
	case Align::Coord::UNDEFINED_POS:  // -> consider MID or some absolute value, or margin. Or error:
		// mout.unimplemented<LOG_WARNING>("Align::Coord: ", AlignSVG::Owner::ANCHOR, '/', Align::Axis::HORZ, '=', pos);
		break;
	default:
		// assert undefined value.
		mout.unimplemented<LOG_ERR>("Align::Coord: ", (int)alignLoc);
	}
	mout.attention("Align::Coord: ", AlignSVG::Owner::ANCHOR, '/', axis, '=', alignLoc);


	mout.debug("Adjusting ", axis, " pos (", alignLoc, ") with OBJECT's own reference point");

	const bool IS_TEXT = object->typeIs(NodeSVG::TEXT);
	static const std::string TEXT_ANCHOR("text-anchor");

	const svg::coord_t objectSpan = (axis==Align::Axis::HORZ) ? object->getBoundingBox().getWidth() : object->getBoundingBox().getHeight();

	if (IS_TEXT && (axis==Align::Axis::VERT)){
		if (objectSpan > 0){
			newPos += objectSpan;
		}
		else {
			newPos -= object->style.get("font-size", 0.0);
			mout.experimental("Vertical adjust by explicit font-size=", object->style["font-size"]);
		}
	}

	switch (alignLoc = object->getAlign(AlignSVG::Owner::OBJECT, axis)){
	case Align::Coord::MIN:
		if (IS_TEXT && (axis==Align::Axis::HORZ)){
			object->setStyle(TEXT_ANCHOR, "start");
		}
		else {
			// no action
		}
		break;
	case Align::Coord::MID:
		if (IS_TEXT && (axis==Align::Axis::HORZ)){ // && (bbox.width == 0)
			object->setStyle(TEXT_ANCHOR, "middle");
		}
		else {
			newPos -= objectSpan/2;
			//location.x -= box.width/2;
		}
		break;
	case Align::Coord::MAX:
		if (IS_TEXT && (axis==Align::Axis::HORZ)){
			object->setStyle(TEXT_ANCHOR, "end"); // Default value
		}
		else {
			newPos -= objectSpan;
		}
		break;
	case Align::Coord::UNDEFINED_POS: // or absolute
		// mout.unimplemented<LOG_WARNING>("Align::Coord: ", AlignSVG::Owner::OBJECT, '/', Align::Axis::HORZ, pos);
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG::Coord");
	}
	// mout.attention("Align::OBJECT-HORZ ", pos);
	mout.attention("Align::Coord: ", AlignSVG::Owner::OBJECT, '/', axis, '=', alignLoc);

}




/**   Traverse the whole thing, at least an ALIGN_GROUP...
 *
 *    Recursion: before aligning an element, align its contents.
 *
 */


/**
 *  \param offset - start?
 *
 */
void TreeUtilsSVG::superAlign(TreeSVG & object, Align::Axis orientation, LayoutSVG::Direction direction, const Point2D<svg::coord_t> & offset){ // offsetInit

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
		superAlign(entry.second, orientation, direction, offset);
	}


	if (object->hasClass(LayoutSVG::ALIGN_FRAME)){
		if (orientation == drain::image::Align::Axis::VERT){
			object->setAlign(AlignSVG::OUTSIDE, Align::Axis::HORZ, (direction==LayoutSVG::Direction::INCR) ? Align::MAX : Align::MIN);
			object->setAlign(AlignSVG::INSIDE,  Align::Axis::VERT, Align::MIN); // = AlignSVG::VertAlign::TOP);
		}
		else { // VERT  -> ASSERT? if (ctx.mainOrientation == drain::image::Align::Axis::VERT){
			object->setAlign(AlignSVG::INSIDE,  Align::Axis::HORZ, Align::MIN); // = AlignSVG::HorzAlign::LEFT);
			object->setAlign(AlignSVG::OUTSIDE, Align::Axis::VERT, (direction==LayoutSVG::Direction::INCR) ? Align::MAX : Align::MIN);
		}
	}

	// Element's bbox, to be updated below
	BBoxSVG & objectBBox = object->getBoundingBox();

	BBoxSVG *bboxAnchorHorz = nullptr;
	const NodeSVG::path_elem_t & anchorHorz = object->getAlignAnchorHorz(); // FIX for both axes
	const bool FIXED_ANCHOR_HORZ = !anchorHorz.empty();

	BBoxSVG *bboxAnchorVert = nullptr;
	const NodeSVG::path_elem_t & anchorVert = object->getAlignAnchorVert(); // FIX for both axes
	const bool FIXED_ANCHOR_VERT = !anchorVert.empty();


	// Explain!
	BBoxSVG bbox;
	bbox.setLocation(offset); // ~ essentially a point (width==height==0).

	if (FIXED_ANCHOR_HORZ){
		bboxAnchorHorz = & object[anchorHorz]->getBoundingBox();
	}
	else {
		bboxAnchorHorz = & bbox;
	}

	if (FIXED_ANCHOR_VERT){
		bboxAnchorVert = & object[anchorVert]->getBoundingBox();
	}
	else {
		bboxAnchorVert = & bbox;
	}

	// bboxAnchorHorz->x = 0;
	// bboxAnchorVert->y = 0;
	bboxAnchorHorz->setLocation(0, 0);
	bboxAnchorVert->setLocation(0, 0);

	for (TreeSVG::pair_t & entry: object){

		if (FIXED_ANCHOR_HORZ){
			if (entry.first == anchorHorz){ // what if still aligned vertically?
				mout.reject<LOG_DEBUG>("Not aligning HORZ anchor elem /", anchorHorz, "/ of ", object->getId());
				continue;
			}
		}

		if (FIXED_ANCHOR_VERT){

			if (entry.first == anchorVert){ // what if still aligned horizontally?
				mout.reject<LOG_DEBUG>("Not aligning VERT anchor elem /", anchorVert, "/ of ", object->getId());
				continue;
			}

		}

		if (entry.second->hasClass(LayoutSVG::FLOAT)){

			mout.pending<LOG_NOTICE>(" ... Object [FLOAT]ing: ", entry.second.data);
			// GENERAL default?
			if (!entry.second->isAligned()){
				entry.second->setAlign(AlignSVG::INSIDE, AlignSVG::CENTER); // check
				entry.second->setAlign(AlignSVG::INSIDE, AlignSVG::MIDDLE); // check
			}

			TreeUtilsSVG::realignObject(*bboxAnchorHorz, *bboxAnchorVert, entry.second);
		}
		else {

			// mout.pending(" ... non-FLOATing: ", entry.second.data);
			if (!entry.second->isAligned()){ // Apply defaults
				if (orientation == Align::HORZ){
					entry.second->setAlign(AlignSVG::OUTSIDE, direction==LayoutSVG::Direction::INCR ? AlignSVG::RIGHT : AlignSVG::LEFT);
					entry.second->setAlign(AlignSVG::INSIDE, AlignSVG::TOP);
				}
				else {
					entry.second->setAlign(AlignSVG::INSIDE, AlignSVG::LEFT);
					entry.second->setAlign(AlignSVG::OUTSIDE, direction==LayoutSVG::Direction::INCR ? AlignSVG::BOTTOM : AlignSVG::TOP);
				}
				mout.reject<LOG_NOTICE>("Forced: ", entry.second.data);
				//mout.pending("No alignment for: ", entry.second.data, " using defaults... (?)");
			}
			else {
				mout.accept<LOG_NOTICE>("Previously set align: ");
			}

			//mout.accept<LOG_NOTICE>("Align [STACK] ", entry.second -> getId(), ' ', entry.second.data); // object->getTag(), " ", object->getId());
			mout.accept<LOG_NOTICE>("Align [NEW] ", entry.second -> getId(), ' ', entry.second.data); // object->getTag(), " ", object->getId());

			// TreeUtilsSVG::realignObject(bbox, entry.second);
			TreeUtilsSVG::realignObject(*bboxAnchorHorz, *bboxAnchorVert, entry.second);
			bbox = entry.second->getBoundingBox(); // Notice: copy
			objectBBox.expand(bbox);
			mout.accept<LOG_NOTICE>("  ... ", bbox, " now, after: ", entry.second.data);

			if (!FIXED_ANCHOR_HORZ){
				/// Move anchor to the last element aligned.
				bboxAnchorHorz = & entry.second->getBoundingBox(); // objectBBox; // entry.second->getBoundingBox();
			}

			if (!FIXED_ANCHOR_VERT){
				/// Move anchor to the last element aligned.
				bboxAnchorVert = & entry.second->getBoundingBox(); // objectBBox; // entry.second->getBoundingBox();
				mout.warn("UPDATE anchor VERT bbox: ", *bboxAnchorVert);
			}

		}


	}

}

void TreeUtilsSVG::superAlignOLD(TreeSVG & object, Align::Axis orientation, LayoutSVG::Direction direction, const Point2D<svg::coord_t> & offset){ // offsetInit

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

	// Point2D<svg::coord_t> offset(offsetInit);

	// Depth-first
	for (TreeSVG::pair_t & entry: object){
		superAlign(entry.second, orientation, direction, offset);
	}

	if (object->hasClass(LayoutSVG::ALIGN_FRAME)){
		if (orientation == drain::image::Align::Axis::VERT){
			object->setAlign(AlignSVG::OUTSIDE, Align::Axis::HORZ, (direction==LayoutSVG::Direction::INCR) ? Align::MAX : Align::MIN);
			object->setAlign(AlignSVG::INSIDE,  Align::Axis::VERT, Align::MIN); // = AlignSVG::VertAlign::TOP);
		}
		else { // VERT  -> ASSERT? if (ctx.mainOrientation == drain::image::Align::Axis::VERT){
			object->setAlign(AlignSVG::INSIDE,  Align::Axis::HORZ, Align::MIN); // = AlignSVG::HorzAlign::LEFT);
			object->setAlign(AlignSVG::OUTSIDE, Align::Axis::VERT, (direction==LayoutSVG::Direction::INCR) ? Align::MAX : Align::MIN);
		}
		//orientation = Align::flip(orientation);
	}


	BBoxSVG & objectBBox = object->getBoundingBox(); // element's bbox (updated above)

	// If an ANCHOR elem is set, RELATIVE objects are aligned with respect to it.
	const NodeSVG::path_elem_t & anchor = object->getAlignAnchorHorz(); // FIX for both axes
	if (!anchor.empty()){

		TreeSVG & anchorElem = object[anchor];

		// Reset for now; translate the bundle later below.
		anchorElem->setLocation(0,0);

		const BBoxSVG & abox = anchorElem->getBoundingBox();
		objectBBox.expand(abox);

		for (TreeSVG::pair_t & entry: object){

			switch (entry.second->getType()){
			case NodeXML<NodeSVG::tag_t>::STYLE:
			case NodeSVG::DESC:
			case NodeSVG::METADATA:
			case NodeSVG::TITLE:
				continue;
			default:
				break;
			}

			if (entry.first == anchor){
				mout.reject<LOG_DEBUG>("Skipping anchor elem [", anchor, "] of ", object->getId());
				continue;
			}
			else {
				if (!entry.second->isAligned()){
					mout.pending<LOG_NOTICE>("No alignment for: ", entry.second.data, " using default (...?)");
					if (orientation == Align::HORZ){
						entry.second->setAlign(AlignSVG::OUTSIDE, direction==LayoutSVG::INCR ? AlignSVG::RIGHT : AlignSVG::LEFT);
						entry.second->setAlign(AlignSVG::INSIDE, AlignSVG::TOP); // check! using orient, or even global last?
					}
					else {
						entry.second->setAlign(AlignSVG::INSIDE, AlignSVG::LEFT); // check! using orient, or even global last?
						entry.second->setAlign(AlignSVG::OUTSIDE, direction==LayoutSVG::INCR ? AlignSVG::BOTTOM : AlignSVG::TOP);
					}
					// setAlignOutside(v==Align::HORZ ? Align::HORZ : Align::VERT, d==LayoutSVG::INCR ? Align::MAX : Align::MIN);
				}
				// mout.accept<LOG_NOTICE>("Align[ANCHORed] ", entry.second.data);
				mout.accept<LOG_NOTICE>("Align [ANCHOR=", anchor, "] ", entry.second -> getId(), ' ', entry.second.data); // object->getTag(), " ", object->getId());
				TreeUtilsSVG::realignObject(abox, entry.second);
			}
			/*
			if (entry.second->hasClass(RELATIVE)){
				TreeUtilsSVG::realignObject(abox, entry.second);
			}*/
			objectBBox.expand(entry.second->getBoundingBox());
		}

	}
	else { // Stack HORZ/VERT + INCR/DECR

		// mout.accept<LOG_NOTICE>("Align [STACK] ", object->getTag(), " ", object->getId());
		// mout.accept<LOG_NOTICE>("Align[STACK] ", object.data);

		BBoxSVG bbox;
		bbox.setLocation(offset); // ~ essentially a point (width==height==0).

		for (TreeSVG::pair_t & entry: object){

			if (entry.second->hasClass(LayoutSVG::FLOAT)){
				mout.pending<LOG_NOTICE>(" ... Object [FLOAT]ing: ", entry.second.data);
				if (!entry.second->isAligned()){
					entry.second->setAlign(AlignSVG::INSIDE, AlignSVG::CENTER); // check
					entry.second->setAlign(AlignSVG::INSIDE, AlignSVG::MIDDLE); // check
				}
				TreeUtilsSVG::realignObject(bbox, entry.second);
			}
			else {
				// mout.pending(" ... non-FLOATing: ", entry.second.data);

				if (!entry.second->isAligned()){ // Apply defaults
					// if (orientation == (entry.second->hasClass(LayoutSVG::ALIGNED) ? Align::VERT : Align::HORZ)){
					if (orientation == Align::HORZ){
						//entry.second->setAlign<AlignSVG::OUTSIDE>(direction==LayoutSVG::INCR ? AlignSVG::HorzAlign::LEFT : AlignSVG::HorzAlign::RIGHT);
						entry.second->setAlign(AlignSVG::OUTSIDE, direction==LayoutSVG::Direction::INCR ? AlignSVG::RIGHT : AlignSVG::LEFT);
						entry.second->setAlign(AlignSVG::INSIDE, AlignSVG::TOP);
						// entry.second->setAlign<AlignSVG::INSIDE>(direction==LayoutSVG::INCR ? AlignSVG::TOP : AlignSVG::BOTTOM);
						// entry.second->setAlign<AlignSVG::INSIDE>(direction==LayoutSVG::INCR ? AlignSVG::TOP : AlignSVG::BOTTOM);
					}
					else {
						entry.second->setAlign(AlignSVG::INSIDE, AlignSVG::LEFT);
						entry.second->setAlign(AlignSVG::OUTSIDE, direction==LayoutSVG::Direction::INCR ? AlignSVG::BOTTOM : AlignSVG::TOP);
					}
					mout.reject<LOG_NOTICE>("Forced: ", entry.second.data);
					//mout.pending("No alignment for: ", entry.second.data, " using defaults... (?)");
				}
				else {
					mout.accept<LOG_NOTICE>("Previously set align: ");
				}

				mout.accept<LOG_NOTICE>("Align [STACK] ", entry.second -> getId(), ' ', entry.second.data); // object->getTag(), " ", object->getId());

				TreeUtilsSVG::realignObject(bbox, entry.second);
				bbox = entry.second->getBoundingBox(); // copy
				objectBBox.expand(bbox);
				mout.accept<LOG_NOTICE>("  ... ", bbox, " now, after: ", entry.second.data);
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
void TreeUtilsSVG::translateAll(TreeSVG & object, const Point2D<svg::coord_t> & offset){

	switch (object->getType()){
	case NodeXML<NodeSVG::tag_t>::STYLE:
	case NodeSVG::DESC:
	case NodeSVG::METADATA:
	case NodeSVG::TITLE:
		return;
	default:
		break;
	}

	// For many elements (like group, "G"), this property is hidden hence not appearing in attributes.
	BBoxSVG & bbox = object->getBoundingBox();
	bbox.x += offset.x;
	bbox.y += offset.y;

	for (TreeSVG::pair_t & entry: object){
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


void TreeUtilsSVG::realignObject(const Box<svg::coord_t> & anchorBoxHorz, const Box<svg::coord_t> & anchorBoxVert, TreeSVG & object){ // <- or a Frame?

	Logger mout(__FILE__, __FUNCTION__);

	static const std::string TEXT_ANCHOR("text-anchor");


	const Box<svg::coord_t> & box = object->getBoundingBox();

	// Assume diagonal stack... Consider: accept absolute pos, if UNDEFINED_
	Point2D<svg::coord_t> location(anchorBoxHorz.x + anchorBoxHorz.width*3/4, anchorBoxVert.y + anchorBoxVert.height*3/4);

	realignObject(Align::Axis::HORZ, anchorBoxHorz.x, anchorBoxHorz.width,  object, location.x);
	realignObject(Align::Axis::VERT, anchorBoxVert.y, anchorBoxVert.height, object, location.y);

	/*
	mout.debug("Adjusting location (", location, ") with ANCHOR's own ref point");

	// Initial position is at the anchor (REF) element.
	Align::Coord pos;
	switch (pos = object->getAlign(AlignSVG::Owner::ANCHOR, Align::Axis::HORZ)){
	case Align::Coord::MAX:
		location.x = anchorBoxHorz.x + anchorBoxHorz.width;
		break;
	case Align::Coord::MIN:
		location.x = anchorBoxHorz.x;
		break;
	case Align::Coord::MID:
		location.x = anchorBoxHorz.x + anchorBoxHorz.width/2;
		break;
	case Align::Coord::UNDEFINED_POS:  // or absolute
		// mout.unimplemented<LOG_WARNING>("Align::Coord: ", AlignSVG::Owner::ANCHOR, '/', Align::Axis::HORZ, '=', pos);
		break;
	default:
		mout.unimplemented<LOG_ERR>("Align::Coord: ", (int)pos);
	}
	mout.attention("Align::Coord: ", AlignSVG::Owner::ANCHOR, '/', Align::Axis::HORZ, '=', pos);

	switch (pos = object->getAlign(AlignSVG::Owner::ANCHOR, Align::Axis::VERT)){
	case Align::Coord::MAX:
		location.y = anchorBoxVert.y + anchorBoxVert.height;
		break;
	case Align::Coord::MIN:
		location.y = anchorBoxVert.y;
		break;
	case Align::Coord::MID:
		location.y = anchorBoxVert.y + anchorBoxVert.height/2;
		break;
	case Align::Coord::UNDEFINED_POS:  // or absolute
		// mout.unimplemented<LOG_WARNING>("Align::Coord: ", AlignSVG::Owner::ANCHOR, '/', Align::Axis::VERT, '=', pos);
		break;
	default:
		mout.unimplemented<LOG_ERR>("Align::Coord: ", (int)pos);
	}
	mout.attention("Align::Coord: ", AlignSVG::Owner::ANCHOR, '/', Align::Axis::VERT, '=', pos);


	mout.debug("Adjusting location (", location, ") with OBJECT's own reference point");

	const bool IS_TEXT = object->typeIs(NodeSVG::TEXT);

	switch (pos = object->getAlign(AlignSVG::Owner::OBJECT, Align::Axis::HORZ)){
	case Align::Coord::MAX:
		if (IS_TEXT){
			object->setStyle(TEXT_ANCHOR, "start"); // Default value
		}
		else {
			location.x -= box.width;
		}
		break;
	case Align::Coord::MIN:
		if (IS_TEXT){
			object->setStyle(TEXT_ANCHOR, "end");
		}
		break;
	case Align::Coord::MID:
		if (IS_TEXT){ // && (bbox.width == 0)
			object->setStyle(TEXT_ANCHOR, "middle");
		}
		else {
			location.x -= box.width/2;
		}
		break;
	case Align::Coord::UNDEFINED_POS: // or absolute
		// mout.unimplemented<LOG_WARNING>("Align::Coord: ", AlignSVG::Owner::OBJECT, '/', Align::Axis::HORZ, pos);
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG::Coord");
	}
	// mout.attention("Align::OBJECT-HORZ ", pos);
	mout.attention("Align::Coord: ", AlignSVG::Owner::OBJECT, '/', Align::Axis::HORZ, '=', pos);


	if (IS_TEXT){
		if (box.height > 0){
			location.y += box.height;
		}
		else {
			location.y -= object->style.get("font-size", 0.0);
			mout.experimental("Vertical adjust by explicit font-size=", object->style["font-size"]);
		}
	}

	switch (pos = object->getAlign(AlignSVG::Owner::OBJECT, Align::Axis::VERT)){
	case Align::Coord::MAX:
		location.y -= box.height;
		break;
	case Align::Coord::MID:
		location.y -= box.height/2;
		break;
	case Align::Coord::MIN:
		//location.y -= box.height;
		break;
	case Align::Coord::UNDEFINED_POS:
		// mout.unimplemented<LOG_WARNING>("Align::Coord: ", AlignSVG::Owner::OBJECT, '/', Align::Axis::VERT, pos);
		break;
	default:
		mout.unimplemented<LOG_ERR>("Align::Coord", (int)pos);
	}
	// mout.attention("Align::OBJECT-VERT ", pos);
	mout.attention("Align::Coord: ", AlignSVG::Owner::OBJECT, '/', Align::Axis::VERT, '=', pos);

	*/

	mout.special("Final location:", location);

	// For clarity
	// Point2D<svg::coord_t> offset(location.x - anchorBox.x, location.y - anchorBox.y);
	Point2D<svg::coord_t> offset(location.x - box.x, location.y - box.y);
	translateAll(object, offset);

	// mout.accept()
	// object->setCoord(...)
	// makes attribs also in group <g> visible?
	// elem->set("x", box.x);
	// elem->set("y", box.y);


}


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

}  // image::

}  // drain::
