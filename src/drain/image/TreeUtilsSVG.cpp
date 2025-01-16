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

//const std::set<svg::tag_t> TreeUtilsSVG::abstractTags = {
const std::set<XML::intval_t> TreeUtilsSVG::abstractTags = {
		svg::tag_t::STYLE,
		svg::tag_t::DESC,
		svg::tag_t::METADATA,
		svg::tag_t::SCRIPT,
		svg::tag_t::TITLE,
		svg::tag_t::TSPAN,
};

//bool TreeUtilsSVG::isAbstract(svg::tag_t tag){
bool TreeUtilsSVG::isAbstract(XML::intval_t tag){
	return (abstractTags.find((XML::intval_t)tag) != abstractTags.end());
}



void TreeUtilsSVG::finalizeBoundingBox(TreeSVG & svg){

	drain::Logger mout(__FILE__, __FUNCTION__);


	drain::image::BBoxSVG bb;
	computeBoundingBox(svg, bb);


	/*
	BBoxRetrieverSVG bbr;
	TreeUtils::traverse(bbr, svg);
	const drain::Box<svg::coord_t> & bb = bbr.box;
	*/

	// Finalize top level bounding box
	svg->setFrame(bb.getFrame()); // width, height
	// Finalize view box
	const std::string viewBox = drain::StringBuilder<' '>(bb.x, bb.y, bb.width, bb.height);
	svg->set("viewBox", viewBox);

}

// Consider XMl Visitor...
/*
void TreeUtilsSVG::setRelativePaths(drain::image::TreeSVG & object, const drain::FilePath & filepath){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// TODO: add explanation
	// drain::TreeXML::path_list_t;
	std::list<drain::TreeXML::path_t> pathList;
	drain::UtilsXML::findByTag(object, drain::image::svg::IMAGE, pathList);

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
*/





void TreeUtilsSVG::realignObject(const Box<svg::coord_t> & anchorBoxHorz, const Box<svg::coord_t> & anchorBoxVert, TreeSVG & object){ // <- or a Frame?

	Logger mout(__FILE__, __FUNCTION__);

	static const std::string TEXT_ANCHOR("text-anchor");

	const Box<svg::coord_t> & box = object->getBoundingBox();
	// const Point2D<svg::coord_t>

	mout.accept<LOG_WARNING>("REALign ", object->getTag(), " bbox=", box, " elem: ", object.data);

	Point2D<svg::coord_t> location(anchorBoxHorz.x, anchorBoxVert.y);

	// realignObject(AlignBase::Axis::HORZ, anchorBoxHorz.x, anchorBoxHorz.width,  object, location.x);
	// realignObject(AlignBase::Axis::VERT, anchorBoxVert.y, anchorBoxVert.height, object, location.y);

	realignObjectHorz(object, anchorBoxHorz, location.x);
	realignObjectVert(object, anchorBoxVert, location.y);

	mout.debug("Final location:", box);

	// For clarity
	// Point2D<svg::coord_t> offset(location.x - anchorBoxHorz.x, location.y - anchorBoxVert.y);
	Point2D<svg::coord_t> offset(location.x - box.x, location.y - box.y);
	// translateAll(object, offset);

	TranslatorSVG translator(offset);
	TreeUtils::traverse(translator, object);

}


void TreeUtilsSVG::realignObjectHorz(TreeSVG & object, const Box<svg::coord_t> & anchorBoxHorz){

	svg::coord_t coord;
	realignObjectHorz(object, anchorBoxHorz, coord);

	TranslatorSVG translator(coord - object->getBoundingBox().x, 0);
	TreeUtils::traverse(translator, object);

}


/**
 *  \param anchorSpan - width or height of the achore rectangle.
 */
void TreeUtilsSVG::realignObjectHorz(TreeSVG & object, const Box<svg::coord_t> & anchorBoxHorz, svg::coord_t & coord){ // Point2D<svg::coord_t> & newLocation){

	Logger mout(__FILE__, __FUNCTION__);


	const bool IS_TEXT = object->typeIs(svg::TEXT);

	Box<svg::coord_t> & obox = object->getBoundingBox();

	if (IS_TEXT){
		// mout.debug("TEXT start coord = (", coord, ") ");
	}

	AlignBase::Pos alignLoc;

	mout.debug("Adjusting location (", ") with ANCHOR's ref point");

	switch (alignLoc = object->getAlign(AlignSVG::Owner::ANCHOR, AlignBase::Axis::HORZ)){
	case AlignBase::Pos::MIN:
		coord = anchorBoxHorz.x;
		break;
	case AlignBase::Pos::MID:
		coord = anchorBoxHorz.x + anchorBoxHorz.width/2;
		break;
	case AlignBase::Pos::MAX:
		coord = anchorBoxHorz.x + anchorBoxHorz.width;
		break;
	case AlignBase::Pos::FILL:
		mout.suspicious<LOG_WARNING>("Alignment:: ANCHOR has fill request: HORZ FILL");
		break;
	case AlignBase::Pos::UNDEFINED_POS:  // -> consider MID or some absolute value, or margin. Or error:
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', AlignBase::Axis::HORZ, '=', pos);
		break;
	default:
		// assert undefined value.
		mout.unimplemented<LOG_ERR>("Alignment::Pos: ", (int)alignLoc);
	}
	// mout.debug("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', axis, '=', alignLoc);

	// mout.debug("Adjusting ", axis, " pos (", alignLoc, ") with OBJECT's own reference point");

	static const std::string TEXT_ANCHOR("text-anchor");

	switch (alignLoc = object->getAlign(AlignSVG::Owner::OBJECT, AlignBase::Axis::HORZ)){
	case AlignBase::Pos::MIN:
		if (IS_TEXT){
			object->setStyle(TEXT_ANCHOR, "start");
			coord += object->getMargin(); //
		}
		break;
	case AlignBase::Pos::MID:
		if (IS_TEXT){
			object->setStyle(TEXT_ANCHOR, "middle");
		}
		else {
			coord -= obox.width/2;
			//location.x -= box.width/2;
		}
		break;
	case AlignBase::Pos::MAX:
		if (IS_TEXT){
			object->setStyle(TEXT_ANCHOR, "end");
			coord -= object->getMargin(); // margin
		}
		else {
			coord -= obox.width;
		}
		break;
	case AlignBase::Pos::FILL:
		//mout.experimental("STRETCHING..." );mout.experimental("STRETCHING..." );
		mout.experimental("FILL:ing horz: ",   obox, " width -> ", anchorBoxHorz.width);
		coord = anchorBoxHorz.x;
		obox.setWidth(anchorBoxHorz.width);

		break;
	case AlignBase::Pos::UNDEFINED_POS: // or absolute
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::OBJECT, '/', AlignBase::Axis::HORZ, pos);
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG::Pos ", alignLoc);
	}


	// mout.attention("Alignment::OBJECT-HORZ ", pos);
	// mout.debug("Alignment::Pos: ", AlignSVG::Owner::OBJECT, '/', axis, '=', alignLoc);

}

void TreeUtilsSVG::realignObjectVert(TreeSVG & object, const Box<svg::coord_t> & anchorBoxVert){

	svg::coord_t coord;
	realignObjectVert(object, anchorBoxVert, coord);

	// Point2D<svg::coord_t> offset(0, coord - object->getBoundingBox().y);
	TranslatorSVG translator(0, coord - object->getBoundingBox().y);
	TreeUtils::traverse(translator, object);

}


/**
 *  \param anchorSpan - width or height of the achore rectangle.
 */
void TreeUtilsSVG::realignObjectVert(TreeSVG & object, const Box<svg::coord_t> & anchorBoxVert, svg::coord_t & coord){ // Point2D<svg::coord_t> & newLocation){

	Logger mout(__FILE__, __FUNCTION__);


	const bool IS_TEXT = object->typeIs(svg::TEXT);

	Box<svg::coord_t> & obox = object->getBoundingBox();

	AlignBase::Pos alignLoc;

	mout.debug("Adjusting location (", ") with ANCHOR's ref point");

	switch (alignLoc = object->getAlign(AlignSVG::Owner::ANCHOR, AlignBase::Axis::VERT)){
	case AlignBase::Pos::MIN:
		coord = anchorBoxVert.y;
		break;
	case AlignBase::Pos::MID:
		coord = anchorBoxVert.y + anchorBoxVert.height/2;
		break;
	case AlignBase::Pos::MAX:
		coord = anchorBoxVert.y + anchorBoxVert.height;
		break;
	case AlignBase::Pos::FILL:
		mout.suspicious<LOG_WARNING>("Alignment:: ANCHOR has fill request: HORZ FILL");
		break;
	case AlignBase::Pos::UNDEFINED_POS:  // -> consider MID or some absolute value, or margin. Or error:
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', AlignBase::Axis::HORZ, '=', pos);
		break;
	default:
		// assert undefined value.
		mout.unimplemented<LOG_ERR>("Alignment::Pos: ", (int)alignLoc);
	}
	// mout.debug("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', axis, '=', alignLoc);


	// mout.debug("Adjusting ", axis, " pos (", alignLoc, ") with OBJECT's own reference point");

	static const std::string TEXT_ANCHOR("text-anchor");

	if (IS_TEXT){
		if (obox.height > 0){
			coord += obox.height;
		}
		else {
			coord += object->getStyle().get("font-size", 0.0);
			mout.experimental("Vertical adjust by explicit font-size=", object->getStyle()["font-size"]);
		}
	}

	switch (alignLoc = object->getAlign(AlignSVG::Owner::OBJECT, AlignBase::Axis::VERT)){
	case AlignBase::Pos::MIN:
		if (IS_TEXT){
			coord += object->getMargin(); //
		}
		break;
	case AlignBase::Pos::MID:
		coord -= obox.height/2;
		break;
	case AlignBase::Pos::MAX:
		coord -= obox.height;
		if (IS_TEXT){
			coord -= object->getMargin(); //
		}
		break;
	case AlignBase::Pos::FILL:
		//mout.experimental("STRETCHING..." );mout.experimental("STRETCHING..." );
		mout.experimental("FILL:ing vert: ", obox, " height ");
		coord = anchorBoxVert.y;
		obox.setHeight(anchorBoxVert.height);
		break;
	case AlignBase::Pos::UNDEFINED_POS: // or absolute
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::OBJECT, '/', AlignBase::Axis::HORZ, pos);
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG::Pos ", alignLoc);
	}

	// mout.attention("Alignment::OBJECT-HORZ ", pos);
	// mout.debug("Alignment::Pos: ", AlignSVG::Owner::OBJECT, '/', axis, '=', alignLoc);

}

/**
 *  \param anchorSpan - width or height of the achore rectangle.
 */
/*
void TreeUtilsSVG::realignObject(AlignBase::Axis axis, svg::coord_t anchorPos, svg::coord_t anchorSpan, TreeSVG & object, svg::coord_t & coord){ // Point2D<svg::coord_t> & newLocation){

	Logger mout(__FILE__, __FUNCTION__);


	const bool IS_TEXT = object->typeIs(svg::TEXT);

	if (IS_TEXT && (axis==AlignBase::Axis::HORZ)){
		mout.debug("TEXT start coord = (", coord, ") ");
	}

	AlignBase::Pos alignLoc;

	mout.debug("Adjusting location (", coord, ") with ANCHOR's ref point");

	switch (alignLoc = object->getAlign(AlignSVG::Owner::ANCHOR, axis)){
	case AlignBase::Pos::MIN:
		coord = anchorPos;
		break;
	case AlignBase::Pos::MID:
		coord = anchorPos + anchorSpan/2;
		break;
	case AlignBase::Pos::MAX:
		coord = anchorPos + anchorSpan;
		break;
	case AlignBase::Pos::FILL:
		mout.suspicious<LOG_WARNING>("Alignment:: ANCHOR has fill request: ", axis, '/', alignLoc, '/', anchorSpan);
		break;
	case AlignBase::Pos::UNDEFINED_POS:  // -> consider MID or some absolute value, or margin. Or error:
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', AlignBase::Axis::HORZ, '=', pos);
		break;
	default:
		// assert undefined value.
		mout.unimplemented<LOG_ERR>("Alignment::Pos: ", (int)alignLoc);
	}
	// mout.debug("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', axis, '=', alignLoc);


	mout.debug("Adjusting ", axis, " pos (", alignLoc, ") with OBJECT's own reference point");

	static const std::string TEXT_ANCHOR("text-anchor");

	const svg::coord_t objectSpan = (axis==AlignBase::Axis::HORZ) ? object->getWidth() : object->getHeight();

	if (IS_TEXT && (axis==AlignBase::Axis::VERT)){
		if (objectSpan > 0){
			coord += objectSpan;
		}
		else {
			coord -= object->getStyle().get("font-size", 0.0);
			mout.experimental("Vertical adjust by explicit font-size=", object->getStyle()["font-size"]);
		}
	}

	switch (alignLoc = object->getAlign(AlignSVG::Owner::OBJECT, axis)){
	case AlignBase::Pos::MIN:
		if (IS_TEXT){
			if (axis==AlignBase::Axis::HORZ){
				object->setStyle(TEXT_ANCHOR, "start");
			}
			coord += object->getMargin(); //
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
			coord -= objectSpan/2;
			//location.y -= box.width/2;
		}
		break;
	case AlignBase::Pos::MAX:
		if (IS_TEXT){
			//object->setStyle(TEXT_ANCHOR, "end"); // Default value
			if (axis==AlignBase::Axis::HORZ){
				object->setStyle(TEXT_ANCHOR, "end");
			}
			else  { // VERT, height
				coord -= objectSpan;
			}
			coord -= object->getMargin(); // margin
		}
		else {
			coord -= objectSpan;
		}
		break;
	case AlignBase::Pos::FILL:
		//mout.experimental("STRETCHING..." );mout.experimental("STRETCHING..." );
		coord = anchorPos;
		if (axis==AlignBase::Axis::HORZ){
			mout.experimental("FILL:ing horz: ",   object->getBoundingBox(), " width -> ", anchorSpan);
			object->setWidth(anchorSpan);
		}
		else if (axis==AlignBase::Axis::VERT){
			mout.experimental("FILL:ing vert: ",   object->getBoundingBox(), " height -> ", anchorSpan);
			object->setHeight(anchorSpan);
		}
		else {
			mout.warn("FILL requested, but HORZ/VERT axis (", axis, ") unset for element: ", object.data);
		}

		break;
	case AlignBase::Pos::UNDEFINED_POS: // or absolute
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::OBJECT, '/', AlignBase::Axis::HORZ, pos);
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG::Pos ", alignLoc);
	}

	if (IS_TEXT && (axis==AlignBase::Axis::HORZ)){
		mout.debug2("TEXT final newPos = (", coord, ") ");
	}

	// mout.attention("Alignment::OBJECT-HORZ ", pos);
	// mout.debug("Alignment::Pos: ", AlignSVG::Owner::OBJECT, '/', axis, '=', alignLoc);

}
*/


/**   Traverse the whole thing, at least an ALIGN_GROUP...
 *
 *    Recursion: before aligning an element, align its contents.
 *
 */


/**
 *  \b Anchors.
 *
 *  In SVG, elements that directly define a graphical object – like \c RECT, \c CIRCLE or \c POLYLINE are here called \e graphical \e elements .
 *  Examples of other, \e abstract elements are \c G (group), \c METADATA or \c DESC elements.
 *
 *  Alignment of graphical element can controlled by methods of drain::image::AlignSVG inherited by drain::image::NodeSVG .
 *  Elements can be aligned sequentially - stacked in rows or colums - as well as superposing them over other elements.
 *
 *  Elements respect to which another element is aligned is called an anchor. Anchors can be implicit or explicitly set.
 *
 *  - By default, any graphical element serves as an anchor for the next element (in order of definition in an SVG document).
 *  - However, if a graphical element is assigned \e style \e class drain::image::LayoutSVG::FLOAT , the previous anchor will stay intact.
 *  - Any group \c G can explicitly define one of its children (direct descendants) to be the default anchor for all the elements in the group.
 *  - Any graphical element can override the previous rules by setting any of its siblings (children of the same group) to be its private anchor.
 *  -
 *
 *
 *
 *  \param offset - start?
 *
 *  \see NodeSVG
 */
void TreeUtilsSVG::superAlign(TreeSVG & object, AlignBase::Axis orientation, LayoutSVG::Direction direction){ //, const Point2D<svg::coord_t> & offset){ // offsetInit

	Logger mout(__FILE__, __FUNCTION__);

	if (object->isAbstract()){
		return;
	}

	/*
	if (isAbstract((svg::tag_t)object->getType())){ // skip TITLE, DESC etc.
		return;
	}
	*/

	// mout.attention("ACCEPT:", object->getTag());
	mout.special<LOG_NOTICE>(__FUNCTION__, " start: ", object.data); //, object->getId(), " -> ", object->getBoundingBox());


	// Depth-first
	for (TreeSVG::pair_t & entry: object){
		superAlign(entry.second, orientation, direction); // , offset);
	}

	// mout.attention("ACCEPT:", object->getTag());
	mout.special<LOG_NOTICE>(__FUNCTION__, " handle: ", object.data); //, object->getId(), " -> ", object->getBoundingBox());


	/// If main orientation (inside containers) is HORZ, stack containers VERT. And vice versa.
	if (object->hasClass(LayoutSVG::ALIGN_FRAME)){
		if (orientation == drain::image::AlignBase::Axis::VERT){
			object->setAlign((direction==LayoutSVG::Direction::INCR) ? AlignSVG::RIGHT : AlignSVG::LEFT, AlignSVG::OUTSIDE);
			object->setAlign(AlignSVG::TOP, AlignSVG::INSIDE);
		}
		else { // VERT  -> ASSERT? if (ctx.mainOrientation == drain::image::AlignBase::Axis::VERT){
			object->setAlign(AlignSVG::LEFT, AlignSVG::INSIDE);
			object->setAlign((direction==LayoutSVG::Direction::INCR) ? AlignSVG::BOTTOM : AlignSVG::TOP, AlignSVG::OUTSIDE);
		}
	}

	// Element's bbox, to be updated below
	BBoxSVG & objectBBox = object->getBoundingBox();

	const TreeSVG::path_elem_t & anchorHorz = object->getAlignAnchorHorz();
	const bool GROUP_ANCHOR_HORZ  = object->typeIs(svg::GROUP) && !anchorHorz.empty();

	// const bool ANCHOR_HORZ_STACKING   = anchorHorz.empty();     // Previous element is the anchor for next.
	// const bool ANCHOR_HORZ_CUMULATIVE = (anchorHorz == "_bbox"); // BBox expanded this far is the anchor.
	// const bool ANCHOR_HORZ_FIXED      = !(ANCHOR_HORZ_STACKING || ANCHOR_HORZ_CUMULATIVE);

	const TreeSVG::path_elem_t & anchorVert = object->getAlignAnchorVert();
	const bool GROUP_ANCHOR_VERT  = object->typeIs(svg::GROUP) && !anchorVert.empty();

	// const bool ANCHOR_VERT_STACKING   = anchorVert.empty();     // Previous element is the anchor for next.
	// const bool ANCHOR_VERT_CUMULATIVE = (anchorVert == "_bbox"); // BBox expanded this far is the anchor.
	// const bool ANCHOR_VERT_FIXED      = !(ANCHOR_VERT_STACKING || ANCHOR_VERT_CUMULATIVE);

	// Incrementally growing extent (width/height)
	BBoxSVG bbox;
	BBoxSVG *bboxAnchorHorz = &bbox;
	BBoxSVG *bboxAnchorVert = &bbox;

	if (GROUP_ANCHOR_HORZ){
		bboxAnchorHorz = & object[anchorHorz]->getBoundingBox();
		objectBBox.expand(*bboxAnchorHorz);
		// or ASSIGN?
	}


	if (GROUP_ANCHOR_VERT){
		bboxAnchorVert = & object[anchorVert]->getBoundingBox();
		objectBBox.expand(*bboxAnchorVert);
		// or ASSIGN?
	}

	// bboxAnchorHorz->setLocation(0, 0); // replaces offset?
	// bboxAnchorVert->setLocation(0, 0); // replaces offset?

	for (TreeSVG::pair_t & entry: object){

		if (entry.second->isAbstract()){
			continue;
		}

		/*
		if (entry.second->typeIs(svg::GROUP)){ //  ? ... or could the specific elem be "master", also for the GROUP?
			continue;
		}
		*/

		if (!entry.second->isAligned()){
			if (entry.second->hasClass(LayoutSVG::FLOAT)){
				entry.second->setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER); // check
			}
			else {
				if (orientation == AlignBase::Axis::HORZ){
					entry.second->setAlign(direction==LayoutSVG::Direction::INCR ? AlignSVG::RIGHT : AlignSVG::LEFT, AlignSVG::OUTSIDE);
					entry.second->setAlign(AlignSVG::TOP);
				}
				else {
					entry.second->setAlign(AlignSVG::LEFT);
					entry.second->setAlign(direction==LayoutSVG::Direction::INCR ? AlignSVG::BOTTOM : AlignSVG::TOP, AlignSVG::OUTSIDE);
				}
			}
		}

		// If the element has specific (sibling) anchor, use it.

		const TreeSVG::path_elem_t & ah = entry.second->getAlignAnchorHorz();
		if (ah == "*"){
			TreeUtilsSVG::realignObjectHorz(entry.second, objectBBox); // this far
		}
		else if (object.hasChild(ah)){ // Specific horz anchor
			TreeUtilsSVG::realignObjectHorz(entry.second, object[ah]->getBoundingBox());
		}
		else if (entry.first != anchorHorz){
			TreeUtilsSVG::realignObjectHorz(entry.second, *bboxAnchorHorz);
		}

		const TreeSVG::path_elem_t & av = entry.second->getAlignAnchorVert();
		if (av == "*"){
			//mout.special<LOG_NOTICE>("Aligning ", entry.first, " => ", entry.second);
			TreeUtilsSVG::realignObjectVert(entry.second, objectBBox); // this far
		}
		else if (object.hasChild(av)){ // Specific vert anchor
			TreeUtilsSVG::realignObjectVert(entry.second, object[av]->getBoundingBox());
		}
		else if (entry.first != anchorVert){
			TreeUtilsSVG::realignObjectVert(entry.second, *bboxAnchorVert);
		}


		if (!entry.second->hasClass(LayoutSVG::FLOAT)){

			bbox = entry.second->getBoundingBox(); // Notice: copy
			objectBBox.expand(bbox);

			if (!GROUP_ANCHOR_HORZ){
				mout.experimental<LOG_INFO>("Jumping HORZ anchor to: /", entry.first, "/ <", entry.second->getTag(), "> ", entry.second->getId());
				bboxAnchorHorz = & entry.second->getBoundingBox();
			}

			if (!GROUP_ANCHOR_VERT){
				// Jumping anchor
				mout.experimental<LOG_INFO>("Jumping VERT anchor to: /", entry.first, "/ <", entry.second->getTag(), "> ", entry.second->getId());
				bboxAnchorVert = & entry.second->getBoundingBox();
			}

			mout.accept<LOG_NOTICE>("Expanding ... ", objectBBox, " horz:", *bboxAnchorHorz, " vert:", *bboxAnchorVert);
			/// mout.accept<LOG_NOTICE>("Expanding ... ", bbox, " now, after: ", entry.second.data);

		}


	}

}



/**  Iteratively traverses elements of ALIGN_GROUP, and (re)aligns elements inside them as panels.
 *
 */
void TreeUtilsSVG::translateAll(TreeSVG & object, const Point2D<svg::coord_t> & offset){

	switch (object->getType()){
	case NodeXML<svg::tag_t>::STYLE:
	case svg::DESC:
	case svg::METADATA:
	case svg::TITLE:
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
int TranslatorSVG::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path) {
	TreeSVG & node = tree(path);
	if (TreeUtilsSVG::isAbstract((svg::tag_t)node->getType())){ // skip TITLE, DESC etc.
		return 1;
	}
	else {
		BBoxSVG & bbox = node->getBoundingBox();
		bbox.x += offset.x;
		bbox.y += offset.y;
		return 0;
	}
}


// Consider XMl Visitor...
bool TreeUtilsSVG::computeBoundingBox(const TreeSVG & elem, Box<svg::coord_t> & box){

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


int BBoxRetrieverSVG::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path) {
	TreeSVG & node = tree(path);
	if (node->typeIs(svg::SVG) || node->typeIs(svg::GROUP)){
		return 0; // continue (traverse children)
	}
	else {
		return 1; // skip this subtree
	}
}

int BBoxRetrieverSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path) {

	TreeSVG & node = tree(path);

	/*
	if (node->typeIs(svg::SVG) || node->typeIs(svg::GROUP)){
		return 0;
	}
	else
	*/
	if (node->typeIs(svg::TEXT)){
		// Note: logic fails if given as first input (next call will see still empty bbox)
		if (box.empty()){
			box.setLocation(node->getBoundingBox().getLocation());
		}
		else {
			box.expand(node->getBoundingBox().getLocation());
		}
	}
	else {
		if (box.empty()){
			box = node->getBoundingBox();
		}
		else {
			// drain::Logger mout(__FILE__, __FUNCTION__);
			//mout.pending("Updating bbox ", box, " with ", elem->getBoundingBox());
			box.expand(node->getBoundingBox());
		}
	}
	return 0;
}





int RelativePathSetterSVG::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path) {
	TreeSVG & node = tree(path);
	if (node->typeIs(svg::SVG) || node->typeIs(svg::GROUP)){
		return 0; // continue (traverse also children)
	}
	else if (node->typeIs(svg::IMAGE)){
		drain::image::TreeSVG & imageNode = tree(path);
		const std::string imagePath = imageNode->get("xlink:href");
		if (drain::StringTools::startsWith(imagePath, dir)){
			imageNode->set("xlink:href", imagePath.substr(dir.size())); // TODO: setURL ?
		}
		else {
			// mout.attention("could not set relative path: ", p, " href:", imagePath);
		}
	}
	return 1; // skip this subtree
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
