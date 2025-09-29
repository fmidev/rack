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





// PanelConfSVG TreeUtilsSVG::defaultConf;

/*
const std::set<XML::intval_t> TreeUtilsSVG::abstractTags = {
		svg::tag_t::STYLE,
		svg::tag_t::DESC,
		svg::tag_t::METADATA,
		svg::tag_t::SCRIPT,
		svg::tag_t::TITLE,
		svg::tag_t::TSPAN,
};
*/



void TreeUtilsSVG::detectBoxNEW(drain::image::TreeSVG & group, bool debug){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//warn("nesting SVG elem not handled");

	// typedef svg::tag_t tag_t;

	if (group->typeIs(svg::GROUP, svg::SVG)){

		BBoxSVG bbox; // for <svg>

		BBoxSVG & b = (group->typeIs(svg::GROUP)) ? group->getBoundingBox() : bbox;

		b.reset();

		//bool GRAPHICS = false;
		unsigned short count = 0;
		for (auto & entry: group.getChildren()){

			// mout.pending<LOG_NOTICE>("initial BBOX ", b, " of: [", entry.first, "]=", entry.second.data);
			drain::image::NodeSVG & node = entry.second.data;

			if (node.typeIs(svg::GROUP, svg::SVG)){

				//BBoxSVG b2;
				detectBoxNEW(entry.second, debug); // returns if not SVG or G

				// Now sub-GROUP bbox is ready.
				b.expand(node.getBoundingBox()); // is this ok? Consider N-level nesting (and translation)

				// mout.accept<LOG_NOTICE>("BBOX after GROUP/SVG", b);
				++count;
			}
			else if (!node.isAbstract()){ // is graphic

				switch (node.getNativeType()){
				case svg::CIRCLE: {
					double cx = node.get("cx");
					double cy = node.get("cy");
					double r  = node.get("r");
					b.expand(cx + r, cy + r);
					b.expand(cx - r, cy - r);
					// mout.accept<LOG_NOTICE>("BBOX after svg::CIRCLE: ", b);
					// node.get("data-bb") << b.x << b.y << b.width << b.height;
				}
				break;
				case svg::RECT: {
					b.expand(node.getBoundingBox());
					// mout.accept<LOG_NOTICE>("BBOX after svg::RECT: ", b);
					//node.get("data-bb") << b.x << b.y << b.width << b.height;
				}
				break;
				case svg::POLYGON: {
					std::list<std::string> points;
					double x=0.0, y=0.0;
					drain::StringTools::split(node.get("points",""), points, ' '); // note: separator not comma
					for (const auto & point: points){
						drain::StringTools::split2(point, x,y, ','); // note: separator not comma
						b.expand(x, y);
					}
					// mout.accept<LOG_NOTICE>("BBOX after POLYGON: ", b);
				}
				break;
				case svg::TEXT:
					// "Point-like object"
					// Height is real, relates to row height.
					// Width is the margin only, yet pushed "bbox" out a bit.
					// TEXT is assumed to be left/right/center aligned with enough space.
					b.expand(node.getBoundingBox());
					// b.expand(node.getBoundingBox().getLocation());
					mout.accept<LOG_NOTICE>("BBOX after svg::TEXT: ", b);
					break;
				default:
					mout.warn("unhandled type: ", node.getType());
					continue;
				}

				++count;
			}

		}

		mout.accept<LOG_NOTICE>("elem ", group->getId(), " has ", count, " children, debug=", debug);

		// Develop...
		if (debug && (count > 1)){
			drain::image::NodeSVG & debugRect = group["debugBBOX"](svg::RECT);
			debugRect.addClass("DEBUG");
			debugRect.setName("debugBBOX");
			// debugRect.setStyle("stroke-width", 3);
			// debugRect.setStyle("stroke-style", "dotted");
			debugRect.getBoundingBox() = b;
			debugRect.set("x", b.x);
			debugRect.set("y", b.y);
			debugRect.setFrame(b.getFrame());
		}

	}


}




/// Set instructions, flipping axis in nesting groups of type STACK_LAYOUT
/**
 *  MOVE
 */
void TreeUtilsSVG::addStackLayout(TreeSVG & object, AlignBase::Axis orientation, LayoutSVG::Direction direction){ //, const Point2D<svg::coord_t> & offset){ // offsetInit

	Logger mout(__FILE__, __FUNCTION__);

	NodeSVG & node = object.data;

	if (!node.typeIs(svg::GROUP, svg::SVG, svg::RECT, svg::IMAGE, svg::TEXT)){
		return;
	}

	// mout.attention("ACCEPT:", object->getTag());
	mout.special<LOG_DEBUG>(__FUNCTION__, " start: ", node); //, object->getId(), " -> ", object->getBoundingBox());

	if (node.hasClass(LayoutSVG::FLOAT)){
		if (!node.isAligned()){
			node.setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER); // check
		}
	}
	else { // if (object->hasClass(LayoutSVG::ALIGN)){
		if (node.isAligned()){
			//mout.special("Overriding Align for: ", object.data);
		}
		else {
			setStackLayout(node, orientation, direction);
		}
	}

	// Recursion
	if (node.typeIs(svg::SVG, svg::GROUP)){

		if (node.hasClass(LayoutSVG::STACK_LAYOUT)){
			orientation = AlignBase::flip(orientation);
			mout.special("flipped orientation to ", orientation, " for children of ", node);
		}

		for (TreeSVG::pair_t & entry: object){
			addStackLayout(entry.second, orientation, direction);
		}
	}


}

void TreeUtilsSVG::setStackLayout(NodeSVG & node, AlignBase::Axis orientation, LayoutSVG::Direction direction){

	if (orientation == drain::image::AlignBase::Axis::HORZ){
		if (direction==LayoutSVG::Direction::INCR){
			node.setAlign(AlignSVG::RIGHT, AlignSVG::OUTSIDE);
		}
		else {
			node.setAlign(AlignSVG::LEFT, AlignSVG::OUTSIDE);
		}
		// Assign "hanging"
		node.setAlign(AlignSVG::TOP, AlignSVG::INSIDE); // for some apps, could be BOTTOM as well?
	}
	else {
		if (direction==LayoutSVG::Direction::INCR){
			node.setAlign(AlignSVG::BOTTOM, AlignSVG::OUTSIDE);
		}
		else {
			node.setAlign(AlignSVG::TOP, AlignSVG::OUTSIDE);
		}
		node.setAlign(AlignSVG::LEFT, AlignSVG::INSIDE); // for some apps, could be RIGHT as well?
	}
}


template <>
inline
void CoordSpan<AlignBase::Axis::HORZ>::copyFrom(const BBoxSVG & bbox){
	if (bbox.isDefined()){
		pos  = bbox.x;
		span = bbox.width;
	}
};

template <>
inline
void CoordSpan<AlignBase::Axis::VERT>::copyFrom(const BBoxSVG & bbox){
	if (bbox.isDefined()){
		pos  = bbox.y;
		span = bbox.height;
	}
};


template <>
inline
void CoordSpan<AlignBase::Axis::HORZ>::copyFrom(const NodeSVG & node){
	const BBoxSVG & bbox = node.getBoundingBox();
	if (bbox.isDefined()){
		CoordSpan<AlignBase::Axis::HORZ>::copyFrom(bbox);
		pos += node.transform.translate.x;
	}
};

template <>
inline
void CoordSpan<AlignBase::Axis::VERT>::copyFrom(const NodeSVG & node){
	const BBoxSVG & bbox = node.getBoundingBox();
	if (bbox.isDefined()){
		CoordSpan<AlignBase::Axis::VERT>::copyFrom(bbox);
		pos += node.transform.translate.y;
	}
};

template <AlignBase::Axis AX>
std::ostream & operator<<(std::ostream & ostr, const CoordSpan<AX> &span){
	ostr << "span" << AX << "=" << span.pos << "(+" << span.pos <<  ')';
	return ostr;
}


template <AlignBase::Axis AX>
void handleAlign(TreeSVG & object, NodeSVG & node, CoordSpan<AX> & span){

	Logger mout(__FILE__, __FUNCTION__);

	const AnchorElem & anchorElem = node.getMyAlignAnchor<AX>().isSet() ? node.getMyAlignAnchor<AX>() : object->getDefaultAlignAnchor<AX>();

	if (anchorElem.isSet()){
		if (anchorElem.isExtensive()){
			span.copyFrom(object->getBoundingBox()); // translate should be anyway 0,0 (now, still)
		}
		else if (anchorElem.isSpecific()){
			if (object.hasChild(anchorElem)){
				const NodeSVG & anchorNode = object[anchorElem];
				if (&anchorNode == &node){ // IMPORTANT!
					mout.pending<LOG_NOTICE>("self-anchoring skipped for [", anchorElem,"] = ", node);
				}
				else {
					span.copyFrom(anchorNode);
				}
			}
			else {
				mout.warn("non-existing anchor elem [", anchorElem,"] requested for node ", node);
			}
		}
		else {
			mout.error("program error - illegal anchor [", anchorElem, "] requested for node  [", node);
		}
		mout.note(AX, " anchor '", anchorElem, "' for ", node.getTag(), ':', node.getId());
	}
	else {
		mout.note("No ", AX, " anchor for ", node.getTag(), ':', node.getId());
	}

	if (span.isDefined()){
		mout.note("Aligning ", AX, " span, ", span, " anchor for ", node.getTag(), ':', node.getId());
		TreeUtilsSVG::realignObject(node, span);
	}

}

/**
 *   Aligns any object which isAligned()
 *   Does not check classes.
 */
void TreeUtilsSVG::superAlignNEW(TreeSVG & object){ //, const Point2D<svg::coord_t> & offset){ // offsetInit

	Logger mout(__FILE__, __FUNCTION__);

	// Quick check
	/*
	if (object->isAbstract()){
		return;
	}
	*/

	if (!object->typeIs(svg::SVG, svg::GROUP)){
		return;
	}


	// mout.attention("ACCEPT:", object->getTag());
	mout.special<LOG_DEBUG>(__FUNCTION__, " start: ", object.data); //, object->getId(), " -> ", object->getBoundingBox());

	// Host element's |STACK_LAYOUT| bbox, to be updated below
	// Incrementally growing extent: not only width/height but also x,y.
	BBoxSVG & compoundBBox = object->getBoundingBox();
	compoundBBox.reset();

	mout.pending<LOG_NOTICE>("start: extensiveBBox ", compoundBBox, " obj=", object.data);

	// const AnchorElem & defaultAnchorHorz = object->getAlignAnchorDefaultHorz();
	// const AnchorElem & defaultAnchorVert = object->getAlignAnchorDefaultVert();

	CoordSpan<AlignBase::Axis::HORZ> anchorSpanHorz;
	// anchorSpanHorz.pos  = 0;
	// anchorSpanHorz.span = 0;

	CoordSpan<AlignBase::Axis::VERT> anchorSpanVert;
	// anchorSpanVert.pos  = 0;
	// anchorSpanVert.span = 0;

	// Main loop (other than recursion)
	for (TreeSVG::pair_t & entry: object){

		NodeSVG & node = entry.second.data;

		if (node.isAbstract()){ // Non-graphic
			continue;
		}

		// Depth first; complete sub-objects first.

		if (node.hasClass(LayoutSVG::COMPOUND)){
			detectBoxNEW(entry.second, true); // only
		}
		else {
			superAlignNEW(entry.second);
		}


		// At least some align instruction has been set. (This could be sufficient, replacing above test?)
		if (node.isAligned()){
			handleAlign(object, node, anchorSpanHorz);
			handleAlign(object, node, anchorSpanVert);
		}

		// Save the last one
		anchorSpanHorz.copyFrom(node);
		anchorSpanVert.copyFrom(node);

		mout.accept<LOG_NOTICE>("completed ", node.getTag(),'=', node.getId(), ": ",
				"H:",  anchorSpanHorz.pos, ',', anchorSpanHorz.span, ", ",
				"V:",  anchorSpanVert.pos, ',', anchorSpanVert.span);

		if (!node.hasClass(LayoutSVG::FLOAT)){
			compoundBBox.expandHorz(anchorSpanHorz.pos);
			compoundBBox.expandHorz(anchorSpanHorz.pos + anchorSpanHorz.span);
			compoundBBox.expandVert(anchorSpanVert.pos);
			compoundBBox.expandVert(anchorSpanVert.pos + anchorSpanVert.span);
			/// mout.accept<LOG_NOTICE>("Expanding ... ", bbox, " now, after: ", node.data);
		}

	}
	mout.accept<LOG_NOTICE>("compoundBBox   end:", object->getBoundingBox(), " obj=", object.data);

	// Debugging
	/*
	for (TreeSVG::pair_t & entry: object){

		NodeSVG & node = entry.second.data;
		if (node.typeIs(svg::GROUP)){ // Non-graphic

			CoordSpan<AlignBase::Axis::HORZ> cx;
			cx.copyFrom(node);
			CoordSpan<AlignBase::Axis::VERT> cy;
			cy.copyFrom(node);

			drain::image::NodeSVG & debugRect = entry.second["debugRect"](svg::RECT);
			// debugRect.addClass("DEBUG");
			debugRect.addClass("DEBUG", LayoutSVG::FLOAT);

			BBoxSVG & b =  debugRect.getBoundingBox();
			b.setLocation(cx.pos, cy.pos);
			b.setArea(cx.span, cy.span);
		}

	}
	*/
	/*
	*/
	// debugRect.set("x", b.x);
	// debugRect.set("y", b.y);
	// debugRect.setFrame(b.getFrame());

}


/**
 *   Pick dx = bbox.x
 *   translate.x = anchor.x -dx
 *
 *
 */
// static
template <>
void TreeUtilsSVG::realignObject(NodeSVG & node, const CoordSpan<AlignBase::Axis::HORZ> & span){
// void TreeUtilsSVG::realignObjectHorzNEW(NodeSVG & node, const Box<svg::coord_t> & anchorBoxHorz){

	// return;

	Logger mout(__FILE__, __FUNCTION__);

	// TODO/FIX: currently TEXT not handled at all (this method not invoked)
	const bool IS_TEXT = node.typeIs(svg::TEXT);

	Box<svg::coord_t> & obox = node.getBoundingBox();

	svg::coord_t coord = 0;

	AlignBase::Pos alignLoc;

	// STEP 1: ANCHOR

	mout.debug("Adjusting location (", ") with ANCHOR's ref point");

	switch (alignLoc = node.getAlign(AlignSVG::Owner::ANCHOR, AlignBase::Axis::HORZ)){
	case AlignBase::Pos::MIN:
		coord = span.pos;
		break;
	case AlignBase::Pos::MID:
		coord = span.pos + span.span/2;
		break;
	case AlignBase::Pos::MAX:
		coord = span.pos + span.span;
		break;
	case AlignBase::Pos::FILL:
		// Maybe ok, since GROUP can be an anchor but also
		// mout.suspicious<LOG_WARNING>("Alignment:: ANCHOR has fill request: HORZ FILL");
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

	// STEP 2: OBJECT

	switch (alignLoc = node.getAlign(AlignSVG::Owner::OBJECT, AlignBase::Axis::HORZ)){
	case AlignBase::Pos::MIN:
		if (IS_TEXT){
			node.setStyle(TEXT_ANCHOR, "start");
			node.transform.translate.set(+2.0 * node.getMargin());
		}
		break;
	case AlignBase::Pos::MID:
		if (IS_TEXT){
			node.setStyle(TEXT_ANCHOR, "middle");
		}
		else {
			coord -= obox.width/2;
		}
		break;
	case AlignBase::Pos::MAX:
		if (IS_TEXT){
			node.setStyle(TEXT_ANCHOR, "end");
			node.transform.translate.set(-2.0 * node.getMargin());
		}
		else {
			coord -= obox.width;
		}
		break;
	case AlignBase::Pos::FILL:
		//mout.experimental("STRETCHING..." );mout.experimental("STRETCHING..." );
		mout.experimental<LOG_DEBUG>("FILL:ing horz: ",   obox, " width -> ", span.span);
		coord = span.pos;
		obox.setWidth(span.span);

		break;
	case AlignBase::Pos::UNDEFINED_POS: // or absolute
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::OBJECT, '/', AlignBase::Axis::HORZ, pos);
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG::Pos ", alignLoc);
	}

	if (!IS_TEXT){
		node.transform.translate.x = coord;
	}
	// mout.attention("Alignment::OBJECT-HORZ ", pos);
	// mout.debug("Alignment::Pos: ", AlignSVG::Owner::OBJECT, '/', axis, '=', alignLoc);
}

/**
 *  \param anchorSpan - width or height of the achore rectangle.
 */
template <>
void TreeUtilsSVG::realignObject(NodeSVG & node, const CoordSpan<AlignBase::Axis::VERT> & span){
// void TreeUtilsSVG::realignObjectVertNEW(NodeSVG & node, const Box<svg::coord_t> & anchorBoxVert){

	Logger mout(__FILE__, __FUNCTION__);

	const bool IS_TEXT = node.typeIs(svg::TEXT);

	svg::coord_t coord = 0;

	AlignBase::Pos alignLoc;

	switch (alignLoc = node.getAlign(AlignSVG::Owner::ANCHOR, AlignBase::Axis::VERT)){
	case AlignBase::Pos::MIN:
		coord = span.pos;
		break;
	case AlignBase::Pos::MID:
		coord = span.pos + span.span/2.0;
		break;
	case AlignBase::Pos::MAX:
		coord = span.pos + span.span;
		break;
	case AlignBase::Pos::FILL:
		mout.suspicious<LOG_NOTICE>("Alignment:: ANCHOR has fill request: HORZ FILL");
		break;
	case AlignBase::Pos::UNDEFINED_POS:  // -> consider MID or some absolute value, or margin. Or error:
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', AlignBase::Axis::HORZ, '=', pos);
		break;
	default:
		// assert undefined value.
		mout.unimplemented<LOG_ERR>("Alignment::Pos: ", (int)alignLoc);
	}
	// mout.debug("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', axis, '=', alignLoc);
	// mout.special("Vertical adjust: after anchor:", alignLoc, " coord=", coord, " based on abox:", anchorBoxVert);
	// mout.debug("Adjusting ", axis, " pos (", alignLoc, ") with OBJECT's own reference point");

	static const std::string TEXT_ANCHOR("text-anchor");

	Box<svg::coord_t> & obox = node.getBoundingBox();

	if (IS_TEXT){
		node.transform.translate.y += (node.getHeight()-node.getMargin());
	}

	switch (alignLoc = node.getAlign(AlignSVG::Owner::OBJECT, AlignBase::Axis::VERT)){
	case AlignBase::Pos::MIN:
		break;
	case AlignBase::Pos::MID:
		coord -= 0.5*obox.height;
		break;
	case AlignBase::Pos::MAX:
		coord -= obox.height;
		break;
	case AlignBase::Pos::FILL:
		// mout.experimental("STRETCHING..." );mout.experimental("STRETCHING..." );
		// mout.experimental<LOG_DEBUG>("FILL:ing vert: ", obox, " height ");
		coord = span.pos;
		obox.setHeight(span.span);
		if (IS_TEXT){
			mout.warn("FILL not applicable for TEXT elem - ", node);
		}
		break;
	case AlignBase::Pos::UNDEFINED_POS: // or absolute
		mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", alignLoc, " for ", AlignSVG::Owner::OBJECT, '/', AlignBase::Axis::VERT);
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG::Pos ", alignLoc);
	}

	if (IS_TEXT){
		// NOT HANDLED!
	}
	node.transform.translate.y = coord;

}




// ---------------------------------------------------

// Consider XMl Visitor... no
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
		if (box.empty()){ // TODO: check/redefine empty
			box = elem->getBoundingBox();
		}
		else {
			// drain::Logger mout(__FILE__, __FUNCTION__);
			// mout.pending("Updating bbox ", box, " with ", elem->getBoundingBox());
			box.expand(elem->getBoundingBox());
		}
	}

	return (box.getWidth() != 0.0) && (box.getHeight() != 0.0);

}



void TreeUtilsSVG::finalizeBoundingBox(TreeSVG & svg){

	drain::Logger mout(__FILE__, __FUNCTION__);


	drain::image::BBoxSVG bb;
	computeBoundingBox(svg, bb);

	/*  FIX!
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


void TreeUtilsSVG::realignObjectHorz(TreeSVG & object, const Box<svg::coord_t> & anchorBoxHorz){


	Logger mout(__FILE__, __FUNCTION__);

	const bool IS_TEXT = object->typeIs(svg::TEXT);

	Box<svg::coord_t> & obox = object->getBoundingBox();

	svg::coord_t coord = 0;

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
		// Maybe ok, since GROUP can be an anchor but also
		// mout.suspicious<LOG_WARNING>("Alignment:: ANCHOR has fill request: HORZ FILL");
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
			object->transform.translate.set(+2.0 * object->getMargin());
		}
		break;
	case AlignBase::Pos::MID:
		if (IS_TEXT){
			object->setStyle(TEXT_ANCHOR, "middle");
		}
		else {
			coord -= obox.width/2;
		}
		break;
	case AlignBase::Pos::MAX:
		if (IS_TEXT){
			object->setStyle(TEXT_ANCHOR, "end");
			object->transform.translate.set(-2.0 * object->getMargin());
		}
		else {
			coord -= obox.width;
		}
		break;
	case AlignBase::Pos::FILL:
		//mout.experimental("STRETCHING..." );mout.experimental("STRETCHING..." );
		mout.experimental<LOG_DEBUG>("FILL:ing horz: ",   obox, " width -> ", anchorBoxHorz.width);
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

	TranslatorSVG translator(coord - object->getBoundingBox().x, 0.0f);
	TreeUtils::traverse(translator, object);

}

/**
 *  \param anchorSpan - width or height of the achore rectangle.
 */
void TreeUtilsSVG::realignObjectVert(TreeSVG & object, const Box<svg::coord_t> & anchorBoxVert){


	Logger mout(__FILE__, __FUNCTION__);

	const bool IS_TEXT = object->typeIs(svg::TEXT);

	svg::coord_t coord = 0;

	AlignBase::Pos alignLoc;

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
		mout.suspicious<LOG_NOTICE>("Alignment:: ANCHOR has fill request: HORZ FILL");
		break;
	case AlignBase::Pos::UNDEFINED_POS:  // -> consider MID or some absolute value, or margin. Or error:
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', AlignBase::Axis::HORZ, '=', pos);
		break;
	default:
		// assert undefined value.
		mout.unimplemented<LOG_ERR>("Alignment::Pos: ", (int)alignLoc);
	}
	// mout.debug("Alignment::Pos: ", AlignSVG::Owner::ANCHOR, '/', axis, '=', alignLoc);

	// mout.special("Vertical adjust: after anchor:", alignLoc, " coord=", coord, " based on abox:", anchorBoxVert);

	// mout.debug("Adjusting ", axis, " pos (", alignLoc, ") with OBJECT's own reference point");

	static const std::string TEXT_ANCHOR("text-anchor");

	Box<svg::coord_t> & obox = object->getBoundingBox();

	if (IS_TEXT){
		object->transform.translate[1] += (object->getHeight()-object->getMargin());
	}

	switch (alignLoc = object->getAlign(AlignSVG::Owner::OBJECT, AlignBase::Axis::VERT)){
	case AlignBase::Pos::MIN:
		break;
	case AlignBase::Pos::MID:
		coord -= 0.5*obox.height;
		break;
	case AlignBase::Pos::MAX:
		coord -= obox.height;
		break;
	case AlignBase::Pos::FILL:
		// mout.experimental("STRETCHING..." );mout.experimental("STRETCHING..." );
		// mout.experimental<LOG_DEBUG>("FILL:ing vert: ", obox, " height ");
		coord = anchorBoxVert.y;
		obox.setHeight(anchorBoxVert.height);
		if (IS_TEXT){
			mout.warn("FILL not applicable for TEXT elem - ", object.data);
		}
		break;
	case AlignBase::Pos::UNDEFINED_POS: // or absolute
		mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", alignLoc, " for ", AlignSVG::Owner::OBJECT, '/', AlignBase::Axis::VERT);
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG::Pos ", alignLoc);
	}


	TranslatorSVG translator(0.0f, coord - object->getBoundingBox().y);
	TreeUtils::traverse(translator, object);
	/*
	if (object->typeIs(svg::GROUP)){
		object->transform.translate[1] += coord - object->getBoundingBox().y;
	}
	else {
	}
	*/

}


/**
 *  \param anchorSpan - width or height of the achore rectangle.
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

	// mout.attention("ACCEPT:", object->getTag());
	mout.special<LOG_DEBUG>(__FUNCTION__, " start: ", object.data); //, object->getId(), " -> ", object->getBoundingBox());

	// Depth-first
	for (TreeSVG::pair_t & entry: object){
		superAlign(entry.second, orientation, direction); // , offset);
	}

	// mout.attention("ACCEPT:", object->getTag());
	mout.special<LOG_DEBUG>(__FUNCTION__, " handle: ", object.data); //, object->getId(), " -> ", object->getBoundingBox());


	/// If main orientation (inside containers) is HORZ, stack containers VERT. And vice versa.
	if (object->hasClass(LayoutSVG::STACK_LAYOUT)){
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

	/* NOTE! Current policy is (potentially) ambiguous as group anchor is also gthe group's own specific anchor */

	const AlignAdapterSVG::anchor_t & groupAnchorHorz = object->getDefaultAlignAnchor<AlignBase::HORZ>();
	const bool GROUP_ANCHOR_HORZ = object->typeIs(svg::GROUP) && !groupAnchorHorz.empty(); // || ANCHOR_HORZ_CUMULATIVE);

	if (GROUP_ANCHOR_HORZ && (groupAnchorHorz == "*")){
		mout.warn("HORZ Group anchor '*' actually means group's own anchor.");
	}

	const AlignAdapterSVG::anchor_t & groupAnchorVert = object->getDefaultAlignAnchor<AlignBase::VERT>();
	const bool GROUP_ANCHOR_VERT  = object->typeIs(svg::GROUP) && !groupAnchorVert.empty(); //  || ANCHOR_VERT_CUMULATIVE);

	if (GROUP_ANCHOR_VERT && (groupAnchorVert == "*")){
		mout.warn("VERT Group anchor '*' actually means group's own anchor.");
	}


	// Incrementally growing extent (width/height)
	BBoxSVG bbox;
	BBoxSVG *bboxAnchorHorz = &bbox;
	BBoxSVG *bboxAnchorVert = &bbox;

	if (GROUP_ANCHOR_HORZ){
		bboxAnchorHorz = & object[groupAnchorHorz]->getBoundingBox();
		objectBBox.expand(*bboxAnchorHorz);
		// or ASSIGN?
	}


	if (GROUP_ANCHOR_VERT){
		bboxAnchorVert = & object[groupAnchorVert]->getBoundingBox();
		objectBBox.expand(*bboxAnchorVert);
		// or ASSIGN?
	}

	// bboxAnchorHorz->setLocation(0, 0); // replaces offset?
	// bboxAnchorVert->setLocation(0, 0); // replaces offset?

	for (TreeSVG::pair_t & entry: object){

		NodeSVG & node = entry.second; // implicit cast

		if (node.isAbstract()){
			continue;
		}

		if (!node.isAligned()){
			if (node.hasClass(LayoutSVG::FLOAT)){
				node.setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER); // check
			}
			else {
				if (orientation == AlignBase::Axis::HORZ){
					node.setAlign(direction==LayoutSVG::Direction::INCR ? AlignSVG::RIGHT : AlignSVG::LEFT, AlignSVG::OUTSIDE);
					node.setAlign(AlignSVG::TOP);
				}
				else {
					node.setAlign(AlignSVG::LEFT);
					node.setAlign(direction==LayoutSVG::Direction::INCR ? AlignSVG::BOTTOM : AlignSVG::TOP, AlignSVG::OUTSIDE);
				}
			}
		}

		// If the element has specific (sibling) anchor, use it.

		// svg::coord_t w = node.getWidth();

		// mout.pending<LOG_NOTICE>(object->getTag(), " object BBOX: ", objectBBox);

		const AlignAdapterSVG::anchor_t & ah = node.getMyAlignAnchor<AlignBase::HORZ>();
		if (ah == "*"){
			// mout.special<LOG_NOTICE>("WIDE horz align for ", entry.first, " => ", entry.second.data);
			TreeUtilsSVG::realignObjectHorz(entry.second, objectBBox); // this far
		}
		else if (object.hasChild(ah)){ // Specific horz anchor (sibling)
			TreeUtilsSVG::realignObjectHorz(entry.second, object[ah]->getBoundingBox());
		}
		else if (entry.first != groupAnchorHorz){
			TreeUtilsSVG::realignObjectHorz(entry.second, *bboxAnchorHorz);
		}

		/*
		mout.reject<LOG_NOTICE>(object->getTag(), " object BBOX: ", objectBBox);
		if (w != node.getWidth()){
			mout.reject<LOG_NOTICE>("WIDTH changed ", w, "->", node.getWidth(), " for ", entry.second.data);
		}
		*/


		const AlignAdapterSVG::anchor_t & av = node.getMyAlignAnchor<AlignBase::HORZ>();
		if (av == "*"){
			// mout.special<LOG_NOTICE>("WIDE vert align for ", entry.first, " => ", entry.second.data);
			TreeUtilsSVG::realignObjectVert(entry.second, objectBBox); // this far
		}
		else if (object.hasChild(av)){ // Specific vert anchor (sibling)
			TreeUtilsSVG::realignObjectVert(entry.second, object[av]->getBoundingBox());
		}
		else if (entry.first != groupAnchorVert){
			TreeUtilsSVG::realignObjectVert(entry.second, *bboxAnchorVert);
		}


		if (!node.hasClass(LayoutSVG::FLOAT)){

			//mout.special<LOG_WARNING>("Expanding ", objectBBox.getFrame(), " of ", object->getId(), " with ", bbox.getFrame());
			bbox = node.getBoundingBox(); // Notice: copy
			if (&bbox == &objectBBox){
				mout.suspicious("Updating BBOX with itself");
			}
			objectBBox.expand(bbox);
			/// mout.special<LOG_WARNING>("Resulting BBOX=", objectBBox, " me: \t", object.data);

			if (!GROUP_ANCHOR_HORZ){
				mout.experimental<LOG_INFO>("Jumping HORZ anchor to: /", entry.first, "/ <", node.getTag(), "> ", node.getId());
				bboxAnchorHorz = & node.getBoundingBox();
			}

			if (!GROUP_ANCHOR_VERT){
				// Jumping anchor
				mout.experimental<LOG_INFO>("Jumping VERT anchor to: /", entry.first, "/ <", node.getTag(), "> ", node.getId());
				bboxAnchorVert = & node.getBoundingBox();
			}

			mout.accept<LOG_DEBUG>("Expanding ... ", objectBBox, " horz:", *bboxAnchorHorz, " vert:", *bboxAnchorVert);
			/// mout.accept<LOG_NOTICE>("Expanding ... ", bbox, " now, after: ", entry.second.data);

		}


	}

}


/**  Iteratively traverses elements of ALIGN_GROUP, and (re)aligns elements inside them as panels.
 *
 */
void TreeUtilsSVG::translateAll(TreeSVG & object, const Point2D<svg::coord_t> & offset){

	if (object->isAbstract()){
		return;
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
	//if (TreeUtilsSVG::isAbstract((svg::tag_t)node->getType())){ // skip TITLE, DESC etc.
	if (node->isAbstract()){ // skip TITLE, DESC etc.
		return 1;
	}
	else {
		BBoxSVG & bbox = node->getBoundingBox();
		bbox.x += offset.x;
		bbox.y += offset.y;
		return 0;
	}
}


int BBoxRetrieverSVG::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path) {
	TreeSVG & node = tree(path);
	if (node->typeIs(svg::SVG, svg::GROUP)){
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
		const std::string & imagePath = imageNode->getUrl(); //imageNode->get("xlink:href");

		if (drain::StringTools::startsWith(imagePath, dir)){
			// Strip directory part from the imagePath, replace with prefix
			imageNode->setUrl(prefix + imagePath.substr(dir.size()));
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
