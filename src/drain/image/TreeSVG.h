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

#ifndef DRAIN_TREESVG_H_
#define DRAIN_TREESVG_H_

#include "drain/util/FileInfo.h"
#include "drain/util/TreeXML.h"

namespace drain {

namespace image {

class NodeSVG;

//typedef drain::UnorderedMultiTree<NodeSVG> TreeSVG;
typedef drain::UnorderedMultiTree<NodeSVG,false, NodeXML<>::path_t> TreeSVG;
// cf.  drain::UnorderedMultiTree<NodeXML<E>,false, NodeXML<>::path_t>

struct BaseSVG {

	enum tag_t {
		UNDEFINED=NodeXML<>::UNDEFINED,
		COMMENT=NodeXML<>::COMMENT,
		CTEXT=NodeXML<>::CTEXT,
		SVG, TITLE, GROUP, TEXT, TSPAN, RECT, CIRC, LINE, IMAGE }; // check CTEXT, maybe implement in XML

	typedef NodeSVG node_t;

	typedef TreeSVG tree_t;

};

/// A node structure for drain::UnorderedMultiTree<NodeSVG>, compatible with TreeXML
/**
  \example TreeSVG-example.cpp

  \see drain::TreeXML

 */
class NodeSVG: public BaseSVG, public NodeXML<BaseSVG::tag_t> {
public:

	//typedef drain::Tree<NodeSVG> tree_t;

	//enum type { UNDEFINED, SVG, TITLE, CTEXT, GROUP, TEXT, RECT, CIRC, LINE, IMAGE }; // check CTEXT, maybe implement in XML

	NodeSVG(tag_t t = BaseSVG::UNDEFINED);

	/// Copy constructor.
	NodeSVG(const NodeSVG & node);

	inline virtual
	~NodeSVG(){};

	virtual
	void setType(const tag_t & type);

	static
	std::ostream & toStream(std::ostream &ostr, const TreeSVG & t);

	/// In opening SVG tag, referred to by attribute "xmlns:xlink"
	static
	std::string xlink;

	/// In opening SVG tag, referred to by attributes "xmlns" and "xmlns:svg"
	static
	std::string svg;


	static
	const drain::FileInfo fileInfo;

	inline
	NodeSVG & operator=(const NodeSVG & n){
		drain::SmartMapTools::setValues<map_t>((map_t &)*this, n);
		return *this;
	}

	inline
	NodeSVG & operator=(const tag_t & type){
		setType(type);
		return *this;
	}

	inline
	NodeSVG & operator=(const std::initializer_list<std::pair<const char *,const drain::Variable> > &l){
		set(l);
		return *this;
	}



protected:

	// svg:
	int x;
	int y;
	//std::string width; // can be "240px" or "90%" ?
	//std::string height;
	int width;
	int height;
	int radius;
	std::string style;
	std::string fill;
	std::string opacity; // empty
	std::string text_anchor;

};

//static
//NodeXML<BaseSVG::tag_t>::tags;
/*
template <>
TreeSVG & TreeSVG::operator=(const std::initializer_list<std::pair<const char *,const drain::Variable> > & l){
	// data.set(l);
	return *this;
}
*/



inline
std::ostream & operator<<(std::ostream &ostr, const TreeSVG & t){
	  return NodeSVG::toStream(ostr, t);
}


}  // image::

}  // drain::

/*
template <>
struct drain::TypeName<drain::image::TreeSVG> {
    static const char* get(){ return "TreeSVG"; }
};
*/

template <>
struct drain::TypeName<drain::image::NodeSVG> {
    static const char* get(){ return "SVG"; }
};

template <>
template <>
inline
drain::image::TreeSVG & drain::image::TreeSVG::operator()(const drain::image::BaseSVG::tag_t & type){
	this->data.setType(type);
	return *this;
}

template <>
template <>
inline
drain::image::TreeSVG & drain::image::TreeSVG::operator()(const std::string & text){
	//if (this->data.)
	this->data.ctext = text;
	return *this;
}



#endif // TREESVG_H_

