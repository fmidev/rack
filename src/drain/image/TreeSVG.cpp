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

#include "TreeSVG.h"

namespace drain {

DRAIN_TYPENAME_DEF(image::NodeSVG);
DRAIN_TYPENAME_DEF(image::svg::tag_t);

/*
const drain::EnumDict<image::svg::tag_t>::dict_t & getDict(){

	static drain::EnumDict<image::svg::tag_t>::dict_t hidden;

	if (hidden.empty()){
		hidden =  {
				{"UNDEFINED", drain::image::svg::UNDEFINED},
				{"#", drain::image::svg::COMMENT},
				{"script", drain::image::svg::SCRIPT},
				{"style", drain::image::svg::STYLE},
				{"style_select", drain::image::svg::STYLE_SELECT},
				{"svg", drain::image::svg::SVG},
				{"circle", drain::image::svg::CIRCLE},
				{"desc", drain::image::svg::DESC},
				{"g", drain::image::svg::GROUP},
				{"image", drain::image::svg::IMAGE},
				{"line", drain::image::svg::LINE},
				{"metadata", drain::image::svg::METADATA},
				{"polygon", drain::image::svg::POLYGON},
				{"rect", drain::image::svg::RECT},
				{"style", drain::image::svg::STYLE}, // raise?
				{"text", drain::image::svg::TEXT},
				{"title", drain::image::svg::TITLE},
				{"tspan", drain::image::svg::TSPAN},
		};
	}

	return hidden;

}
*/

/*
template <>
const EnumDict<image::svg::tag_t>::dict_t EnumDict<image::svg::tag_t>::dict = drain::EnumDict<image::svg::tag_t>::getDict();
*/

// NEW (slower?)

template <>
const EnumDict<image::svg::tag_t>::dict_t EnumDict<image::svg::tag_t>::dict = {
		/*
		{"UNDEFINED", XML::UNDEFINED},
		{"COMMENT",   XML::COMMENT},
		{"CTEXT",     XML::CTEXT},
		{"SCRIPT",    XML::SCRIPT},
		{"STYLE",     XML::STYLE},
		{"STYLE_SELECT", XML::STYLE_SELECT},
		*/
		{"UNDEFINED", drain::image::svg::UNDEFINED},
		{"#", drain::image::svg::COMMENT},
		{"CTEXT", drain::image::svg::CTEXT},
		{"script", drain::image::svg::SCRIPT},
		{"style", drain::image::svg::STYLE},
		{"style_select", drain::image::svg::STYLE_SELECT},
		{"svg", drain::image::svg::SVG},
		{"circle", drain::image::svg::CIRCLE},
		{"desc", drain::image::svg::DESC},
		{"g", drain::image::svg::GROUP},
		{"image", drain::image::svg::IMAGE},
		{"line", drain::image::svg::LINE},
		{"metadata", drain::image::svg::METADATA},
		{"polygon", drain::image::svg::POLYGON},
		{"rect", drain::image::svg::RECT},
		{"style", drain::image::svg::STYLE}, // raise?
		{"text", drain::image::svg::TEXT},
		{"title", drain::image::svg::TITLE},
		{"tspan", drain::image::svg::TSPAN},
};



// Applied by XML::xmlAddChild()
template <>
const NodeXML<drain::image::svg::tag_t>::xml_default_elem_map_t NodeXML<drain::image::svg::tag_t>::xml_default_elems = {
		{drain::image::svg::STYLE,  drain::image::svg::CTEXT},
		{drain::image::svg::SCRIPT, drain::image::svg::CTEXT},
		//{drain::image::svg::SVG, drain::image::svg::GROUP},
		{drain::image::svg::STYLE, drain::image::svg::STYLE_SELECT},
};


namespace image {

TransformSVG::TransformSVG(){ //  : translate(typeid(svg::coord_t)) {

	/*
	rotate.setOutputSeparator(' ');
	scale.setOutputSeparator(' ');
	translate.setOutputSeparator(' ');
	matrix.setOutputSeparator(' ');
	*/
};


void TransformSVG::toStream(std::ostream & ostr) const {
#define DRAIN_SVGTRANS_STREAM(transform) if (!transform.empty()){ostr << #transform << " " << transform << " ";}
	DRAIN_SVGTRANS_STREAM(rotate)
	DRAIN_SVGTRANS_STREAM(scale)
	DRAIN_SVGTRANS_STREAM(translate)
	DRAIN_SVGTRANS_STREAM(matrix)
}

template <>
NodeSVG::xmldoc_attrib_map_t NodeSVG::xml_node_t::xmldoc_attribs = {
		{"version",  "1.0"},
		{"encoding", "UTF-8"},
		{"standalone", "no"},
		// {"data-remark", "svg"} debugging? inkview claims
};

const drain::FileInfo NodeSVG::fileInfo("svg");

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
		link("width", box.width); // = 0);
		link("height", box.height); // = 0);
		link("xmlns", NodeSVG::svg);
		link("xmlns:svg", NodeSVG::svg);
		link("xmlns:xlink", NodeSVG::xlink);
		break;
	case image::svg::TITLE:
		break;
	case image::svg::GROUP:
		link("data-pos", box.getLocation().tuple());
		link("data-frm", box.getFrame().tuple());
		break;
	case image::svg::RECT:
		link("x", box.x = 0);
		link("y", box.y = 0);
		link("width", box.width); // = 0);
		link("height", box.height); // = 0);
		break;
	case image::svg::CIRCLE:
		link("cx", box.x = 0);
		link("cy", box.y = 0);
		set("r", svg::coord_t(0));
		// link("r", radius = 0);
		break;
	case image::svg::IMAGE:
		link("x", box.x = 0);
		link("y", box.y = 0);
		link("width", box.width); //  = 0);
		link("height", box.height); //  = 0);
		// if (version == 1) {
		link("xlink:href", url); // text_anchor
		// if (version > 2.x ?) {
		break;
	case image::svg::POLYGON:
		// link("path", ctext);
		// set("path", "");
		get("points").setType<std::string>();
		get("points").setSeparator(0);
		break;
	case image::svg::TEXT:
		link("x", box.x); //  = 0);
		link("y", box.y); //  = 0);
		break;
	case image::svg::TSPAN:
		break;
	default:
		return;
	}

	// link("pos", box.getLocation().tuple());
	// link("frm", box.getFrame().tuple());
	// DEPRECATING: see separate STYLE and CLASS?
	// link("style", style = "");
	// link("fill", fill = "");
	// link("opacity", opacity = ""); // string, so silent if empty


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
			//transform.translate[0] = getMargin();
			//transform.translate[1] = getHeight()-getMargin();
			// no break
		default:
			setStyle("font-size", size);
			break;
	}
}


void NodeSVG::updateAlign(){

	updateAlignStr();

	if (alignStr.empty()){
		this->unlink("data-align");
	}
	else {
		if (!this->hasKey("data-align")){
			this->link("data-align", alignStr); // (should be safe anyway)
		}
	}

	this->unlink("data-anchor");
	this->unlink("data-anchorHorz");
	this->unlink("data-anchorVert");

	if (anchorHorz.empty() && anchorVert.empty()){
		return;
	}

	if (anchorHorz == anchorVert){
		this->link("data-anchor", anchorHorz);
		return;
	}

	if (!anchorHorz.empty()){
		this->link("data-anchorHorz", anchorHorz);
	}

	if (!anchorVert.empty()){
		this->link("data-anchorVert", anchorVert);
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
	return XML::xmlSetType(*this, type);
	//this->datasetType(type);
	//this->data->handleType();
	//return *this;
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

/// Automatic conversion of element type (enum value) to a string.
/**
 *  Convenience
 */
template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const image::svg::tag_t & type){
	return (*this)[EnumDict<image::svg::tag_t>::dict.getKey(type, false)];
}


}  // drain::


