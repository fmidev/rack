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
#include "drain/util/Frame.h"
#include "drain/util/TreeXML.h"

namespace drain {

namespace image {

class NodeSVG;

typedef drain::UnorderedMultiTree<NodeSVG,false, NodeXML<>::path_t> TreeSVG;

struct svg {

	typedef int coord_t;

	enum tag_t {
		UNDEFINED=NodeXML<>::UNDEFINED,
		COMMENT=NodeXML<>::COMMENT,
		CTEXT=NodeXML<>::CTEXT,
		STYLE=NodeXML<>::STYLE,
		SCRIPT=NodeXML<>::SCRIPT,
		SVG, CIRCLE, DESC, GROUP, LINE, IMAGE, METADATA, POLYGON, RECT, TEXT, TITLE, TSPAN };
	// check CTEXT, maybe implement in XML

};

struct AlignSVG2 {

	enum pos_t {
		ORIG=0,
		REF=1,
	};

	enum axis_t {
		HORZ=0,
		VERT=1,
	};

	enum value_t {
		UNDEFINED,
		MIN,
		MID,
		MAX,
		// ABSOLUTE?
	};

	/*
	typedef EnumFlagger<SingleFlagger<AlignSVG2::value_t> > Alignment;
	typedef EnumFlagger<SingleFlagger<AlignSVG2::axis_t> > Axis;
	typedef EnumFlagger<SingleFlagger<AlignSVG2::pos_t> > Position;
	*/

	template <typename V>
	static inline
	const V & getValue(const std::string &key){
		return EnumDict<V>::dict.getValue(key);
	}

	template <typename V>
	static inline
	const V & getValue(const char &key){
		return EnumDict<V>::dict.getValue(key);
	}

	template <typename V>
	static inline
	const V & getValue(const V &value){
		return value;
	}

	// typedef UniTuple<value_t,2> align;
};

//template <typename POS,typename AX>
//class ASVG : public Align


template<>
const FlagResolver::dict_t EnumDict<AlignSVG2::value_t>::dict;

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
class NodeSVG: public svg, public NodeXML<svg::tag_t> {
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
	NodeSVG(tag_t t = svg::UNDEFINED);

	/// Copy constructor.
	NodeSVG(const NodeSVG & node);

	inline virtual
	~NodeSVG(){};

	///
	virtual
	void setType(const tag_t & type) override final;

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

	/// Set attribute value, handling units in string arguments, like in "50%" or "640px".
	virtual
	void setAttribute(const std::string & key, const std::string &value);

	/// Set attribute value, handling units in string arguments, like in "50%" or "640px".
	virtual
	void setAttribute(const std::string & key, const char *value);



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

	/// Set position (x,y) of an object.
	template <typename T>
	inline
	void setLocation(const drain::Point2D<T> & point){
		box.x = point.x;
		box.y = point.y;
	}

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

	/// Set position (x,y), width and height of an object.
	template <typename T>
	inline
	void setBoundingBox(const drain::Box<T> & b){
		setLocation(b);
		setFrame(b);
	}



	//void setAlign(AlignSVG2::pos_t pos, AlignSVG2::axis_t axis,  AlignSVG2::value_t value);
	template <typename P, typename A,typename V>
	void setAlign(const P & pos, const A & axis,  const V &value){
		getAlign(pos, axis) = AlignSVG2::getValue<AlignSVG2::value_t>(value);
		updateAlignStr();
	}

	template <typename P, typename A>
	AlignSVG2::value_t & getAlign(const P & pos, const A & axis);

	template <typename P, typename A>
	const AlignSVG2::value_t & getAlign(const P & pos, const A & axis) const;

	void clearAlign();


protected:

	typedef std::vector<std::vector<AlignSVG2::value_t> > align_t;
	align_t alignments = align_t(2, std::vector<AlignSVG2::value_t>(2, drain::image::AlignSVG2::UNDEFINED));

	std::string alignStr;

	void updateAlignStr();

	//drain::Box<coord_t> box;
	BBoxSVG box;
	// consider:
	// bool x_PERCENTAGE = false;
	// bool y_PERCENTAGE = false;
	// svg:
	/*
	int x;
	int y;
	int width;
	int height;
	*/
	// std::string width; // can be "240px" or "90%" ?
	// std::string height;
	int radius;
	// std::string style;
	// std::string fill;
	// std::string opacity; // empty
	// std::string text_anchor;

};


template <typename P, typename A>
AlignSVG2::value_t & NodeSVG::getAlign(const P & pos, const A & axis){
	const AlignSVG2::pos_t p   = AlignSVG2::getValue<AlignSVG2::pos_t>(pos);
	const AlignSVG2::axis_t a  = AlignSVG2::getValue<AlignSVG2::axis_t>(axis);
	return alignments[p][a];
}

template <typename P, typename A>
const AlignSVG2::value_t & NodeSVG::getAlign(const P & pos, const A & axis) const {
	const AlignSVG2::pos_t p   = AlignSVG2::getValue<AlignSVG2::pos_t>(pos);
	const AlignSVG2::axis_t a  = AlignSVG2::getValue<AlignSVG2::axis_t>(axis);
	return alignments[p][a];
}

/*
template <typename P, typename A,typename V>
void NodeSVG::setAlign(const P & pos, const A & axis,  const V &value){
	alignments[p][a] = v;
}
*/

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

