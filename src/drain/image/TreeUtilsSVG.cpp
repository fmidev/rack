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


#include <drain/StringBuilder.h>
#include <drain/util/Output.h> // Debugging
#include <drain/util/Point.h>

#include "TransformSVG.h"
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



void TreeUtilsSVG::detectBox(drain::image::TreeSVG & group, bool debug){

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
				detectBox(entry.second, debug); // returns if not SVG or G

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
					// SVG does not support computing text object length"
					// Only height is taken into account
					b.expandVert(node.getBoundingBox());
					// Height is real, relates to row height.
					// Width is the margin only, yet pushed "bbox" out a bit.
					// Practically, TEXT is assumed to be left/right/center aligned inside a (virtual or RECT) rectangle.
					// b.expand(node.getBoundingBox());
					// b.expand(node.getBoundingBox().getLocation());
					// mout.accept<LOG_NOTICE>("BBOX after svg::TEXT: ", b);
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

/*
void surround(TreeSVG & group, const TreeSVG::path_elem_t & childKey){

	static const TreeSVG::path_elem_t cageElem("cage");
	static const TreeSVG::path_elem_t dummyElem("dummy");


	TreeSVG & cage  = group[cageElem](svg::GROUP);
	cage->addClass("cage");
	TreeSVG & dummy = cage[dummyElem](svg::GROUP);
	dummy->addClass("dungeon");
	// dummy->setText("Hello!");
	dummy.swap(group[childKey]);

}
*/



/// Set instructions, flipping axis in nesting groups of type STACK_LAYOUT
/**
 *  MOVE
 */
void TreeUtilsSVG::addStackLayout(TreeSVG & object, AlignBase::Axis orientation, LayoutSVG::Direction direction, unsigned short depth){ //, const Point2D<svg::coord_t> & offset){ // offsetInit

	Logger mout(__FILE__, __FUNCTION__);

	NodeSVG & node = object.data;



	if (!node.typeIs(svg::GROUP, svg::SVG, svg::RECT, svg::IMAGE, svg::TEXT)){
		return;
	}

	// mout.attention("ACCEPT:", object->getTag());
	mout.special<LOG_DEBUG>(__FUNCTION__, " start: ", node); //, object->getId(), " -> ", object->getBoundingBox());

	if (node.hasClass(LayoutSVG::INDEPENDENT)){
		if (!node.isAligned()){
			// somewhat atribitrary
			node.setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER); // check
		}
	}
	else if (node.isAligned()){
		//mout.special("Overriding Align for: ", object.data);
	}
	else if (!node.hasClass(LayoutSVG::ADAPTER)){
		setStackLayout(node, orientation, direction);
	}


	// Recursion
	if (node.typeIs(svg::SVG, svg::GROUP)){

		if (node.hasClass(LayoutSVG::STACK_LAYOUT)){
			orientation = AlignBase::flip(orientation);
			// mout.special("flipped orientation to ", orientation, " for children of ", node);
			// Also mark and increment level:
			// node.addClass(StringBuilder<'_'>("STACK", depth).str()); // future option...
			++depth;
		}

		for (TreeSVG::pair_t & entry: object){
			addStackLayout(entry.second, orientation, direction, depth);
			/*
			if (entry.second->typeIs(svg::IMAGE)){
				// steal DESC and TITLE
				surround(object, entry.first);
			}
			*/

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
	ostr << "span" << AX << "=" << span.pos << "(+" << span.span <<  ')';
	return ostr;
}


/*
std::ostream & operator<<(std::ostream & ostr, const NodePrinter &np){
	ostr << "span" << AX << "=" << span.pos << "(+" << span.pos <<  ')';
	return ostr;
}
*/
template <AlignBase::Axis AX>
void expandBBox(BBoxSVG & bbox, CoordSpan<AX> & anchorSpan){
	Logger(__FILE__, __FUNCTION__).error("Unimplemented method");
}

template <>
void expandBBox(BBoxSVG & bbox, CoordSpan<AlignBase::Axis::HORZ> & anchorSpan){
	bbox.expandHorz(anchorSpan.pos);
	bbox.expandHorz(anchorSpan.pos + anchorSpan.span);
}

template <>
void expandBBox(BBoxSVG & bbox, CoordSpan<AlignBase::Axis::VERT> & anchorSpan){
	bbox.expandVert(anchorSpan.pos);
	bbox.expandVert(anchorSpan.pos + anchorSpan.span);
}

/**
   Semantics:

   if myAnchor is set, use it.
   if myAnchor is unset, use group anchor; it servers as a default anchor
   if also groupAnchor is unset -> use previous object as anchor.

   GroupAnchor (default anchor):
	    DEFAULT(=UNSET): use previous object
		NONE: use nothing, don't align
		PREVIOUS: use previous (unneeded, this is the default for group)
		COLLECTIVE: use compound bounding bbox

		MyAnchor (specific anchor, always overrides)
		DEFAULT(=UNSET) -> use GroupAnchor
		NONE: don't align me! (raise error if Align set?)
		PREVIOUS: use previous object
		COLLECTIVE: use compound bounding box

		SPECIAL: both DEFAULT: use PREVIOUS

 */
template <AlignBase::Axis AX>
const AnchorElem & getAnchorElem(TreeSVG & group, NodeSVG & node){

	const AnchorElem & myAnchor = node.getMyAlignAnchor<AX>();

	if (myAnchor.isSet()){
		return myAnchor;
	}
	else {
		const AnchorElem & defaultAnchor = group->getDefaultAlignAnchor<AX>();
		if (defaultAnchor.isSet()){
			return defaultAnchor;
		}
		else {
			// grand default
			return drain::EnumDict<AnchorElem::Anchor>::getKey(AnchorElem::PREVIOUS);
		}
	}

}

// To be under TreeUtilsSVG
/**
 *   Given a node (requesting Alignment), re-locate it by adjusting transform.x and .y
 *
 *   The anchor always resides in the same group.
 */
template <AlignBase::Axis AX>
void TreeUtilsSVG::adjustLocation(TreeSVG & group, NodeSVG & node, CoordSpan<AX> anchorSpan){ // NOTE: COPY

	Logger mout(__FILE__, __FUNCTION__);


	const AnchorElem & anchorElem = node.getMyAlignAnchor<AX>().isSet() ? node.getMyAlignAnchor<AX>() : group->getDefaultAlignAnchor<AX>();

	std::string id = NodePrinter(node).str();
	// mout.note("NODE: ", id);

	if (anchorElem.isPrevious() || !anchorElem.isSet()){
		// using previous, given through argument anchorSpan
	}
	else if (anchorElem.isNone()){

		if (node.isAligned()){
			mout.suspicious("contradiction: anchor(", AX, ") =[", anchorElem,"] for element requesting Alignment: ", id, " ALIGN:", node.getAlignStr());
		}
		mout.experimental("SKIPPING anchor(", AX, ") =[", anchorElem,"] for ", id);
		//return;
	}
	else if (anchorElem.isCollective()){
		BBoxSVG b;
		getAdjustedBBox(group.data, b);
		anchorSpan.copyFrom(b); // translate should be anyway 0,0 (now, still)
		//anchorSpan.copyFrom(node); // translate should be anyway 0,0 (now, still)
	}
	else if (anchorElem.isSpecific()){
		if (group.hasChild(anchorElem)){
			const NodeSVG & anchorNode = group[anchorElem];
			if (&anchorNode == &node){ // IMPORTANT!
				mout.debug("self-anchoring (", AX, ") skipped for [", anchorElem,"] = ", id);
			}
			else {
				BBoxSVG b;
				getAdjustedBBox(anchorNode, b);
				anchorSpan.copyFrom(b);
				//anchorSpan.copyFrom(anchorNode);
			}
		}
		else {
			mout.warn("non-existing anchor(", AX, ") element [", anchorElem,"] requested for node ", id);
			for (const auto & entry: group.getChildren()){
				const NodeSVG & n = entry.second.data;
				if ((&n != &node) && !n.isAbstract()){
					mout.advice("  candidate: [", entry.first, "]  = ", NodePrinter(n).str());
				}
			}
		}
	}
	else {
		mout.error("program error - illegal (", AX, ") anchor [", anchorElem, "] requested for ", id);
	}

	if (anchorSpan.isDefined()){
		if (!anchorElem.isNone()){ // unless exclusively denied.
			// mout.note("Using span(", AX, ") = ", anchorSpan, " for Aligning: ", id);
			// mout.note("Using ", anchorSpan, " of '",  anchorElem, "' for Aligning: ", id);
			realignObject(node, anchorSpan);
			// mout.accept<LOG_NOTICE>("Current COMPOUND bbox: ", group->getBoundingBox());
			// mout.accept<LOG_NOTICE>("Current span ", AX, ": ", anchorSpan);
		}
	}
	else {
		// When should this happen?
	}

}

/*
inline
std::ostream & operator<<(std::ostream & ostr, const drain::image::TransformSVG & tr){
	tr.toStream(ostr);
	return ostr;
}
*/

/**
 *   Aligns each object which isAligned()
 *
 */
void TreeUtilsSVG::superAlign(TreeSVG & group){ //, const Point2D<svg::coord_t> & offset){ // offsetInit

	Logger mout(__FILE__, __FUNCTION__);


	if (!group->typeIs(svg::SVG, svg::GROUP)){
		return;
	}


	// mout.attention("ACCEPT:", object->getTag());
	// mout.special<LOG_DEBUG>(__FUNCTION__, " start: ", group.data); //, object->getId(), " -> ", object->getBoundingBox());


	// mout.pending<LOG_NOTICE>("start: extensiveBBox ", compoundBBox, " obj=", group.data);

	// Starting point (indeed): origin.
	// Basic idea: Stack-align objects on a "blanche", and finally adjust group if needed.


	/// Ideally, each object (graphic element or compound object) should:
	/**  - return a bounding box, or more specifically (width,height) that is, bbox.getFrame())
	 *   - obey relocating
	 *   -> RECT, IMAGE, etc. true-(x,y)-upper-left-origin: change (x,y)
	 *   -> others (esp. groups: change translate(x,y)
	 *
	 *   When should an object have both?
	 *
	 */

	// std::set<TreeSVG::path_elem_t>  aligned;
	// TreeSVG::path_elem_t anchorHorzRequest;
	// TreeSVG::path_elem_t anchorVertRequest;

	// std::list<TreeSVG::path_elem_t> alignWaitingForCollectiveBBox;


	for (TreeSVG::pair_t & entry: group){

		NodeSVG & node = entry.second.data;

		if (node.isAbstract()){ // Non-graphic
			continue;
		}

		if (node.hasClass(LayoutSVG::FIXED)){ // consider joining this with COMPOUND?
			// mout.attention("fixed, ok ", node);
		}
		else if (!node.hasClass(LayoutSVG::COMPOUND)){
			// First, align the children of this node, recursively
			superAlign(entry.second);
		}
		else { // COMPOUND - or require detectBox being already calculated?
			// Unused at the moment?
			mout.attention("detecting bbox of ", node);
			detectBox(entry.second, true); // only
		}
	}

	// Separated to two loops for future option: @NEXT and @COLLECTIVE_FINAL (bbox) – using several iterations if needed.

	// Host element's |STACK_LAYOUT| bbox, to be updated below
	// Incrementally growing extent: not only width/height but also x,y.
	BBoxSVG & compoundBBox = group->getBoundingBox();
	compoundBBox.reset();

	// Future option / Iterative loop starts here

	// Initial anchor box is actually just a point (0,0)
	CoordSpan<AlignBase::Axis::HORZ> anchorSpanHorz(0, 0);
	CoordSpan<AlignBase::Axis::VERT> anchorSpanVert(0, 0);

	for (TreeSVG::pair_t & entry: group){

		NodeSVG & node = entry.second.data;

		if (node.isAbstract()){ // Non-graphic
			continue;
		}

		if (node.hasClass(LayoutSVG::FIXED)){
			// skip moving/translating
		}
		else if (node.isAligned()){
			// if (node.getMyAlignAnchor<AlignBase::HORZ>().)
			adjustLocation(group, node, anchorSpanHorz);
			// if false (has store HORZ request) skip?
			adjustLocation(group, node, anchorSpanVert);
		}

		/*
		if (!anchorHorzRequest.empty()){
			NodeSVG & n = group[alignWaitingForNext];
			alignWaitingForNext.clear();
		}
		*/

		// Update spanHorz and spanVert

		if (node.hasClass(LayoutSVG::ADAPTER)){
			// This is a riddle... consider better nodePrinter with aligns.
			///mout.accept<LOG_NOTICE>(" adapt1: ", NodePrinter(node).str() );
			BBoxSVG & bn =  node.getBoundingBox();
			node.transform.setTranslate(-bn.x, -bn.y);
			bn.setLocation(0.0, 0.0);
			// Save the last one
			anchorSpanHorz.copyFrom(bn); // NOT adjusted by transform.x
			anchorSpanVert.copyFrom(bn); // NOT adjusted by transform.y
			// anchorSpanHorz.copyFrom(node); // adjusted by transform.x
			// anchorSpanVert.copyFrom(node); // adjusted by transform.y
			// mout.accept<LOG_NOTICE>(" adapt2: ", NodePrinter(node).str() );
		}
		else {
			// Save the last one
			anchorSpanHorz.copyFrom(node); // adjusted by transform.x
			anchorSpanVert.copyFrom(node); // adjusted by transform.y
		}


		if (!node.hasClass(LayoutSVG::INDEPENDENT)){
			//aligned.insert(entry.first);
		}


		/*
		// mout.accept<LOG_NOTICE>("NOW ", NodePrinter(node).str(), " with ", anchorSpanHorz, " and ", anchorSpanVert, " are expanding BBOX of ", NodePrinter(group).str());
		mout.accept<LOG_NOTICE>("completed ", node.getTag(),'=', node.getId(), ": ",
				"H:",  anchorSpanHorz.pos, ',', anchorSpanHorz.span, ", ",
				"V:",  anchorSpanVert.pos, ',', anchorSpanVert.span);
		*/

		if (!node.hasClass(LayoutSVG::INEFFECTIVE)){
			//compoundBBox.expand(node.getBoundingBox()); // this should be valid :-(
			compoundBBox.expandHorz(anchorSpanHorz.pos);
			compoundBBox.expandHorz(anchorSpanHorz.pos + anchorSpanHorz.span);
			compoundBBox.expandVert(anchorSpanVert.pos);
			compoundBBox.expandVert(anchorSpanVert.pos + anchorSpanVert.span);
			// mout.accept<LOG_NOTICE>("COMPOUND BBOX, now ", compoundBBox, " now, after: ", NodePrinter(node).str());
		}


	}
	// mout.accept<LOG_NOTICE>("compoundBBox   end:", group->getBoundingBox(), " obj=", group.data);

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

	/*
	if (group->hasClass("ADAPTER") || group->hasClass("MAIN") ){
		mout.accept<LOG_NOTICE>("Finally TUNING  ... ", compoundBBox, " of ", group.data);
		// TransformSVG & tr = group->transform;
		// group->transform.setTranslateX(-compoundBBox.x);
		if (compoundBBox.x || compoundBBox.y){
			group->set("hey", compoundBBox.getLocation().tuple());
		}

		//tr.translate.x = -compoundBBox.x;
		//tr.translate.y = -compoundBBox.y;
		group->transform.setTranslate(-compoundBBox.x, -compoundBBox.y);
		compoundBBox.setLocation(0.0, 0.0);
		//mout.accept<LOG_NOTICE>("... and we get ", NodePrinter(group).str());
		// std::cerr <<  tr << std::endl;
		std::stringstream sstr;
		group->nodeToStream(sstr);
		//mout.accept<LOG_NOTICE>("... and we get ", NodePrinter(group).str(), " tr=",  tr.translate.tuple(), " and bbox ", compoundBBox);
		mout.accept<LOG_NOTICE>("... and we get ", sstr.str(), " tr=",  group->transform.translate.tuple(), " and bbox ", compoundBBox);

	}
	*/

	/*
	if (group->hasClass("MAIN")){
		group->transform.translate.x = -group->getBoundingBox().x;
		group->transform.translate.y = -group->getBoundingBox().y;
	}
	*/


}


/**
 *   Pick dx = bbox.x
 *   translate.x = anchor.x -dx
 *
 *
 */
template <>
void TreeUtilsSVG::realignObject(NodeSVG & node, const CoordSpan<AlignBase::Axis::HORZ> & anchorSpan){

	// return;

	Logger mout(__FILE__, __FUNCTION__);

	// TODO/FIX: currently TEXT not handled at all (this method not invoked)
	const bool IS_TEXT = node.typeIs(svg::TEXT);

	// Elements with x and y attributes (typically declaring Upper Right corner, except for TEXT)
	const bool HAS_TRUE_ORIGIN = node.typeIs(svg::IMAGE, svg::RECT, svg::TEXT);

	Box<svg::coord_t> & obox = node.getBoundingBox();

	svg::coord_t coord = 0;

	AlignBase::Pos alignLoc;

	// STEP 1: derive reference point at the ANCHOR

	const std::string ns = NodePrinter(node).str();

	mout.debug("Adjusting ", ns, " with ", anchorSpan);

	switch (alignLoc = node.getAlign(AlignSVG::Owner::ANCHOR, AlignBase::Axis::HORZ)){
	case AlignBase::Pos::MIN:
		coord = anchorSpan.pos; // "+ 0%"
		break;
	case AlignBase::Pos::MID:
		coord = anchorSpan.pos + anchorSpan.span/2; // " + 50%"
		break;
	case AlignBase::Pos::MAX:
		coord = anchorSpan.pos + anchorSpan.span; // " + 50%"
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


	// STEP 2: OBJECT itself – define the reference point of the object to be aligned, i.e. adjusted.

	switch (alignLoc = node.getAlign(AlignSVG::Owner::OBJECT, AlignBase::Axis::HORZ)){
	case AlignBase::Pos::MIN:
		if (IS_TEXT){
			node.setStyle(StyleXML::TEXT_ANCHOR, "start");
			node.transform.translate.x = +2.0*node.getMargin();
		}
		break;
	case AlignBase::Pos::MID:
		if (IS_TEXT){
			node.setStyle(StyleXML::TEXT_ANCHOR, "middle");
		}
		else {
			coord -= obox.width/2;
		}
		break;
	case AlignBase::Pos::MAX:
		if (IS_TEXT){
			node.setStyle(StyleXML::TEXT_ANCHOR, "end");
			node.transform.translate.x = -2.0*node.getMargin();
		}
		else {
			coord -= obox.width;
		}
		break;
	case AlignBase::Pos::FILL:
		if (IS_TEXT){
			mout.reject<LOG_WARNING>("skipping FILL horz set for TEXT, : ", NodePrinter(node).str());
		}
		else {
			mout.experimental<LOG_DEBUG>("FILL:ing horz: ",   obox, " width -> ", anchorSpan.span);
			coord = anchorSpan.pos;
			obox.setWidth(anchorSpan.span);
		}
		break;
	case AlignBase::Pos::UNDEFINED_POS: // or absolute
		// mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", AlignSVG::Owner::OBJECT, '/', AlignBase::Axis::HORZ, pos);
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG::Pos ", alignLoc);
	}

	if (IS_TEXT){
		// Debuggin, remove later
		if ((obox.x)||(obox.y)){
			mout.suspicious("TEXT element with absolute coords: ", obox);
		}
	}

	if (HAS_TRUE_ORIGIN){ // || node.hasClass(LayoutSVG::ADAPTER)){ // (node.typeIs(svg::GROUP) && (node.hasClass("MAIN") || node.hasClass(LayoutSVG::ADAPTER)))){
		node.getBoundingBox().x = coord;
		if ((!IS_TEXT) && (node.transform.translate.x != 0.0)){
			mout.warn("Node to be moved by (x,y) also has translation:", node.transform.translate.tuple());
		}
		mout.debug("moved: ", ns, " <- ", anchorSpan, " MOVING -> ", node.getBoundingBox(), " (tr=", node.transform.translate.tuple(), ")");
	}
	else {
		node.transform.translate.x = coord;
		if (node.getBoundingBox().x != 0.0){
			mout.warn("Node to be translated also has non-zero x coordinate: ", node.getBoundingBox());
		}
		mout.debug("translated: ", ns, " <- ", anchorSpan, "  -> (", node.getBoundingBox(), ") TRANSLATED: tr=", node.transform.translate.tuple());
	}
	// mout.attention("Alignment::OBJECT-HORZ ", pos);
	// mout.debug("Alignment::Pos: ", AlignSVG::Owner::OBJECT, '/', axis, '=', alignLoc);
}

/**
 *  \param anchorSpan - width or height of the achore rectangle.
 */
template <>
void TreeUtilsSVG::realignObject(NodeSVG & node, const CoordSpan<AlignBase::Axis::VERT> & anchorSpan){
// void TreeUtilsSVG::realignObjectVertNEW(NodeSVG & node, const Box<svg::coord_t> & anchorBoxVert){

	Logger mout(__FILE__, __FUNCTION__);

	const bool IS_TEXT = node.typeIs(svg::TEXT);

	// Elements with x and y attributes (typically declaring Upper Right corner, except for TEXT)
	const bool HAS_TRUE_ORIGIN = node.typeIs(svg::IMAGE, svg::RECT, svg::TEXT);

	svg::coord_t coord = 0;
	AlignBase::Pos alignLoc;

	// STEP 1: derive reference point at the ANCHOR


	switch (alignLoc = node.getAlign(AlignSVG::Owner::ANCHOR, AlignBase::Axis::VERT)){
	case AlignBase::Pos::MIN:
		coord = anchorSpan.pos;
		break;
	case AlignBase::Pos::MID:
		coord = anchorSpan.pos + anchorSpan.span/2.0;
		break;
	case AlignBase::Pos::MAX:
		coord = anchorSpan.pos + anchorSpan.span;
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

	// STEP 2: OBJECT itself – define the reference point of the object to be aligned, i.e. adjusted.
	Box<svg::coord_t> & obox = node.getBoundingBox();

	if (IS_TEXT){
		coord += node.getHeight();
		node.transform.translate.y = -node.getMargin();
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
		if (IS_TEXT){
			mout.reject<LOG_WARNING>("skipping FILL (vert) set for TEXT, : ", NodePrinter(node).str());
			// mout.warn("FILL not applicable for TEXT elem - ", node);
		}
		else {
			coord = anchorSpan.pos;
			obox.setHeight(anchorSpan.span);
		}
		break;
	case AlignBase::Pos::UNDEFINED_POS: // or absolute
		mout.unimplemented<LOG_WARNING>("Alignment::Pos: ", alignLoc, " for ", AlignSVG::Owner::OBJECT, '/', AlignBase::Axis::VERT);
		break;
	default:
		mout.unimplemented<LOG_ERR>("AlignSVG::Pos ", alignLoc);
	}

	if (HAS_TRUE_ORIGIN) { // || node.hasClass(LayoutSVG::ADAPTER)){ //){
		node.getBoundingBox().y = coord;
	}
	else {
		node.transform.translate.y = coord;
	}

	// node.transform.translate.y = coord;

}




// ---------------------------------------------------





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


const std::string MetaDataCollectorSVG::LOCAL("LOCAL");

const std::string MetaDataCollectorSVG::SHARED("SHARED");

/// Iterate children and their attributes: check which attributes (key and value) are shared by all the children.
/**
 *  Collects meta data written upon creating image panels.
 *
 *  General - move to TreeUtilsSVG
 */
int MetaDataCollectorSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){

	drain::Logger mout(__FILE__, __FUNCTION__);

	TreeSVG & current = tree(path);

	if (!current->typeIs(svg::GROUP, svg::SVG)){ // At least METADATA must be skipped...
		return 0;
	}


	/// Statistics: computer count for each (key,value) pair, for example (what:time, "20240115")
	// typedef std::map<std::string, unsigned short> variableStat_t;

	// Statistics of "<key>=<value>" entries.
	variableStat_t attributeValueCounts;

	/// Number of children having (any) metadata.
	int childCount = 0;

	/// Iterate children and their attributes: mark down all the (key,value) pairs.
	for (const auto & entry: current.getChildren()){
		const TreeSVG & child = entry.second;
		if (child.hasChild(svg::METADATA) && !child->hasClass("legend")){ // or has "data"? Is this for Palette? Generalize somehow...
			++childCount;
			for (const auto & attr: child[svg::METADATA]->getAttributes()){
				std::string s = drain::StringBuilder<>(attr.first,'=',attr.second);
				++attributeValueCounts[s];
			}
		}
	}

	if (childCount == 0){
		return 0;
	}

	// There are metadata, so investigate...


	TreeSVG & debugSharedBase = current[svg::DESC](svg::DESC);
	// debugSharedBase->addClass("SHARED"); // just a marker.
	// TreeSVG & debugShared = debugSharedBase["cmt"](svg::COMMENT);

	if (mout.isLevel(LOG_DEBUG)){
		TreeSVG & debugAll = current["description"](svg::DESC);
		debugAll->set("COUNT", childCount);
		debugAll->setText("All"); //CTXX
	}
	/*
		TreeSVG & debugExplicit = current["rejected"](svg::DESC);
		debugExplicit->addClass("EXPLICIT");
		debugAll->ctext += drain::sprinter(stat).str();
	 */

	// Better order this way - METADATA elems (LOCAL and SHARED) will be together.
	TreeSVG & metadata = current[svg::METADATA](svg::METADATA);


	mout.pending<LOG_DEBUG>("pruning: ", drain::sprinter(attributeValueCounts), path.str());

	// If this group level has variable entries ABC=123, DEF=456, ... , "lift" them from the lower level, ie prune them there
	for (const auto & e: attributeValueCounts){

		mout.pending<LOG_DEBUG>('\t', e.first, ':', e.second);
		// std::cerr << "\t vector " << e.first << ' ' << e.second << std::endl;

		if (e.second == childCount){ // This attribute value is shared by all the children.

			mout.accept<LOG_DEBUG>('\t', e.first, ' ', path.str());

			debugSharedBase.addChild() = e.first;

			// Split the entry back to (key,value) pair and update "upwards", ie. from children to this level.
			std::string key, value;
			drain::StringTools::split2(e.first, key, value, '=');
			metadata->set(key, value); // NOTE: forced strings (later, consider type dict?)

			// Actual pruning, "downwards".

			for (auto & entry: current.getChildren()){

				TreeSVG & child = entry.second;

				if (child.hasChild(svg::METADATA)){

					TreeSVG & childMetadata = child[svg::METADATA]; //(svg::METADATA);
					//
					childMetadata->addClass("LOCAL");
					childMetadata->removeAttribute(key);

					//TreeSVG & childMetadata2 = child[PanelConfSVG::ElemClass::SHARED_METADATA](svg::METADATA);
					TreeSVG & childMetadata2 = child[MetaDataCollectorSVG::SHARED](svg::METADATA);
					childMetadata2->addClass(MetaDataCollectorSVG::SHARED);
					childMetadata2->set(key, value);

				}
			}
			// }

		}
		else {
			mout.reject<LOG_DEBUG>('\t', e.first, ' ', path.str());
			// debugExplicit->ctext += e.first;
		}
	}

	/*
	if (current.hasChild("ADAPTER")){

		TreeSVG & adapterMetadata = current["ADAPTER"][svg::METADATA];

		for (const auto & entry: metadata->getAttributes()){
			adapterMetadata->set(entry.first, entry.second);
		}
		// adapterMetadata->set("time", "123456");
		// mout.warn(adapterMetadata->getAttributes());
	}
	*/


	return 0;

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

/*
DRAIN_ENUM_DICT(image::MetaDataCollectorSVG::MetaDataType) = {
		DRAIN_ENUM_ENTRY(image::MetaDataCollectorSVG, LOCAL),
		DRAIN_ENUM_ENTRY(image::MetaDataCollectorSVG, SHARED),
};
*/

/*
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const image::MetaDataCollectorSVG::MetaDataType & type) const {
	return (*this)[EnumDict<image::MetaDataCollectorSVG::MetaDataType>::dict.getKey(type, false)];
}
*/


}  // drain::
