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

#include "drain/util/EnumFlags.h"
#include "drain/util/FileInfo.h"
#include "drain/util/Frame.h"
#include "drain/util/TreeXML.h"
#include "AlignSVG.h"

namespace drain {

namespace image {

class NodeSVG;

typedef drain::UnorderedMultiTree<NodeSVG,false, NodeXML<>::path_t> TreeSVG;

struct svg {

	typedef int coord_t;

	enum tag_t {
		UNDEFINED=XML::UNDEFINED,
		COMMENT=XML::COMMENT,
		CTEXT=XML::CTEXT,
		SCRIPT=XML::SCRIPT,
		STYLE=XML::STYLE,
		STYLE_SELECT=XML::STYLE_SELECT,
		SVG=10,
		CIRCLE, DESC, GROUP, LINE, IMAGE, METADATA, POLYGON, RECT, TEXT, TITLE, TSPAN };
	// check CTEXT, maybe implement in XML

};

} // image::


const drain::EnumDict<image::svg::tag_t>::dict_t & getDict();


template <>
const EnumDict<image::svg::tag_t>::dict_t EnumDict<image::svg::tag_t>::dict;

DRAIN_ENUM_OSTREAM(image::svg::tag_t)

namespace image {

// Future option
class BBoxSVG : public drain::Box<svg::coord_t> {

public:

	inline
	BBoxSVG(svg::coord_t x=0, svg::coord_t y=0, svg::coord_t width=0, svg::coord_t height=0) : drain::Box<svg::coord_t>(x, y, width, height)  {
	}

	inline
	BBoxSVG(const BBoxSVG & bbox) : drain::Box<svg::coord_t>(bbox)  {
	}

	// Future option
	bool x_PERCENTAGE = false;
	bool y_PERCENTAGE = false;
	bool width_PERCENTAGE = false;
	bool height_PERCENTAGE = false;

};

/// A node structure for drain::UnorderedMultiTree<NodeSVG>, compatible with TreeXML
/**
  \example TreeSVG-example.cpp

  \see drain::TreeXML

 */
// class NodeSVG: public svg, public NodeXML<svg::tag_t>, public AlignAdapterSVG {
class NodeSVG: public NodeXML<svg::tag_t>, public AlignAdapterSVG {
public:

	/// In opening SVG tag, referred to by attribute "xmlns:xlink"
	static
	std::string xlink;

	/// In opening SVG tag, referred to by attributes "xmlns" and "xmlns:svg"
	static
	std::string svg;

	static
	const drain::FileInfo fileInfo;


	/// Default constructor. Create a node of given type.
	NodeSVG(svg::tag_t t = svg::UNDEFINED);

	/// Copy constructor.
	NodeSVG(const NodeSVG & node);

	inline virtual
	~NodeSVG(){};

	inline
	bool isAbstract(){
		return typeIs(
				svg::tag_t::STYLE,
				svg::tag_t::DESC,
				svg::tag_t::METADATA,
				svg::tag_t::SCRIPT,
				svg::tag_t::TITLE,
				svg::tag_t::TSPAN
		);
	}

	/// Copy data from a node. (Does not copy subtree.)
	inline
	NodeSVG & operator=(const NodeSVG & node){
		XML::xmlAssignNode(*this, node);
		/*
		if (&node != this){
			XML::xmlAssignNode(*this, node);
			setType(n.getType()); // 2025
			drain::SmartMapTools::setValues<map_t>((map_t &)*this, n);
		}
		*/
		return *this;
	}

	/// Copy data from a node. (Does not copy subtree.)
	inline
	NodeSVG & operator=(const std::initializer_list<Variable::init_pair_t > &l){
		set(l);
		return *this;
	}

	template <class T>
	inline
	NodeSVG & operator=(const T & arg){
		set(arg);
		//assign(arg);
		return *this;
	}



	/// Set attribute value, handling units in string arguments, like in "50%" or "640px".
	virtual
	void setAttribute(const std::string & key, const std::string &value) override;

	/// Set attribute value, handling units in string arguments, like in "50%" or "640px".
	virtual
	void setAttribute(const std::string & key, const char *value) override;



	/// Get position (x,y), width and height of an object.
	inline
	const BBoxSVG & getBoundingBox() const {
		return box;
	}
	// Consider also with conversion:  getBoundingBox(Box<T> &b)

	/// Get position (x,y), width and height of an object.
	inline
	BBoxSVG & getBoundingBox(){
		return box;
	}

	/// Set position (x,y), width and height of an object.
	template <typename T>
	inline
	void setBoundingBox(const drain::Box<T> & b){
		setLocation(b);
		setFrame(b);
		// setAttribute("data:bbox", StringBuilder<' '>(b.x, b.y, b.getWidth(), b.getHeight()));
	}

	/// Set position (x,y) of an object.
	template <typename T>
	inline
	void setLocation(const drain::Point2D<T> & point){
		box.x = point.x;
		box.y = point.y;
	}

	/**
	 *
	 *  Future option: std::string args.
	 */
	template <typename T>
	inline
	void setLocation(const T & x, const T & y){
		box.setLocation(x, y);
	}

	template <typename T>
	inline
	void setFrame(const drain::Frame2D<T> & frame){
		box.width  = frame.width;
		box.height = frame.height;
	}

	/**
	 *
	 *  Future option: std::string args.
	 */
	template <typename T>
	inline
	void setFrame(const T & w, const T & h){
		box.setArea(w, h);
	}


	/**
	 *
	 *  Future option: std::string arg, allowing units.
	 */
	template <typename T>
	inline
	void setWidth(T w){
		box.width  = w;
	}

	inline
	svg::coord_t getWidth(){
		return box.width;
	}

	/// Set margin of a TEXT element (non-standard).
	/**
	 *
	 */
	template <typename T>
	inline
	void setMargin(T w){
		link("data-margin", box.width);
		box.width = w;
	}

	/// Get margin of a TEXT element (non-standard).
	/**
	 *
	 */
	inline
	svg::coord_t getMargin(){
		return box.width;
	}


	/**
	 *
	 *  Future option: std::string arg, allowing units.
	 */
	template <typename T>
	inline
	void setHeight(T h){
		box.height = h;
	}

	inline
	svg::coord_t getHeight(){
		return box.height;
	}


protected:

	///
	/**
	 *  Special: for TEXT and SPAN elements, links STYLE[font-size] to bbox.height?
	 */
	virtual
	void handleType(const svg::tag_t & t) override final;

	virtual
	void updateAlign() override;

	//drain::Box<coord_t> box;
	BBoxSVG box;
	// consider:
	// bool x_PERCENTAGE = false;
	// bool y_PERCENTAGE = false;
	// svg:

	int radius = 0;

};


/*
template <typename P, typename A,typename V>
void NodeSVG::setAlign(const P & pos, const A & axis,  const V &value){
	alignments[p][a] = v;
}
*/

}  // image::

// template <>
// void NodeXML<image::svg::tag_t>::handleType(const image::svg::tag_t & type);

}  // drain::


inline
std::ostream & operator<<(std::ostream &ostr, const drain::image::TreeSVG & tree){
	//return drain::NodeXML<const drain::image::NodeSVG>::docToStream(ostr, tree);
	return drain::image::NodeSVG::docToStream(ostr, tree);
	//return drain::image::TreeSVG::node_data_t::docToStream(ostr, tree);
}



namespace drain {

DRAIN_TYPENAME(image::NodeSVG);
DRAIN_TYPENAME(image::svg::tag_t);


template <>
const NodeXML<image::svg::tag_t>::xml_default_elem_map_t NodeXML<image::svg::tag_t>::xml_default_elems;

/*
template <>
template <typename K, typename V>
image::TreeSVG & image::TreeSVG::operator=(std::initializer_list<std::pair<K,V> > args){
	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.attention("initlist pair<K,V>: ", args);
	data.set(args); // what about TreeSVG & arg
	return *this;
}
*/

/*
template <> // referring to Tree<NodeSVG>
image::TreeSVG & image::TreeSVG::operator=(std::initializer_list<std::pair<const char *,const char *> > l);
*/

template <> // referring to Tree<NodeSVG>
inline
image::TreeSVG & image::TreeSVG::operator=(std::initializer_list<std::pair<const char *,const Variable> > l){
//image::TreeSVG & image::TreeSVG::operator=(std::initializer_list<std::pair<const char *,const char *> > l){
	return XML::xmlAssign(*this, l);
}


template <>
template <class T>
image::TreeSVG & image::TreeSVG::operator=(const T & arg){
	return XML::xmlAssign(*this, arg);
	/*
	data.set(arg); // what about TreeSVG & arg
	return *this;
	*/
}

// Important! Useful and widely used – but  fails with older C++ compilers ?
template <>
template <>
inline
image::TreeSVG & image::TreeSVG::operator()(const image::svg::tag_t & type){
		return XML::xmlSetType(*this, type);
}

template <>
inline
image::TreeSVG & image::TreeSVG::addChild(const image::TreeSVG::key_t & key){
	return XML::xmlAddChild(*this, key);
}

/*
template <>
template <class K>
const image::TreeSVG::key_t & image::TreeSVG::getKey(const K & key){
	return EnumDict<K>::dict.getKey(key, false);
}
*/

/*  tested 2025/01/20 but caused problems with Hi5Tree("dataset2") etc.
template <>
template <typename K> // for K (path elem arg)
const image::TreeSVG::key_t & image::TreeSVG::getKey(const K & key){
	return EnumDict<K>::dict.getKey(key, false);
}

template <>
template <> // for K (path elem arg)
inline
const image::TreeSVG::key_t & image::TreeSVG::getKey(const drain::StyleSelectorXML<image::NodeSVG> & selector){
	return selector;
}

*/

template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const image::svg::tag_t & type) const;


template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const image::svg::tag_t & type);

template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const image::svg::tag_t & type) const ;

} // drain::

#endif // DRAIN_TREE_SVG

