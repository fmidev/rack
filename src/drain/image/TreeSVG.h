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

#include <drain/Enum.h>
#include <drain/image/AlignAnchorSVG.h>
#include "drain/util/FileInfo.h"
#include "drain/util/Frame.h"
#include "drain/util/SelectorXML.h"
#include "drain/util/TreeXML.h"
#include "drain/util/UtilsXML.h"

#include "SVG.h"
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
		PREFIXED = 2,  // file:// is appended, sometimes required?
		RELATIVE = 2,  // Duplicate, for now.
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

	static
	int radialBezierResolution;

	static
	int visualDebugLevel;

	static
	std::string DEBUG;

	static
	std::string DEBUG_ALIGN;

};

}

DRAIN_ENUM_DICT(image::FileSVG::IncludePolicy);
DRAIN_ENUM_DICT(image::FileSVG::PathPolicy);


namespace image {

DRAIN_ENUM_OSTREAM(image::FileSVG::IncludePolicy);
DRAIN_ENUM_OSTREAM(image::FileSVG::PathPolicy);


class NodeSVG;

// typedef drain::UnorderedMultiTree<NodeSVG,false, NodeXML<>::path_t> TreeSVG;
typedef DRAIN_XML_TREE(NodeSVG) TreeSVG;

typedef SelectXML<svg::tag_t> SelectSVG;


/// A node structure for drain::UnorderedMultiTree<NodeSVG>, compatible with TreeXML
/**
  \example TreeSVG-example.cpp

  \see drain::TreeXML

 */
// class NodeSVG: public svg, public NodeXML<svg::tag_t>, public AlignAnchorSVG {
class NodeSVG: public NodeXML<svg::tag_t>, public AlignSVG, public AlignAnchorSVG {
public:

	typedef svg::tag_t tag_t; // DRAIN_XML_...  macros expect (NodeXML)::tag_t

	/// In opening SVG tag, referred to by attribute "xmlns:xlink"
	static
	std::string xlink;

	/// In opening SVG tag, referred to by attributes "xmlns" and "xmlns:svg"
	static
	std::string svg_decl;


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
				svg::tag_t::COMMENT,
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


	/// Set attribute value, handling units in string arguments, like in "50%" or "640px".
	/*
	virtual
	void setAttribute(const std::string & key, const std::string &value) override;

	/// Set attribute value, handling units in string arguments, like in "50%" or "640px".
	virtual
	void setAttribute(const std::string & key, const char *value) override;
	*/


	/// Get location (x,y) and geometry (width,height) of an object.
	inline
	const BBoxSVG & getBoundingBox() const {
		return box;
	}
	// Consider also with conversion:  getBoundingBox(Box<T> &b)

	/// Get location (x,y) and geometry (width,height) of an object.
	inline
	BBoxSVG & getBoundingBox(){ // needed? (by AlignSVG, perhaps?)
		return box;
	}

	/// Set position (x,y), width and height of an object.
	template <typename T>
	inline
	void setBoundingBox(const drain::Box<T> & b){
		setLocation(b);
		setGeometry(b);
		// setAttribute("data:bbox", StringBuilder<' '>(b.x, b.y, b.getWidth(), b.getHeight()));
	}

	/// Get location (x,y) and geometry (width,height) of an object.
	inline
	const Frame2D<svg::coord_t> & getGeometry() const {
		return box.getFrame();
	}

	/// Get location (x,y) and geometry (width,height) of an object.
	inline
	Frame2D<svg::coord_t> & getGeometry() {
		return box.getFrame();
	}

	inline
	const Point2D<svg::coord_t> & getLocation() const {
		return box.getLocation();
	}

	inline
	Point2D<svg::coord_t> & getLocation() {
		return box.getLocation();
	}

	template <typename T>
	inline
	void setViewBox(const drain::Box<T> & bb){
		setViewBox(bb.x, bb.y, bb.width, bb.height);
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
	void setGeometry(const drain::Frame2D<T> & frame){
		box.width  = frame.width;
		box.height = frame.height;
	}

	/**
	 *
	 *  Future option: std::string args.
	 */
	template <typename T>
	inline
	void setGeometry(const T & w, const T & h){
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
	svg::coord_t getWidth() const {
		return box.width;
	}

	/// Set margin of a TEXT element (non-standard).
	/**
	 *
	 */
	template <typename T>
	inline
	void setMargin(T w){
		// getMap()["data-margin"].link(box.width = w);
		//getMap()["data-margin"].link(margin = w);
		setUserAttribute("margin", w);
	}

	/// Get margin of a TEXT element (non-standard).
	/**
	 *
	 */
	inline
	svg::coord_t getMargin() const {
		//return box.width;
		// return margin;
		return getUserAttribute("margin");
	}


	/**
	 *
	 *  Future option: std::string arg, allowing units.
	 */
	template <typename T>
	inline
	void setHeight(T h){
		if (typeIs(svg::TEXT)){
			getMap()["data-height"].link(box.height);
		}
		box.height = h;
	}

	inline
	svg::coord_t getHeight() const {
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

	/// Swaps XML properties and also alignment, anchors etc.
	void swapDataSVG(NodeSVG & node);

protected:

	virtual
	void handleType() override final;


	virtual
	void updateAlign() override;

	// TODO: allowedUnits drain::Units
	BBoxSVG box;

	virtual inline
	void setAlignClass(AlignClass cls) override final {
		addClass(cls);
	};

	// For TEXT elems only...
	// svg::coord_t margin = 0;

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
template <> // for T - Tree class
template <> // for K - operator() argument
image::TreeSVG & image::TreeSVG::operator()(const image::svg::tag_t & type);
 */

DRAIN_XML_EASY_TYPE(image::TreeSVG);

DRAIN_GETKEY_ENUM(image::TreeSVG, image::svg::tag_t);


template <> // for T (Tree class)
template <> // for K (path elem arg)
inline
const image::TreeSVG::key_t & image::TreeSVG::getKey(const ClassXML & cls){
	//return image::TreeSVG::getKey(cls.strPrefixed()); // const !
	//return image::TreeSVG::getKey((const std::string &)cls); // const !
	return (const std::string &)cls;
}

template <> // for T (Tree class)
template <> // for K (path elem arg)
inline
const image::TreeSVG::key_t & image::TreeSVG::getKey(const drain::SelectXML<image::svg::tag_t> & sel){
	// return image::TreeSVG::getKey(sel.str()); // const !
	return (const std::string &)sel;
}



DRAIN_XML_DEFAULT_ELEMS(image::TreeSVG);

/// Specialization of default child elements in SVG. For example, \c TEXT has \c TSPAN elements, by default.
/**
 *   Relates to the above list of default elements.
 */
DRAIN_XML_DEFAULT_INIT(image::TreeSVG);

// const drain::Enum<image::svg::tag_t>::dict_t & getDict();
template <>
void image::TreeSVG::swapData(image::NodeSVG & node);


/// Add child of given type
/**
 *  Note that in UnorderedTree, the children are not unique.
 *
 *  TODO: repeat with HTML?
 */
template <> // for T - Tree class
template <> // for K - argument
image::TreeSVG & image::TreeSVG::addChild(const image::svg::tag_t & type);

} // drain::

#endif // DRAIN_TREE_SVG

