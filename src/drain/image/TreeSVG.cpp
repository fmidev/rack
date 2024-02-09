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

namespace image {

const drain::FileInfo NodeSVG::fileinfo("svg");

std::string NodeSVG::xlink("http://www.w3.org/1999/xlink");
std::string NodeSVG::svg("http://www.w3.org/2000/svg");

// NodeSVG::NodeSVG(){	setType(UNDEFINED);}


template <>
std::map<BaseSVG::tag_t,std::string> NodeXML<BaseSVG::tag_t>::tags = {
	{drain::image::BaseSVG::UNDEFINED,	"UNDEFINED"},
	{drain::image::BaseSVG::SVG,   "svg"},
	{drain::image::BaseSVG::TITLE, "title"},
	{drain::image::BaseSVG::CTEXT, "ctext"},
	{drain::image::BaseSVG::GROUP, "group"},
	{drain::image::BaseSVG::TEXT,  "text"},
	{drain::image::BaseSVG::TSPAN, "tspan"},
	{drain::image::BaseSVG::RECT,  "rect"},
	{drain::image::BaseSVG::CIRC,  "circ"},
	{drain::image::BaseSVG::LINE,  "line"},
	{drain::image::BaseSVG::IMAGE, "image"}
};

NodeSVG::NodeSVG(const NodeSVG & node) : x(0), y(0), width(0), height(0), radius(0) {
	//copyStruct(node, node, *this, RESERVE);
	copyStruct(node, node, *this, LINK);
	type = UNDEFINED;
	setType(node.getType());
}


NodeSVG::NodeSVG(tag_t t){
	type = UNDEFINED;
	setType(t);
}

void NodeSVG::setType(const tag_t & t) {

	if (type == t){
		return; // lazy
	}

	type = t;

	switch (t) {
	case SVG:
		tag = "svg";
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
		tag = "title";
		break;
	case GROUP:
		tag = "g";
		break;
	case RECT:
		tag = "rect";
		link("x", x = 0);
		link("y", y = 0);
		link("width", width = 0);
		link("height", height = 0);
		// link("width", width = "0");
		// link("height", height = "0");
		break;
	case CIRC:
		tag = "circ";
		link("x", x = 0);
		link("y", y = 0);
		link("radius", radius = 0);
		break;
	case IMAGE:
		tag = "image";
		link("x", x = 0);
		link("y", y = 0);
		link("width", width = 0);
		link("height", height = 0);
		//link("width", width = "0");
		// link("height", height = "0");
		// if (version == 1) {
		link("xlink:href", text_anchor);
		// if (version > 2.x ?) {
		//link("href", text_anchor);
		break;
	case TEXT:
		tag = "text";
		link("x", x = 0);
		link("y", y = 0);
		link("text-anchor", text_anchor = "");
		break;
	case TSPAN:
		tag = "tspan";
		//link("text-anchor", text_anchor = "");
		break;
	case CTEXT:
		tag = "";
		//link("x", x, 0);
		break;
	case UNDEFINED:
	default:
		return;
	}

	// DEPRECATING: see separate STYLE and CLASS?
	link("style", style = "");
	link("fill", fill = "");
	link("opacity", opacity = ""); // string, so silent if empty


}

std::ostream & NodeSVG::toStream(std::ostream &ostr, const TreeSVG & tree){
	//ostr << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>";
	ostr << "<?xml ";
	attribToStream(ostr, "version",  "1.0");
	attribToStream(ostr, "encoding", "UTF-8");
	attribToStream(ostr, "standalone", "no");
	ostr << "?>";
	ostr << '\n';
	NodeXML::toStream(ostr, tree);
	return ostr;
}



}  // image::

}  // drain::
