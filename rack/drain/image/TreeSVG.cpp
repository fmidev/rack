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

std::string NodeSVG::xlink("http://www.w3.org/1999/xlink");
std::string NodeSVG::svg("http://www.w3.org/2000/svg");

// NodeSVG::NodeSVG(){	setType(UNDEFINED);}

NodeSVG::NodeSVG(type t){
	setType(t);
}

void NodeSVG::setType(type t) {

	switch (t) {
	case SVG:
		tag = "svg";
		link("x", x = 0);
		link("y", y = 0);
		link("width", width = 0);
		link("height", height = 0);
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
	case TEXT:
		tag = "text";
		link("x", x = 0);
		link("y", y = 0);
		link("text-anchor", text_anchor = "");
		break;
	case RECT:
		tag = "rect";
		link("x", x = 0);
		link("y", y = 0);
		link("width", width = 0);
		link("height", height = 0);
		break;
	case CIRC:
		tag = "circ";
		link("x", x = 0);
		link("y", y = 0);
		link("radius", radius = 0);
		break;
	case CTEXT:
		tag = "";
		//link("x", x, 0);
		break;
	case UNDEFINED:
	default:
		return;
	}

	//if ((t = TEXT)||(t == TEXT)){
	link("style", style = "");
	link("fill", fill = "");
	link("opacity", opacity = ""); // string, so silent if empty


}

std::ostream & NodeSVG::toOStr(std::ostream &ostr, const TreeSVG & tree){
	ostr << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>";
	ostr << '\n';
	//NodeXML::toOStr()
	//NodeXML::toOStr(ostr, tree, "svg");
	NodeXML::toOStr(ostr, tree);
	return ostr;
}



}  // namespace image

}  // namespace drain


// Rack
