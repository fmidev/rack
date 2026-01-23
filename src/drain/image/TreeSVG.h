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

#include <drain/image/AlignAnchorSVG.h>
#include <drain/util/EnumUtils.h>
#include "drain/util/FileInfo.h"
#include "drain/util/Frame.h"
#include "drain/util/SelectorXML.h"
#include "drain/util/TreeXML.h"
#include "drain/util/UtilsXML.h"
#include "TransformSVG.h"

namespace drain {

namespace image {

/*
class NodeSVG;

typedef drain::UnorderedMultiTree<NodeSVG,false, NodeXML<>::path_t> TreeSVG;
//typedef NodeXML<NodeSVG>::xml_tree_t TreeSVG;
*/

class FileSVG {

public:

	static
	const drain::FileInfo fileInfo;

	enum PathPolicy {
		ABSOLUTE = 1,  // Else is relative, stripped away using inputPrefix?
		PREFIXED = 2,  // file:// is appended
	};

	// Rename IncludePolicy ?
	enum IncludePolicy {
		NONE = 0,
		PNG = 1,  // Main title in SVG image
		SVG = 2,
		TXT = 4,
		SKIP = 16,
		NEXT = 32,
		ON   = 64,
		OFF  = 128,
		ALL = ON|(PNG|SVG|TXT),
		UNKNOWN = 255,
		// --- unused ? ---
		// TITLE,      // Default title
	};

};

}

DRAIN_ENUM_DICT(image::FileSVG::IncludePolicy);
DRAIN_ENUM_OSTREAM(image::FileSVG::IncludePolicy);

DRAIN_ENUM_DICT(image::FileSVG::PathPolicy);
DRAIN_ENUM_OSTREAM(image::FileSVG::PathPolicy);

namespace image {

struct svg {

	typedef float coord_t;

	enum tag_t {
		UNDEFINED=XML::UNDEFINED,
		COMMENT=XML::COMMENT,
		CTEXT=XML::CTEXT,
		SCRIPT=XML::SCRIPT,
		STYLE=XML::STYLE,
		STYLE_SELECT=XML::STYLE_SELECT,
		SVG=10,
		CIRCLE, CLIP_PATH, DEFS, DESC, GROUP, IMAGE, LINE, LINEAR_GRADIENT, MASK, METADATA, PATH, POLYGON, RECT, TEXT, TITLE, TSPAN };
	// check CTEXT, maybe implement in XML

};

class NodeSVG;

// typedef drain::UnorderedMultiTree<NodeSVG,false, NodeXML<svg::tag_t>::path_t> TreeSVG;
typedef drain::UnorderedMultiTree<NodeSVG,false, NodeXML<>::path_t> TreeSVG;


} // image::


// const drain::EnumDict<image::svg::tag_t>::dict_t & getDict();


// template <>
// const EnumDict<image::svg::tag_t>::dict_t EnumDict<image::svg::tag_t>::dict;
DRAIN_ENUM_DICT(image::svg::tag_t);

DRAIN_ENUM_OSTREAM(image::svg::tag_t)



namespace image {

class BBoxSVG : public drain::Box<svg::coord_t> {

public:

	inline
	BBoxSVG(svg::coord_t x=0, svg::coord_t y=0, svg::coord_t width=0, svg::coord_t height=0) : drain::Box<svg::coord_t>(x, y, width, height)  {
	}

	inline
	BBoxSVG(const BBoxSVG & bbox) : drain::Box<svg::coord_t>(bbox)  {
	}


	// New



	// Future option - also other units!
	/*
	bool x_PERCENTAGE = false;
	bool y_PERCENTAGE = false;
	bool width_PERCENTAGE = false;
	bool height_PERCENTAGE = false;
	*/
};


//typedef drain::StyleSelectorXML<NodeSVG> SelectSVG;
// typedef drain::SelectorXML SelectSVG;

/// A node structure for drain::UnorderedMultiTree<NodeSVG>, compatible with TreeXML
/**
  \example TreeSVG-example.cpp

  \see drain::TreeXML

 */
// class NodeSVG: public svg, public NodeXML<svg::tag_t>, public AlignAnchorSVG {
class NodeSVG: public NodeXML<svg::tag_t>, public AlignSVG, public AlignAnchorSVG {
public:

	/// In opening SVG tag, referred to by attribute "xmlns:xlink"
	static
	std::string xlink;

	/// In opening SVG tag, referred to by attributes "xmlns" and "xmlns:svg"
	static
	std::string svg;

	/* -> FileSVG
	static
	const drain::FileInfo fileInfo;
	*/

	/// Default constructor. Create a node of given type.
	NodeSVG(svg::tag_t t = svg::UNDEFINED);

	/// Copy constructor.
	NodeSVG(const NodeSVG & node);

	inline virtual
	~NodeSVG(){};

	inline
	bool isAbstract() const {
		return typeIs(
				svg::tag_t::UNDEFINED,
				svg::tag_t::STYLE,
				svg::tag_t::CLIP_PATH, // Semi-abstract..
				svg::tag_t::DESC,
				svg::tag_t::DEFS,
				svg::tag_t::METADATA,
				svg::tag_t::SCRIPT,
				svg::tag_t::TITLE,
				svg::tag_t::TSPAN
		);
	}

	/// If true, render always as single elem (without child elems)
	virtual inline
	bool isSingular() const override final {
		return false;
	}


	/// Copy data from a node. (Does not copy subtree.)
	inline
	NodeSVG & operator=(const NodeSVG & node){
		if (!typeIsSet()){
			setType(node.getType());
			handleType();
			//handleType(node.getNativeType());
		}
		XML::xmlAssignNode(*this, node);
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

	inline
	NodeSVG & operator=(xml_tag_t type){
		setType(type);
		return *this;
	}

	// Dangerous, if XML has codes not registered by SVG?
	/*
	inline
	NodeSVG & operator=(xml_tag_t type){
		setType(type);
		return *this;
	}
	*/


	/* Well, every graphic obj may have DESC and TITLE?
	virtual
	bool isSingular() const override final;
	*/

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

	template <typename T>
	inline
	void setViewBox(const drain::Box<T> & bb){
		setViewBox(bb.x, bb.y, bb.width, bb.height);
		//set("viewBox", drain::StringBuilder<' '>(bb.x, bb.y, bb.width, bb.height));
		// setAttribute("data:bbox", StringBuilder<' '>(b.x, b.y, b.getWidth(), b.getHeight()));
	}

	template <typename T>
	inline
	void setViewBox(T & x, T & y, T & width, T & height){
		set("viewBox", drain::StringBuilder<' '>(x, y, width, height).str());
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
		if (typeIs(svg::TEXT)){
			link("data-height", box.height);
		}
		box.height = h;
	}

	inline
	svg::coord_t getHeight(){
		return box.height;
	}

	/// Sets font size and also text elem "height".
	void setFontSize(svg::coord_t size, svg::coord_t elemHeight = 0.0);

	TransformSVG transform;

	/// Write transform, in addition to XML::ClassList.
	/**
	 *  Writes special attributes:
	 *  - class="..." as defined XMK::specificAttributesToStream()
	 *  - transform="..." geometric transformations – only if defined.
	 *
	 */
	virtual
	void specificAttributesToStream(std::ostream & ostr) const override;


	void swapSVG(NodeSVG & node);

protected:

	virtual
	void handleType() override final;


	virtual
	void updateAlign() override;

	//drain::Box<coord_t> box;
	BBoxSVG box;
	// consider:
	// bool x_PERCENTAGE = false;
	// bool y_PERCENTAGE = false;
	// svg:

	// int radius = 0;

};


/*
template <typename P, typename A,typename V>
void NodeSVG::setAlign(const P & pos, const A & axis,  const V &value){
	alignments[p][a] = v;
}
*/


// Utility (move to utils?)
class NodePrinter {

public:

	inline
	NodePrinter(const NodeSVG & node){
		std::stringstream sstr;
		sstr << "<" << node.getTag();
		if (::atoi(node.getId().c_str())==0){
			sstr << " id=" << node.getId();
		}
		if (node.getName().isValid()){
			sstr << " name=" << node.getName();
		}
		if (!node.getClasses().empty()){
			sstr << " class=[" << node.getClasses() << ']';
		}
		sstr << '>';
		id = sstr.str();
	}

	const std::string & str() const {
		return id;
	}


protected:

	std::string id;

};


//typedef NodeSVG::xml_tree_t TreeSVG;


}  // image::


/*
inline
std::ostream & operator<<(std::ostream &ostr, const image::NodeSVG & node){
	return node.nodeToStream(ostr);
}
*/



}  // drain::


inline
std::ostream & operator<<(std::ostream &ostr, const drain::image::NodeSVG & node){
	return node.nodeToStream(ostr);
}



inline
std::ostream & operator<<(std::ostream &ostr, const drain::image::TreeSVG & tree){
	return drain::image::NodeSVG::docToStream(ostr, tree);
}



namespace drain {

DRAIN_TYPENAME(image::NodeSVG);
DRAIN_TYPENAME(image::svg::tag_t);



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


template <> // referring to Tree<NodeSVG>
template <> // referring to tparam T
inline
image::TreeSVG & image::TreeSVG::operator=(const std::string & arg){
	UtilsXML::assignString(*this, arg);
	return *this;
}


template <> // referring to Tree<NodeSVG>
inline
image::TreeSVG & image::TreeSVG::operator=(std::initializer_list<std::pair<const char *,const Variable> > l){
	return UtilsXML::assign(*this, l);
}


template <>
template <class T>
image::TreeSVG & image::TreeSVG::operator=(const T & arg){
	return UtilsXML::assign(*this, arg);
}

/// Handy in setting the type.
/**
 *  Usage:
 *    TreeSVG & child = tree[path_elem](type);
 *    TreeSVG & child = tree(path)(type);
 *
 *   Note: fails with older C++ compilers ?
 */
template <> // for T - Tree class
template <> // for K - operator() argument
image::TreeSVG & image::TreeSVG::operator()(const image::svg::tag_t & type);


template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const image::svg::tag_t & type) const;

/// Automatic conversion of element type (enum value) to a string.
template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const image::svg::tag_t & type);



/// Automatic conversion of element type (enum value) to a string.
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const image::svg::tag_t & type) const ;


template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const ClassXML & cls) const;

template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const ClassXML & cls);

/// Automatic conversion of element type (enum value) to a string.
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const ClassXML & cls) const ;



template <>
inline
image::TreeSVG & image::TreeSVG::addChild(){
	return UtilsXML::addChild(*this);
}



template <>
const NodeXML<image::svg::tag_t>::xml_default_elem_map_t NodeXML<image::svg::tag_t>::xml_default_elems;

/// Specialization of default child elements in SVG. For example, \c TEXT has \c TSPAN elements, by default.
/**
 *   Relates to the above list of default elements.
 */
template <>
inline
void image::TreeSVG::initChild(image::TreeSVG & child) const {
	const typename image::svg::tag_t type = UtilsXML::retrieveDefaultType(this->data);
	if (type){
		child->setType(type);
	}
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

} // drain::

#endif // DRAIN_TREE_SVG

