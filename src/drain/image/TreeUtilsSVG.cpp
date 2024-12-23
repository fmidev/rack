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



PanelConfSVG TreeUtilsSVG::defaultConf;

// Consider XMl Visitor...
bool TreeUtilsSVG::computeBoundingBox(const TreeSVG & elem, Box<NodeSVG::coord_t> & box){

	if (elem->typeIs(svg::SVG) || elem->typeIs(svg::GROUP)){
		for (const TreeSVG::pair_t & entry: elem){
			computeBoundingBox(entry.second, box); // recursion
		}
	}
	else if (elem->typeIs(svg::TEXT)){
		// skip, trust others (rect anchor)
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

void TreeUtilsSVG::finalizeBoundingBox(TreeSVG & svg){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// if (svg.getT)

	drain::image::BBoxSVG bb;
	computeBoundingBox(svg, bb);

	// Finalize top level bounding box
	svg->setFrame(bb.getFrame()); // width, height
	// Finalize view box
	const std::string viewBox = drain::StringBuilder<' '>(bb.x, bb.y, bb.width, bb.height);
	svg->set("viewBox", viewBox);

}

// Consider XMl Visitor...
void TreeUtilsSVG::setRelativePaths(drain::image::TreeSVG & object, const drain::FilePath & filepath){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// TODO: add explanation
	drain::image::NodeSVG::path_list_t pathList;
	drain::image::NodeSVG::findByTag(object, drain::image::svg::IMAGE, pathList);

	if (!filepath.dir.empty()){
		const std::string dir = filepath.dir.str()+'/';  // <- consider plain, and remove leading slashes, or add only if non-empty.
		for (drain::image::NodeSVG::path_t & p: pathList){
			drain::image::TreeSVG & image = object(p);
			const std::string imagePath = image->get("xlink:href");
			if (drain::StringTools::startsWith(imagePath, dir)){
				image->set("xlink:href", imagePath.substr(dir.size())); // TODO: setURL ?
			}
			else {
				mout.attention("could not set relative path: ", p, " href:", imagePath);
			}
			//mout.attention("path: ", p, " href:", image->get("xlink:href"));
		}
	}

}



//
/**  Retrieves and traverses elements of ALIGN_GROUP, and (re)aligns elements inside them as panels.
 *
 */


/// Does not "know" about top-level layout; only implements what has been requested for this object.
/**
 *   Consider also Horz:STRETCH = true
 *   Repositions object (x,y). Currently, does not touch any width and height.
void TreeUtilsSVG::realignObject(const Box<svg::coord_t> & anchorBox, TreeSVG & object){ // <- or a Frame?
	realignObject(anchorBox, anchorBox, object);
}
 */


void TreeUtilsSVG::realignObject(const Box<svg::coord_t> & anchorBoxHorz, const Box<svg::coord_t> & anchorBoxVert, TreeSVG & object){ // <- or a Frame?

	Logger mout(__FILE__, __FUNCTION__);

	static const std::string TEXT_ANCHOR("text-anchor");

	const Box<svg::coord_t> & box = object->getBoundingBox();

	// Assume diagonal stack... Consider: accept absolute pos, if UNDEFINED_
	// Point2D<svg::coord_t> location(anchorBoxHorz.x + anchorBoxHorz.width*3/4, anchorBoxVert.y + anchorBoxVert.height*3/4);
	// Point2D<svg::coord_t> location(anchorBoxHorz.x + anchorBoxHorz.width, anchorBoxVert.y + anchorBoxVert.height);
	Point2D<svg::coord_t> location(anchorBoxHorz.x, anchorBoxVert.y);

	realignObject(AlignBase::Axis::HORZ, anchorBoxHorz.x, anchorBoxHorz.width,  object, location.x);
	realignObject(AlignBase::Axis::VERT, anchorBoxVert.y, anchorBoxVert.height, object, location.y);

	mout.special("Final location:", location);

	// For clarity
	// Point2D<svg::coord_t> offset(location.x - anchorBox.x, location.y - anchorBox.y);
	Point2D<svg::coord_t> offset(location.x - box.x, location.y - box.y);
	translateAll(object, offset);

}



/**
 *  \param anchorSpan - width or height of the achore rectangle.
 */
void TreeUtilsSVG::realignObject(AlignBase::Axis axis, svg::coord_t anchorPos, svg::coord_t anchorSpan, TreeSVG & object, svg::coord_t & newPos){ // Point2D<svg::coord_t> & newLocation){

	Logger mout(__FILE__, __FUNCTION__);

	// svg::coord_t & newPos = newLocation.x; // FIX
	mout.debug("Adjusting location (", newPos, ") with ANCHOR's ref point");

	const bool IS_TEXT = object->typeIs(NodeSVG::TEXT);

	if (IS_TEXT && (axis==AlignBase::Axis::HORZ)){
		mout.reject<LOG_WARNING>("TEXT start newPos = (", newPos, ") ");
	}

	AlignBase::Pos alignLoc;
	switch (alignLoc = object->getAlign(AlignSVG::Owner::ANCHOR, axis)){
	case AlignBase::Pos::MIN:
		newPos = anchorPos;
		break;
	case AlignBase::Pos::MID:
		newPos = anchorPos + anchorSpan/2;
		break;
	case AlignBase::Pos::MAX:
		newPos = anchorPos + anchorSpan;
		break;
	case AlignBase::Pos::UNDEFINED_POS:  // -> consider MID or some absolute value, or margin. Or error:
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', AlignBase::Axis::HORZ, '=', pos);
		break;
	default:
		// assert undefined value.
		mout.unimplemented<LOG_ERR>("Alignment::Pos: ", (int)alignLoc);
	}
	mout.attention("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', axis, '=', alignLoc);


	mout.debug("Adjusting ", axis, " pos (", alignLoc, ") with OBJECT's own reference point");

	static const std::string TEXT_ANCHOR("text-anchor");

	const svg::coord_t objectSpan = (axis==AlignBase::Axis::HORZ) ? object->getBoundingBox().getWidth() : object->getBoundingBox().getHeight();

	if (IS_TEXT && (axis==AlignBase::Axis::VERT)){
		if (objectSpan > 0){
			newPos += objectSpan;
		}
		else {
			newPos -= object->style.get("font-size", 0.0);
			mout.experimental("Vertical adjust by explicit font-size=", object->style["font-size"]);
		}
	}

	switch (alignLoc = object->getAlign(AlignSVG::Owner::OBJECT, axis)){
	case AlignBase::Pos::MIN:
		if (IS_TEXT && (axis==AlignBase::Axis::HORZ)){
			object->setStyle(TEXT_ANCHOR, "start");
			newPos += object->getBoundingBox().getWidth(); // margin!
		}
		else {
			// no action
		}
		break;
	case AlignBase::Pos::MID:
		if (IS_TEXT && (axis==AlignBase::Axis::HORZ)){ // && (bbox.width == 0)
			object->setStyle(TEXT_ANCHOR, "middle");
		}
		else {
			newPos -= objectSpan/2;
			//location.x -= box.width/2;
		}
		break;
	case AlignBase::Pos::MAX:
		if (IS_TEXT && (axis==AlignBase::Axis::HORZ)){
			object->setStyle(TEXT_ANCHOR, "end"); // Default value
			newPos -= object->getBoundingBox().getWidth(); // margin
		}
		else {
			newPos -= objectSpan;
		}
		break;
	case AlignBase::Pos::FILL:
		mout.experimental<LOG_WARNING>("STRETCHING..." );
		newPos = anchorPos;
		if (axis==AlignBase::Axis::HORZ){
			object->getBoundingBox().setWidth(anchorSpan);
		}
		else if (axis==AlignBase::Axis::VERT){
			object->getBoundingBox().setHeight(anchorSpan);
		}
		else {
			mout.warn("FILL requested, but HORZ/VERT axis unset for element: ", object.data);
		}

		break;
	case AlignBase::Pos::UNDEFINED_POS: // or absolute
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::OBJECT, '/', AlignBase::Axis::HORZ, pos);
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG::Pos");
	}

	if (IS_TEXT && (axis==AlignBase::Axis::HORZ)){
		mout.reject<LOG_WARNING>("TEXT final newPos = (", newPos, ") ");
	}


	// mout.attention("Alignment::OBJECT-HORZ ", pos);
	mout.attention("Alignment::Pos: ", AlignSVG::Owner::OBJECT, '/', axis, '=', alignLoc);

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
void TreeUtilsSVG::superAlign(TreeSVG & object, AlignBase::Axis orientation, LayoutSVG::Direction direction){ //, const Point2D<svg::coord_t> & offset){ // offsetInit

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
		superAlign(entry.second, orientation, direction); // , offset);
	}


	if (object->hasClass(LayoutSVG::ALIGN_FRAME)){
		if (orientation == drain::image::AlignBase::Axis::VERT){
			object->setAlign(AlignBase::Axis::HORZ, (direction==LayoutSVG::Direction::INCR) ? AlignBase::Pos::MAX : AlignBase::Pos::MIN, AlignSVG::OUTSIDE);
			object->setAlign(AlignBase::Axis::VERT, AlignBase::Pos::MIN, AlignSVG::INSIDE); // = AlignSVG::VertAlignment::TOP);
		}
		else { // VERT  -> ASSERT? if (ctx.mainOrientation == drain::image::AlignBase::Axis::VERT){
			object->setAlign(AlignBase::Axis::HORZ, AlignBase::Pos::MIN, AlignSVG::INSIDE);
			object->setAlign(AlignBase::Axis::VERT, (direction==LayoutSVG::Direction::INCR) ? AlignBase::Pos::MAX : AlignBase::Pos::MIN, AlignSVG::OUTSIDE);
		}
	}

	// Element's bbox, to be updated below
	BBoxSVG & objectBBox = object->getBoundingBox();


	BBoxSVG *bboxAnchorHorz = nullptr;
	const NodeSVG::path_elem_t & anchorElemHorz = object->getAlignAnchorHorz(); // FIX for both axes
	const bool FIXED_ANCHOR_HORZ = !anchorElemHorz.empty();

	BBoxSVG *bboxAnchorVert = nullptr;
	const NodeSVG::path_elem_t & anchorElemVert = object->getAlignAnchorVert(); // FIX for both axes
	const bool FIXED_ANCHOR_VERT = !anchorElemVert.empty();

	// Explain! Dynamically growing extent (width/height)
	BBoxSVG bbox;
	//bbox.setLocation(offset); // ~ essentially a point (width==height==0).

	if (FIXED_ANCHOR_HORZ){
		bboxAnchorHorz = & object[anchorElemHorz]->getBoundingBox();
		objectBBox.expand(*bboxAnchorHorz); // Anchor elem typically has spatial extent while parent object is a group.
	}
	else {
		bboxAnchorHorz = & bbox;
	}

	if (FIXED_ANCHOR_VERT){
		bboxAnchorVert = & object[anchorElemVert]->getBoundingBox();
		objectBBox.expand(*bboxAnchorVert); // Anchor elem typically has spatial extent while parent object is a group.
	}
	else {
		bboxAnchorVert = & bbox;
	}

	// bboxAnchorHorz->x = 0;
	// bboxAnchorVert->y = 0;
	bboxAnchorHorz->setLocation(0, 0); // replaces offset?
	bboxAnchorVert->setLocation(0, 0); // replaces offset?
	// what about bbox.setLocation(offset); // ~ essentially a point (width==height==0).

	for (TreeSVG::pair_t & entry: object){

		if (FIXED_ANCHOR_HORZ){
			if (entry.first == anchorElemHorz){ // what if still aligned vertically?
				mout.reject<LOG_WARNING>("A bit... aligning HORZ anchor elem /", anchorElemHorz, "/ of ", object->getId());
				//TreeUtilsSVG::realignObject(*bboxAnchorHorz, *bboxAnchorVert, entry.second);
				continue;  // OR.. align!?!
			}
		}

		if (FIXED_ANCHOR_VERT){

			if (entry.first == anchorElemVert){ // what if still aligned horizontally?
				mout.reject<LOG_WARNING>("Not aligning VERT anchor elem /", anchorElemVert, "/ of ", object->getId());
				continue;
			}

		}

		if (entry.second->hasClass(LayoutSVG::FLOAT)){

			mout.pending<LOG_NOTICE>(" ... Object [FLOAT]ing: ", entry.second.data);
			// GENERAL default?
			if (!entry.second->isAligned()){
				entry.second->setAlign(AlignSVG::CENTER, AlignSVG::INSIDE); // check
				entry.second->setAlign(AlignSVG::MIDDLE, AlignSVG::INSIDE); // check
			}

			TreeUtilsSVG::realignObject(*bboxAnchorHorz, *bboxAnchorVert, entry.second);
		}
		else {

			// mout.pending(" ... non-FLOATing: ", entry.second.data);
			if (!entry.second->isAligned()){ // Apply defaults
				if (orientation == AlignBase::Axis::HORZ){
					entry.second->setAlign(direction==LayoutSVG::Direction::INCR ? AlignSVG::RIGHT : AlignSVG::LEFT, AlignSVG::OUTSIDE);
					entry.second->setAlign(AlignSVG::TOP, AlignSVG::INSIDE);
				}
				else {
					entry.second->setAlign(AlignSVG::LEFT, AlignSVG::INSIDE);
					entry.second->setAlign(direction==LayoutSVG::Direction::INCR ? AlignSVG::BOTTOM : AlignSVG::TOP, AlignSVG::OUTSIDE);
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
			//if (!entry.second->typeIs(svg::TEXT)){
			bbox = entry.second->getBoundingBox(); // Notice: copy
			objectBBox.expand(bbox);
			mout.accept<LOG_NOTICE>("  ... ", bbox, " now, after: ", entry.second.data);
			//}

			if (!FIXED_ANCHOR_HORZ){
				/// Move anchor to the last element aligned.
				mout.experimental<LOG_WARNING>("Flipping HORZ anchor to: /", entry.first, "/ <", entry.second->getTag(), "> ", entry.second->getId());
				bboxAnchorHorz = & entry.second->getBoundingBox(); // objectBBox; // entry.second->getBoundingBox();
			}

			if (!FIXED_ANCHOR_VERT){
				/// Move anchor to the last element aligned.
				mout.experimental<LOG_WARNING>("Flipping VERT anchor to: /", entry.first, "/ <", entry.second->getTag(), "> ", entry.second->getId());
				bboxAnchorVert = & entry.second->getBoundingBox(); // objectBBox; // entry.second->getBoundingBox();
				//mout.warn("UPDATE anchor VERT bbox: ", *bboxAnchorVert);
			}

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
