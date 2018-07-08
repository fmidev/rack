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
 * TreeSVG.h
 *
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#ifndef TREESVG_H_
#define TREESVG_H_

#include "util/TreeXML.h"

namespace drain {

namespace image {

/// A node for TreeXML
/**
  \example TreeSVG-example.cpp
 */
class NodeSVG: public NodeXML {
public:

	enum type { UNDEFINED, SVG, CTEXT, GROUP, TEXT, RECT, CIRC, LINE }; // check CTEXT, maybe implement in XML

	NodeSVG(type t = UNDEFINED);

	void setType(type t);

	static
	std::ostream & toOStr(std::ostream &ostr, const drain::Tree<NodeSVG> & t);

	/// In opening SVG tag, referred to by attribute "xmlns:xlink"
	static
	std::string xlink;

	/// In opening SVG tag, referred to by attributes "xmlns" and "xmlns:svg"
	static
	std::string svg;

protected:

	// svg:
	int x;
	int y;
	int width;
	int height;
	int radius;
	std::string style;
	std::string fill;
	std::string opacity; // empty
	std::string text_anchor;

};


typedef drain::Tree<NodeSVG> TreeSVG;



inline
std::ostream & operator<<(std::ostream &ostr, const TreeSVG & t){
	  return NodeSVG::toOStr(ostr, t);
}


}  // namespace image

}  // namespace drain

#endif /* TREESVG_H_ */
