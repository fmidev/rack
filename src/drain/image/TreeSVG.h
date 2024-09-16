/*

MIT License

Copyright (c) 2023 FMI Open Development / Markus Peura, first.last@fmi.fi

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
 * TreeSVG.h
 *
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#ifndef DRAIN_TREE_SVG
#define DRAIN_TREE_SVG

#include "drain/util/FileInfo.h"
#include "drain/util/TreeXML.h"

namespace drain {

namespace image {

class NodeSVG;

typedef drain::UnorderedMultiTree<NodeSVG,false, NodeXML<>::path_t> TreeSVG;

struct svg {

	enum tag_t {
		UNDEFINED=NodeXML<>::UNDEFINED,
		COMMENT=NodeXML<>::COMMENT,
		CTEXT=NodeXML<>::CTEXT,
		STYLE=NodeXML<>::STYLE,
		SCRIPT=NodeXML<>::SCRIPT,
		SVG, CIRCLE, DESC, GROUP, LINE, IMAGE, METADATA, POLYGON, RECT, TEXT, TITLE, TSPAN };
	// check CTEXT, maybe implement in XML


};

/// A node structure for drain::UnorderedMultiTree<NodeSVG>, compatible with TreeXML
/**
  \example TreeSVG-example.cpp

  \see drain::TreeXML

 */
class NodeSVG: public svg, public NodeXML<svg::tag_t> {
public:

	// typedef NodeXML<svg::tag_t> xml_node_t;

	/// Default constructor. Create a node of given type.
	NodeSVG(tag_t t = svg::UNDEFINED);

	/// Copy constructor.
	NodeSVG(const NodeSVG & node);

	inline virtual
	~NodeSVG(){};

	///
	virtual
	void setType(const tag_t & type);

	/* Consider this later, for user-defined (not enumerated) tag types.
	virtual
	void setType(const std::string & type);
	*/

	/// Copy data from a node. (Does not copy subtree.)
	inline
	NodeSVG & operator=(const NodeSVG & n){
		if (&n != this){
			drain::SmartMapTools::setValues<map_t>((map_t &)*this, n);
		}
		return *this;
	}

	/// Set type.
	inline
	NodeSVG & operator=(const tag_t & type){
		setType(type);
		return *this;
	}

	/// Set text (CTEXT).
	inline
	NodeSVG & operator=(const char *s){
		setText(s);
		return *this;
	}

	/// Set text (CTEXT).
	inline
	NodeSVG & operator=(const std::string &s){
		setText(s);
		return *this;
	}

	/// Set attributes.
	inline
	NodeSVG & operator=(const std::initializer_list<Variable::init_pair_t > &l){
		set(l);
		return *this;
	}


	/// In opening SVG tag, referred to by attribute "xmlns:xlink"
	static
	std::string xlink;

	/// In opening SVG tag, referred to by attributes "xmlns" and "xmlns:svg"
	static
	std::string svg;

	static
	const drain::FileInfo fileInfo;


protected:

	// svg:
	int x;
	int y;
	//std::string width; // can be "240px" or "90%" ?
	//std::string height;
	int width;
	int height;
	int radius;
	// std::string style;
	std::string fill;
	std::string opacity; // empty
	std::string text_anchor;

};




}  // image::

}  // drain::



inline
std::ostream & operator<<(std::ostream &ostr, const drain::image::TreeSVG & tree){
	return drain::NodeXML<>::docToStream(ostr, tree);
	//return drain::image::TreeSVG::node_data_t::docToStream(ostr, tree);
}


namespace drain {

DRAIN_TYPENAME(image::NodeSVG);

template <>
template <>
image::TreeSVG & image::TreeSVG::operator()(const image::svg::tag_t & type);
/*{
	this->data.setType(type);
	return *this;
}*/


}


#endif // TREESVG_H_

