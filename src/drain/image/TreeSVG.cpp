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
		// {"data-remark", "svg"} debugging? inkview claims
};

const drain::FileInfo NodeSVG::fileInfo("svg");

std::string NodeSVG::xlink("http://www.w3.org/1999/xlink");
std::string NodeSVG::svg("http://www.w3.org/2000/svg");

// NodeSVG::NodeSVG(){	setType(UNDEFINED);}

// OLD
/*
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
*/

// NEW (slower?)
template <>
const drain::EnumDict<svg::tag_t>::dict_t drain::EnumDict<svg::tag_t>::dict = {
		/*
		{"UNDEFINED", drain::image::svg::UNDEFINED},
		{"#", drain::image::svg::COMMENT},
		{"", drain::image::svg::CTEXT},
		{"g", drain::image::svg::GROUP},
		DRAIN_ENUM_ENTRY(drain::image::svg, GROUP),
		*/
		/// TODO: complete
		{"UNDEFINED", drain::image::svg::UNDEFINED},
		{ "#", drain::image::svg::COMMENT},
		{ "", drain::image::svg::CTEXT},
		{   "svg", drain::image::svg::SVG},
		{  "circle", drain::image::svg::CIRCLE},
		{  "desc", drain::image::svg::DESC},
		{ "g", drain::image::svg::GROUP},
		{ "image", drain::image::svg::IMAGE},
		{  "line", drain::image::svg::LINE},
		{  "metadata", drain::image::svg::METADATA},
		{  "polygon", drain::image::svg::POLYGON},
		{  "rect", drain::image::svg::RECT},
		{ "style", drain::image::svg::STYLE}, // raise?
		{  "text", drain::image::svg::TEXT},
		{ "title", drain::image::svg::TITLE},
		{ "tspan", drain::image::svg::TSPAN},
};


NodeSVG::NodeSVG(tag_t t){
	// type = (intval_t) svg::UNDEFINED;
	setType(t);
}

//NodeSVG::NodeSVG(const NodeSVG & node) : xml_node_t(), x(0), y(0), width(0), height(0), radius(0) {
NodeSVG::NodeSVG(const NodeSVG & node) : xml_node_t(), box(0,0,0,0), radius(0) {
	copyStruct(node, node, *this, LINK); // <-- risky! may link Variable contents?
	// type = elem_t::UNDEFINED; // = force fresh setType below
	setType(node.getType());
}


void NodeSVG::handleType(const tag_t & t) { // setType(const elem_t & t) {

	// drain::Logger mout(drain::TypeName<NodeSVG>::str().c_str(), __FUNCTION__);
	// mout.attention(__FUNCTION__, ": current type=", type, " arg=", t);
	/*
	if (type == t){
		return; // lazy
	}
	*/
	// type = t;

	switch (t) {
	case svg::UNDEFINED:
		// case UNDEFINED:
		break;
	case svg::COMMENT:
		// setComment();
		break;
	case svg::CTEXT:
		//case XML::CTEXT:
		// setText();
		// tag = "";
		break;
	case SVG:
		//tag = "svg";
		//link("x", box.x = 0);
		//link("y", box.y = 0);
		link("width", box.width = 0);
		link("height", box.height = 0);
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
		link("x", box.x = 0);
		link("y", box.y = 0);
		link("width", box.width = 0);
		link("height", box.height = 0);
		// link("width", width = "0");
		// link("height", height = "0");
		break;
	case CIRCLE:
		// tag = "circ";
		link("cx", box.x = 0);
		link("cy", box.y = 0);
		link("r", radius = 0);
		break;
	case IMAGE:
		// tag = "image";
		link("x", box.x = 0);
		link("y", box.y = 0);
		link("width", box.width = 0);
		link("height", box.height = 0);
		//link("width", width = "0");
		// link("height", height = "0");
		// if (version == 1) {
		link("xlink:href", url); // text_anchor
		// if (version > 2.x ?) {
		//link("href", text_anchor);
		break;
	case TEXT:
		// tag = "text";
		link("x", box.x = 0);
		link("y", box.y = 0);
		// style.link("font-size", this->box.height); // Not good, shows zero size
		// link("text-anchor", text_anchor = "");
		break;
	case TSPAN:
		// style.link("font-size", this->box.height); // Not good, shows zero size
		break;
	default:
		return;
	}

	//link("pos", box.getLocation().tuple());
	// link("frm", box.getFrame().tuple());
	// DEPRECATING: see separate STYLE and CLASS?
	// link("style", style = "");
	// link("fill", fill = "");
	// link("opacity", opacity = ""); // string, so silent if empty


}

/// Needed for handling units in strings, like "50%" or "640px".
void NodeSVG::setAttribute(const std::string & key, const std::string &value){
	(*this)[key] = value;
}

/// Needed for handling units in strings, like "50%" or "640px".
void NodeSVG::setAttribute(const std::string & key, const char *value){
	(*this)[key] = value; // -> handleString()
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

	if (anchorHorz.empty()){
		this->unlink("data-alignAnchor");
	}
	else {
		this->link("data-alignAnchor", anchorHorz);
	}

}

/*
std::ostream & NodeSVG::toStream(std::ostream &ostr, const TreeSVG & tree){
	NodeXML::toStream(ostr, tree);
	return ostr;
}
*/


}  // image::

}  // drain::

// Important! Useful and widely used – but  fails with older C++ compilers ?
template <>
template <>
drain::image::TreeSVG & drain::image::TreeSVG::operator()(const drain::image::svg::tag_t & type){
	this->data.setType(type);
	return *this;
}

template <> // referring to Tree<NodeSVG>
drain::image::TreeSVG & drain::image::TreeSVG::operator=(std::initializer_list<std::pair<const char *,const char *> > l){
	// drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.attention("cchar* initlist:", sprinter(l));
	for (const auto & entry: l){
		data.setAttribute(entry.first, entry.second);
	}
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

