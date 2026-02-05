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

#include "drain/util/UtilsXML.h"
#include "TreeSVG.h"

namespace drain {

DRAIN_ENUM_DICT(image::FileSVG::PathPolicy) = {
		DRAIN_ENUM_ENTRY(image::FileSVG::PathPolicy, ABSOLUTE),
		DRAIN_ENUM_ENTRY(image::FileSVG::PathPolicy, PREFIXED)
};

DRAIN_ENUM_DICT(image::FileSVG::IncludePolicy) = {
		DRAIN_ENUM_ENTRY(image::FileSVG::IncludePolicy, NONE),
		DRAIN_ENUM_ENTRY(image::FileSVG::IncludePolicy, PNG),
		DRAIN_ENUM_ENTRY(image::FileSVG::IncludePolicy, SVG),
		DRAIN_ENUM_ENTRY(image::FileSVG::IncludePolicy, TXT),
		DRAIN_ENUM_ENTRY(image::FileSVG::IncludePolicy, ALL),
		DRAIN_ENUM_ENTRY(image::FileSVG::IncludePolicy, NEXT),
		DRAIN_ENUM_ENTRY(image::FileSVG::IncludePolicy, SKIP),
		DRAIN_ENUM_ENTRY(image::FileSVG::IncludePolicy, ON),
		DRAIN_ENUM_ENTRY(image::FileSVG::IncludePolicy, OFF),
		DRAIN_ENUM_ENTRY(image::FileSVG::IncludePolicy, UNKNOWN),
};


int image::FileSVG::radialBezierResolution = 8;


DRAIN_TYPENAME_DEF(image::NodeSVG);
DRAIN_TYPENAME_DEF(image::svg::tag_t);

DRAIN_ENUM_DICT(image::svg::tag_t) = {
		{"UNDEFINED", drain::image::svg::UNDEFINED},
		{"#", drain::image::svg::COMMENT},
		{"CTEXT", drain::image::svg::CTEXT},
		{"script", drain::image::svg::SCRIPT},
		{"style", drain::image::svg::STYLE},
		{"style_select", drain::image::svg::STYLE_SELECT},
		{"javascript_scope", drain::image::svg::JAVASCRIPT_SCOPE},
		// -- Actual SVG elems
		{"svg", drain::image::svg::SVG},
		{"circle", drain::image::svg::CIRCLE},
		{"clipPath", drain::image::svg::CLIP_PATH},
		{"defs", drain::image::svg::DEFS},
		{"desc", drain::image::svg::DESC},
		{"g", drain::image::svg::GROUP},
		{"image", drain::image::svg::IMAGE},
		{"line", drain::image::svg::LINE},
		{"linearGradient", drain::image::svg::LINEAR_GRADIENT},
		{"mask", drain::image::svg::MASK},
		{"metadata", drain::image::svg::METADATA},
		{"polygon", drain::image::svg::POLYGON},
		{"path", drain::image::svg::PATH},
		{"rect", drain::image::svg::RECT},
		{"style", drain::image::svg::STYLE}, // raise?
		{"text", drain::image::svg::TEXT},
		{"title", drain::image::svg::TITLE},
		{"tspan", drain::image::svg::TSPAN},
};



// Applied by UtilsXML::addChild()
template <>
const NodeXML<drain::image::svg::tag_t>::xml_default_elem_map_t NodeXML<drain::image::svg::tag_t>::xml_default_elems = {
		{drain::image::svg::STYLE, drain::image::svg::STYLE_SELECT}, // ??
		// {drain::image::svg::STYLE,  drain::image::svg::CTEXT}, // ??
		{drain::image::svg::SCRIPT, drain::image::svg::CTEXT},
		//{drain::image::svg::SVG, drain::image::svg::GROUP},
};


namespace image {

const drain::FileInfo FileSVG::fileInfo("svg");
// const drain::FileInfo NodeSVG::fileInfo("svg");


template <>
NodeSVG::xmldoc_attrib_map_t NodeSVG::xml_node_t::xmldoc_attribs = {
		{"version",  "1.0"},
		{"encoding", "UTF-8"},
		{"standalone", "no"},
		// {"data-remark", "svg"} debugging? inkview claims
};


std::string NodeSVG::xlink("http://www.w3.org/1999/xlink");
std::string NodeSVG::svg("http://www.w3.org/2000/svg");


// Explicit default constructor of base class  xml_node_t
NodeSVG::NodeSVG(svg::tag_t t) : xml_node_t() {
	setType(t);
}

// Explicit default constructor of base class  xml_node_t
NodeSVG::NodeSVG(const NodeSVG & node) : xml_node_t(), box(0,0,0,0) { // , radius(0)
	// RISKY references! copyStruct(node, node, *this, ReferenceMap2::extLinkPolicy::LINK); // <-- risky! may link Variable contents?
	XML::xmlAssignNode(*this, node); // this should be good.
}

//void NodeSVG::handleType(const svg::tag_t & t) { // setType(const elem_t & t) {
void NodeSVG::handleType() { // setType(const elem_t & t) {

	switch (getNativeType()) {
	case image::svg::UNDEFINED:
		break;
	case image::svg::COMMENT:
		break;
	case image::svg::CTEXT:
		break;
	case image::svg::SVG:
	  getMap()["width"].link(box.width); // = 0);
		// link("width", box.width); // = 0);
	  getMap()["height"].link(box.height); // = 0);
	  getMap()["xmlns"].link(NodeSVG::svg);
	  getMap()["xmlns:svg"].link(NodeSVG::svg);
	  getMap()["xmlns:xlink"].link(NodeSVG::xlink);
		break;
	case image::svg::TITLE:
		break;
	case image::svg::GROUP:
	  getMap()["data-pos"].link(box.getLocation().tuple());
	  getMap()["data-frm"].link(box.getFrame().tuple());
		break;
	case image::svg::RECT:
	  getMap()["x"].link(box.x = 0);
	  getMap()["y"].link(box.y = 0);
	  getMap()["width"].link(box.width); // = 0);
	  getMap()["height"].link(box.height); // = 0);
		break;
	case image::svg::CIRCLE:
	  getMap()["cx"].link(box.x = 0);
	  getMap()["cy"].link(box.y = 0);
		set("r", svg::coord_t(0));
		// getMap()["r"].link(radius = 0);
		break;
	case image::svg::IMAGE:
	  getMap()["x"].link(box.x = 0);
	  getMap()["y"].link(box.y = 0);
	  getMap()["width"].link(box.width); //  = 0);
	  getMap()["height"].link(box.height); //  = 0);
		// if (version == 1) {
	  getMap()["xlink:href"].link(url); // text_anchor
		// if (version > 2.x ?) {
		break;
	case image::svg::PATH:
		get("d").setType<std::string>();
		get("d").setSeparator(0);
		break;
	case image::svg::POLYGON:
		// getMap()["path"].link(ctext);
		// set("path", "");
		get("points").setType<std::string>();
		get("points").setSeparator(0);
		break;
	case image::svg::TEXT:
	  getMap()["x"].link(box.x); //  = 0);
	  getMap()["y"].link(box.y); //  = 0);
		break;
	case image::svg::TSPAN:
		break;
	default:
		return;
	}

	// getMap()["pos"].link(box.getLocation().tuple());
	// getMap()["frm"].link(box.getFrame().tuple());
	// DEPRECATING: see separate STYLE and CLASS?
	// getMap()["style"].link(style = "");
	// getMap()["fill"].link(fill = "");
	// getMap()["opacity"].link(opacity = ""); // string, so silent if empty


}

/* Well, every graphic obj may have DESC and TITLE?
bool NodeSVG::isSingular() const {

	return typeIs(
			svg::CIRCLE,
	);
};
*/

/// Needed for handling units in strings, like "50%" or "640px".
void NodeSVG::setAttribute(const std::string & key, const std::string &value){
	(*this)[key] = value;
}

/// Needed for handling units in strings, like "50%" or "640px".
void NodeSVG::setAttribute(const std::string & key, const char *value){
	(*this)[key] = value; // -> handleString()
}

void NodeSVG::setFontSize(svg::coord_t size, svg::coord_t elemHeight){
	switch (getNativeType()) {
		case svg::TEXT:
			//case svg::TSPAN:
			// elemHeight = (elemHeight>0.0) ? elemHeight : 1.5*size;
			if (elemHeight > 0){
				//setHeight(-elemHeight);
				setHeight(elemHeight);
			}
			else if (getHeight() == 0.0){
				//setHeight(-1.5*size);
				setHeight(1.5*size);
			}
			setMargin(0.25*size);
			// svg::coord_t
			// transform.translate[0] = getMargin();
			// transform.translate[1] = getHeight()-getMargin();
			// no break
		default:
			setStyle("font-size", size);
			// elemHeight unused... warn if given?
			break;
	}
}

void NodeSVG::swapSVG(NodeSVG & node){
	// Attributes
	NodeXML<svg::tag_t>::swap(node);
	// Classes (this could be in NodeXML<svg::tag_t> ?)
	// classList.swap(node.classList);
	// Additional SVG properties
	swapAlign(node);
	swapAnchors(node);
	/*
	alignments.swap(node.alignments);
	myAnchorHorz.swap(node.myAnchorHorz);
	myAnchorVert.swap(node.myAnchorVert);
	defaultAnchorHorz.swap(node.defaultAnchorHorz);
	defaultAnchorVert.swap(node.defaultAnchorVert);
	*/
};

void NodeSVG::updateAlign(){

	updateAlignStr();

	if (alignStr.empty()){
		this->getMap().erase("data-align");
	}
	else {
		// if (!this->hasKey("data-align")){
		this->getMap()["data-align"].link(alignStr); // (should be safe anyway)
		//}
	}

	this->getMap().erase("data-anchor");
	this->getMap().erase("data-anchorHorz");
	this->getMap().erase("data-anchorVert");

	if ((!myAnchorHorz.empty()) || (!myAnchorVert.empty())){

		if (myAnchorHorz == myAnchorVert){
		  this->getMap()["data-anchor"].link(myAnchorHorz.str());
		}
		else if (!myAnchorHorz.empty()){
		  this->getMap()["data-anchorHorz"].link(myAnchorHorz.str());
		}
		else if (!myAnchorVert.empty()){
		  this->getMap()["data-anchorVert"].link(myAnchorVert.str());
		}

	}

	this->getMap().erase("data-anchorDefault");
	this->getMap().erase("data-anchorHorzDefault");
	this->getMap().erase("data-anchorVertDefault");

	if ((!defaultAnchorHorz.empty()) || (!defaultAnchorVert.empty())){

		if (defaultAnchorHorz == defaultAnchorVert){
		  this->getMap()["data-anchorDefault"].link(defaultAnchorHorz.str());
		}
		else if (!defaultAnchorHorz.empty()){
		  this->getMap()["data-anchorHorzDefault"].link(defaultAnchorHorz.str());
		}
		else if (!defaultAnchorVert.empty()){
		  this->getMap()["data-anchorVertDefault"].link(defaultAnchorVert.str());
		}

	}


}

void NodeSVG::specificAttributesToStream(std::ostream & ostr) const {
	XML::specificAttributesToStream(ostr);
	if (!transform.empty()){
		ostr << ' ' << "transform=" << '"';
		transform.toStream(ostr);
		ostr << '"';
	}
}

}  // image::


template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const image::svg::tag_t & type) const {
	return hasChild(EnumDict<image::svg::tag_t>::dict.getKey(type, true)); // no error on non-existent dict entry
}

/// Set the type.
template <> // for T - Tree class
template <> // for K - operator() argument
image::TreeSVG & image::TreeSVG::operator()(const image::svg::tag_t & type){
	return UtilsXML::setType(*this, type);
}

/// Automatic conversion of element type (enum value) to a string.
/**
 *  Convenience
 */
template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const image::svg::tag_t & type){
	return (*this)[EnumDict<image::svg::tag_t>::dict.getKey(type, false)];
}


/// Automatic conversion of element type (enum value) to a string.
/**
 *   Convenience
 */
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const image::svg::tag_t & type) const {
	return (*this)[EnumDict<image::svg::tag_t>::dict.getKey(type, false)];
}

template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const ClassXML & cls) const {
	return hasChild(cls.strPrefixed()); // no error on non-existent dict entry
}

template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const ClassXML & cls){
	return (*this)[cls.strPrefixed()];
}

/// Automatic conversion of element type (enum value) to a string.
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const ClassXML & cls) const {
	return (*this)[cls.strPrefixed()];
}


}  // drain::



