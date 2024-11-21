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

/*
#define ENUM_PAIR(NSPACE, ARG) {#ARG, NSPACE::ARG}
#define APPLY_ENUM_ENTRY(NSPACE, FIRST, ...)  ENUM_PAIR(NSPACE, FIRST)
#define IS_EMPTY(...) _IS_EMPTY_IMPL(__VA_ARGS__, 1, 0)
#define _IS_EMPTY_IMPL(_1, _2, N, ...) N
*/


template <>
const drain::EnumDict<PanelConfSVG::Orientation>::dict_t  drain::EnumDict<PanelConfSVG::Orientation>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Orientation, HORZ),
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Orientation, VERT),
};

template <>
const drain::EnumDict<PanelConfSVG::Direction>::dict_t  drain::EnumDict<PanelConfSVG::Direction>::dict = {
		// {"INCR", drain::image::PanelConfSVG::INCR},
		// {"DECR", drain::image::PanelConfSVG::DECR}
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Direction, INCR),
		DRAIN_ENUM_ENTRY(drain::image::PanelConfSVG::Direction, DECR)
};

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

bool TreeUtilsSVG::getRect(const TreeSVG & group, Box<double> & rect){

	rect.setLocation(group->get("x", 0), group->get("y", 0));
	rect.setWidth(group->get("width", 0));
	rect.setHeight(group->get("height", 0));

	return (rect.getWidth() != 0.0) && (rect.getHeight() != 0.0);

}

/** Stacks all the non-FLOAT elements horizontally or vertically, computing their collective width or height.
 *
 */
void TreeUtilsSVG::getBoundingFrame(const TreeSVG & group, Frame2D<int> & frame, PanelConfSVG::Orientation orientation){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (orientation == PanelConfSVG::UNDEFINED_ORIENTATION){
		orientation = defaultConf.orientation;
		if (orientation == PanelConfSVG::UNDEFINED_ORIENTATION){
			orientation = PanelConfSVG::HORZ;
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

	// if (mout.isDebug()){}
	// group->set("data-frame", frame.tuple());
	// bbox.set(0, 0, width, height);
}

void TreeUtilsSVG::alignSequence(TreeSVG & group, const drain::Frame2D<int> & frame, const drain::Point2D<int> & start, PanelConfSVG::Orientation orientation, PanelConfSVG::Direction direction){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("aligning elems of group ", group->get("name", "?"));

	if (orientation == PanelConfSVG::UNDEFINED_ORIENTATION){
		orientation = TreeUtilsSVG::defaultConf.orientation;
		if (orientation == PanelConfSVG::UNDEFINED_ORIENTATION){
			orientation = PanelConfSVG::HORZ;
		}
	}

	if (direction == PanelConfSVG::UNDEFINED_DIRECTION){
		direction = TreeUtilsSVG::defaultConf.direction;
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


}


/// Marker for...
const std::string TreeUtilsSVG::attr_FRAME_REFERENCE("data-frame-ref");


#define ENUM_ENTRY(NSPACE, ARG1, ...) {#ARG1, NSPACE::ARG1}
//, ENUM_ENTRY(NSPACE, __VA_ARGS__)
#define _ARG2(_0, _1, _2, ...) _2
#define NARG2(...) _ARG2(__VA_ARGS__, 2, 1, 0)

//#define MAP_ENTRY(NSPACE, ARG1, ...) ({#ARG1, NSPACE::ARG1}, ENUM_ENTRY(NSPACE, ...) )

std::map<std::string,int> m = {
		{"mika", 1}
};

// #define  DRAIN_ENUM_NAMESPACE drain::image::AlignSVG
template <>
const drain::EnumDict<AlignSVG>::dict_t  drain::EnumDict<AlignSVG>::dict = {
		//ENUM_ENTRY(drain::image::AlignSVG, PANEL),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, ALIGN_GROUP),
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, LEFT),
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
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, FLOAT), // (probably deprecating)
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, ALIGN), // (probably deprecating)
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, ANCHOR), // (probably deprecating)
		DRAIN_ENUM_ENTRY(drain::image::AlignSVG, RELATIVE), // (probably deprecating)
};
// #undef  DRAIN_ENUM_NAMESPACE


void TreeUtilsSVG::markAligned(const TreeSVG & elem, TreeSVG & alignedElem){
	alignedElem->addClass(AlignSVG::FLOAT);
	alignedElem->addClass(AlignSVG::ALIGN);
	alignedElem->set(attr_FRAME_REFERENCE, elem->getId());
}


void TreeUtilsSVG::alignText(TreeSVG & group){

	drain::Logger mout(__FILE__, __FUNCTION__);

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
				Box<double> box;

				if (TreeUtilsSVG::getRect(frame, box)){

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

}


//
/**  Retrieves all the elements of ALIGN class and (re)set their positions.
 *
 */
void TreeUtilsSVG::alignNEW(TreeSVG & group){

	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::image::NodeSVG::path_list_t pathList;
	drain::image::NodeSVG::findByClass(group, ALIGN, pathList);

	// mout.accept<LOG_WARNING>("yes accept: ", drain::EnumDict<AlignSVG>::dict.getKey(FLOAT));

	// const drain::Point2D<double> textOffset(5.0, 5.0);

	for (const drain::image::NodeSVG::path_t & path: pathList){

		mout.attention("Would like to align: ", path);

		TreeSVG & elem =  group(path);

		const std::string refID = elem->get(attr_FRAME_REFERENCE, "");

		if (refID.empty()){
			mout.warn("Reference attribute '", attr_FRAME_REFERENCE, "' missing for <", elem->getTag(), ".. > named ", elem->get("name",""), " at ", path);
			continue;
		}

		const bool TEXT = elem->typeIs(NodeSVG::TEXT);

		Box<double> box;
		if ((!TEXT) && TreeUtilsSVG::getRect(elem, box)){
			mout.attention("Computed box '", ((const drain::Point2D<double> &)box).tuple(), ' ', ((const drain::Frame2D<double> &)box).tuple(), "' of <", elem->getTag(), ".. > named ", elem->get("name",""), " at ", path);
		}
		else {
			// mout.warn("Faild in computing box of <", elem->getTag(), ".. > named ", elem->get("name",""), " at ", path);
			box.x = elem->get("x", 0.0);
			box.y = elem->get("y", 0.0);
			box.width  = 250;
			box.height = 100;
			mout.pending<LOG_WARNING>("Using box ", box, "for <", elem->getTag(), ".. > named ", elem->get("name",""), " at ", path);
		}


		drain::image::NodeSVG::path_t p;

		if (NodeSVG::findById(group, refID, p)){ // maybe slow

			TreeSVG & frame = group(p);
			Box<double> frameBox;

			if (TreeUtilsSVG::getRect(frame, frameBox)){
				mout.accept<LOG_WARNING>("Frame <", frame->getTag(), ".. > id='", refID, "' for <", elem->getTag(), ".. > named ", elem->get("name",""), " at ", path);

				Point2D<double> pos;

				// "Main"

				for (const auto & alignCls: {
						REF_LEFT, REF_CENTER, REF_RIGHT, REF_TOP, REF_MIDDLE, REF_BOTTOM,
						LEFT, CENTER, RIGHT, TOP, MIDDLE, BOTTOM
				}){

					const std::string & clsLabel = drain::EnumDict<AlignSVG>::dict.getKey(alignCls);

					if (elem->hasClass(clsLabel)){

						switch(alignCls){
						case REF_LEFT:
							pos.x = frameBox.x;
							// if (DEBUG) draw marker
							break;
						case REF_CENTER:
							pos.x = frameBox.x + frameBox.width/2;
							break;
						case REF_RIGHT:
							pos.x = frameBox.x + frameBox.width;
							break;
						case REF_TOP:
							pos.y = frameBox.y;
							break;
						case REF_MIDDLE:
							pos.y = frameBox.y + frameBox.height/2;
							break;
						case REF_BOTTOM:
							pos.y = frameBox.y + frameBox.height;
							break;
							// ------------------------------------------------
						case LEFT:
							//pos.x = box.x;
							if (TEXT){

							}
							break;
						case CENTER:
							pos.x -= box.width/2;
							if (TEXT){
							}
							break;
						case RIGHT:
							pos.x -= box.width;
							if (TEXT){
							}
							break;
						case TOP:
							//pos.y += 0;
							break;
						case MIDDLE:
							pos.y -= box.height/2;
							break;
						case BOTTOM:
							pos.y -= box.height;
							break;
						default:
							mout.error("Something went wrong, unknown class '", clsLabel, "'", (int) alignCls);
						}


					}


				}

				elem->set("x", pos.x);
				elem->set("y", pos.y);
				// if (elem->hasClass(ALIGN))...
				// elem->clearClasses();
				elem->addClass(ALIGN);
			}
			else {
				mout.reject("Could compute frame dimensions of <", frame->getTag(), ".. > id='", refID, "'");
			}
		}
		else {
			mout.reject("Could not find reference (frame) element with id='", refID, "' for <", elem->getTag(), ".. > named ", elem->get("name",""), " at ", path);
		}

	}

}

//
/**  Retrieves all the elements of ALIGN class and (re)set their positions.
 *
 */
void alignFOO_NEW(TreeSVG & group){

	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::image::NodeSVG::path_list_t pathList;
	drain::image::NodeSVG::findByClass(group, ALIGN, pathList);

	// mout.accept<LOG_WARNING>("yes accept: ", drain::EnumDict<AlignSVG>::dict.getKey(FLOAT));

	// const drain::Point2D<double> textOffset(5.0, 5.0);

	for (const drain::image::NodeSVG::path_t & path: pathList){

		mout.attention("Would like to align: ", path);

		TreeSVG & elem =  group(path);

		const std::string refID = elem->get(TreeUtilsSVG::attr_FRAME_REFERENCE, "");

		if (refID.empty()){
			mout.warn("Reference attribute '", TreeUtilsSVG::attr_FRAME_REFERENCE, "' missing for <", elem->getTag(), ".. > named ", elem->get("name",""), " at ", path);
			continue;
		}

		const bool TEXT = elem->typeIs(NodeSVG::TEXT);

		Box<double> box;
		if ((!TEXT) && TreeUtilsSVG::getRect(elem, box)){
			mout.attention("Computed box '", ((const drain::Point2D<double> &)box).tuple(), ' ', ((const drain::Frame2D<double> &)box).tuple(), "' of <", elem->getTag(), ".. > named ", elem->get("name",""), " at ", path);
		}
		else {
			// mout.warn("Faild in computing box of <", elem->getTag(), ".. > named ", elem->get("name",""), " at ", path);
			box.x = elem->get("x", 0.0);
			box.y = elem->get("y", 0.0);
			box.width  = 250;
			box.height = 100;
			mout.pending<LOG_WARNING>("Using box ", box, "for <", elem->getTag(), ".. > named ", elem->get("name",""), " at ", path);
		}


		drain::image::NodeSVG::path_t p;

		if (NodeSVG::findById(group, refID, p)){ // maybe slow

			TreeSVG & frame = group(p);
			Box<double> frameBox;

			if (TreeUtilsSVG::getRect(frame, frameBox)){
				mout.accept<LOG_WARNING>("Frame <", frame->getTag(), ".. > id='", refID, "' for <", elem->getTag(), ".. > named ", elem->get("name",""), " at ", path);

				Point2D<double> pos;

				// "Main"

				for (const auto & alignCls: {
						REF_LEFT, REF_CENTER, REF_RIGHT, REF_TOP, REF_MIDDLE, REF_BOTTOM,
						LEFT, CENTER, RIGHT, TOP, MIDDLE, BOTTOM
				}){

					const std::string & clsLabel = drain::EnumDict<AlignSVG>::dict.getKey(alignCls);

					if (elem->hasClass(clsLabel)){

						switch(alignCls){
						case REF_LEFT:
							pos.x = frameBox.x;
							// if (DEBUG) draw marker
							break;
						case REF_CENTER:
							pos.x = frameBox.x + frameBox.width/2;
							break;
						case REF_RIGHT:
							pos.x = frameBox.x + frameBox.width;
							break;
						case REF_TOP:
							pos.y = frameBox.y;
							break;
						case REF_MIDDLE:
							pos.y = frameBox.y + frameBox.height/2;
							break;
						case REF_BOTTOM:
							pos.y = frameBox.y + frameBox.height;
							break;
							// ------------------------------------------------
						case LEFT:
							//pos.x = box.x;
							if (TEXT){

							}
							break;
						case CENTER:
							pos.x -= box.width/2;
							if (TEXT){
							}
							break;
						case RIGHT:
							pos.x -= box.width;
							if (TEXT){
							}
							break;
						case TOP:
							//pos.y += 0;
							break;
						case MIDDLE:
							pos.y -= box.height/2;
							break;
						case BOTTOM:
							pos.y -= box.height;
							break;
						default:
							mout.error("Something went wrong, unknown class '", clsLabel, "'", (int) alignCls);
						}


					}


				}

				elem->set("x", pos.x);
				elem->set("y", pos.y);
				// if (elem->hasClass(ALIGN))...
				// elem->clearClasses();
				elem->addClass(ALIGN);
			}
			else {
				mout.reject("Could compute frame dimensions of <", frame->getTag(), ".. > id='", refID, "'");
			}
		}
		else {
			mout.reject("Could not find reference (frame) element with id='", refID, "' for <", elem->getTag(), ".. > named ", elem->get("name",""), " at ", path);
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
