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

namespace image {

template <>
NodeSVG::xmldoc_attrib_map_t NodeSVG::xml_node_t::xmldoc_attribs = {
		{"version",  "1.0"},
		{"encoding", "UTF-8"},
		{"standalone", "no"},
		{"data-remark", "svg"}
};

const drain::FileInfo NodeSVG::fileInfo("svg");

std::string NodeSVG::xlink("http://www.w3.org/1999/xlink");
std::string NodeSVG::svg("http://www.w3.org/2000/svg");

// NodeSVG::NodeSVG(){	setType(UNDEFINED);}


template <>
std::map<svg::tag_t,std::string> NodeXML<svg::tag_t>::tags = {
	{drain::image::svg::UNDEFINED,	"UNDEFINED"},
	{drain::image::svg::COMMENT, "#"},
	{drain::image::svg::CTEXT, ""},
	{drain::image::svg::SVG,   "svg"},
	{drain::image::svg::CIRCLE,  "circle"},
	{drain::image::svg::DESC,  "desc"},
	{drain::image::svg::GROUP, "g"},
	{drain::image::svg::IMAGE, "image"},
	{drain::image::svg::LINE,  "line"},
	{drain::image::svg::METADATA,  "metadata"},
	{drain::image::svg::POLYGON,  "polygon"},
	{drain::image::svg::RECT,  "rect"},
	{drain::image::svg::STYLE, "style"}, // raise?
	{drain::image::svg::TEXT,  "text"},
	{drain::image::svg::TITLE, "title"},
	{drain::image::svg::TSPAN, "tspan"},
};


NodeSVG::NodeSVG(tag_t t){
	type = elem_t::UNDEFINED;
	setType(t);
}

NodeSVG::NodeSVG(const NodeSVG & node) : xml_node_t(), x(0), y(0), width(0), height(0), radius(0) {
	copyStruct(node, node, *this, LINK); // <-- risky! may link Variable contents?
	// type = elem_t::UNDEFINED; // = force fresh setType below
	setType(node.getType());
}


void NodeSVG::setType(const tag_t & t) {

	if (type == t){
		return; // lazy
	}

	type = t;

	switch (t) {
	case elem_t::UNDEFINED:
		break;
	case elem_t::COMMENT:
		// setComment();
		break;
	case elem_t::CTEXT:
		// setText();
		// tag = "";
		break;
	case SVG:
		//tag = "svg";
		link("x", x = 0);
		link("y", y = 0);
		link("width", width = 0);
		link("height", height = 0);
		//link("width", width = "0");
		//link("height", height = "0");
		link("xmlns", NodeSVG::svg);
		link("xmlns:svg", NodeSVG::svg);
		link("xmlns:xlink", NodeSVG::xlink);
		break;
	case TITLE:
		//tag = "title";
		break;
	case GROUP:
		// tag = "g";
		break;
	case RECT:
		// tag = "rect";
		link("x", x = 0);
		link("y", y = 0);
		link("width", width = 0);
		link("height", height = 0);
		// link("width", width = "0");
		// link("height", height = "0");
		break;
	case CIRCLE:
		// tag = "circ";
		link("cx", x = 0);
		link("cy", y = 0);
		link("r", radius = 0);
		break;
	case IMAGE:
		// tag = "image";
		link("x", x = 0);
		link("y", y = 0);
		link("width", width = 0);
		link("height", height = 0);
		//link("width", width = "0");
		// link("height", height = "0");
		// if (version == 1) {
		link("xlink:href", url); // text_anchor
		// if (version > 2.x ?) {
		//link("href", text_anchor);
		break;
	case TEXT:
		// tag = "text";
		link("x", x = 0);
		link("y", y = 0);
		// link("text-anchor", text_anchor = "");
		break;
	case TSPAN:
		// tag = "tspan";
		//link("text-anchor", text_anchor = "");
		break;
	default:
		return;
	}

	// DEPRECATING: see separate STYLE and CLASS?
	// link("style", style = "");
	//link("fill", fill = "");
	//link("opacity", opacity = ""); // string, so silent if empty


}

/*
std::ostream & NodeSVG::toStream(std::ostream &ostr, const TreeSVG & tree){
	NodeXML::toStream(ostr, tree);
	return ostr;
}
*/


}  // image::

}  // drain::

/*
template <>
struct drain::TypeName<drain::image::NodeSVG> {
    static const char* get(){ return "SVG"; }
};
*/

template <>
template <>
// inline
drain::image::TreeSVG & drain::image::TreeSVG::operator()(const drain::image::svg::tag_t & type){
	this->data.setType(type);
	return *this;
}


/*
template <>
template <>
drain::image::TreeSVG & drain::image::TreeSVG::operator()(const std::string & text){
	this->data.ctext = text;
	return *this;
}
*/

