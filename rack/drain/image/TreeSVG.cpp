/*

    Copyright 2001 - 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

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
		reference("x", x = 0);
		reference("y", y = 0);
		reference("width", width = 0);
		reference("height", height = 0);
		reference("xmlns", NodeSVG::svg);
		reference("xmlns:svg", NodeSVG::svg);
		reference("xmlns:xlink", NodeSVG::xlink);
		break;
	case GROUP:
		tag = "g";
		//reference("x", x, 0);
		//reference("y", y, 0);
		break;
	case TEXT:
		tag = "text";
		reference("x", x = 0);
		reference("y", y = 0);
		reference("text-anchor", text_anchor = "");
		break;
	case RECT:
		tag = "rect";
		reference("x", x = 0);
		reference("y", y = 0);
		reference("width", width = 0);
		reference("height", height = 0);
		break;
	case CIRC:
		tag = "circ";
		reference("x", x = 0);
		reference("y", y = 0);
		reference("radius", radius = 0);
		break;
	case CTEXT:
		tag = "";
		//reference("x", x, 0);
		break;
	case UNDEFINED:
	default:
		return;
	}

	//if ((t = TEXT)||(t == TEXT)){
	reference("style", style = "");
	reference("fill", fill = "");
	reference("opacity", opacity = ""); // string, so silent if empty


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

