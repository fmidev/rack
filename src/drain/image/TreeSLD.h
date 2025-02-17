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
 * TreeSLD.h
 *
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#ifndef DRAIN_TREE_SLD
#define DRAIN_TREE_SLD

#include "drain/util/EnumFlags.h"
#include "drain/util/FileInfo.h"
#include "drain/util/Frame.h"
#include "drain/util/TreeXML.h"

namespace drain {

namespace image {

class NodeSLD;

typedef drain::UnorderedMultiTree<NodeSLD,false, NodeXML<>::path_t> TreeSLD;

struct SLD {

	// typedef float coord_t;

	enum tag_t {
		UNDEFINED=XML::UNDEFINED,
		COMMENT=XML::COMMENT,
		CTEXT=XML::CTEXT,
		SCRIPT=XML::SCRIPT,
		Abstract = 10,
		ColorMap,
		ColorMapEntry,
		CssParameter,
		FeatureTypeStyle,
		Fill,
		Graphic,
		Mark,
		Name,
		NamedLayer,
		Opacity,
		PointSymbolizer,
		RasterSymbolizer,
		Rule,
		Size,
		StyledLayerDescriptor,
		Title,
		UserStyle,
		WellKnownName,
	};
	// check CTEXT, maybe implement in XML

};

} // image::


//const drain::EnumDict<image::SLD::elem_t>::dict_t & getDict();


template <>
const EnumDict<image::SLD::tag_t>::dict_t EnumDict<image::SLD::tag_t>::dict;

DRAIN_ENUM_OSTREAM(image::SLD::tag_t)



namespace image {

/// A node structure for drain::UnorderedMultiTree<NodeSLD>, compatible with TreeXML
/**
  \example TreeSLD-example.cpp

  \see drain::TreeXML

 */
class NodeSLD: public NodeXML<SLD::tag_t> {
public:


	static
	const drain::FileInfo fileInfo;


	/// Default constructor. Create a node of given type.
	NodeSLD(SLD::tag_t t = SLD::UNDEFINED);

	/// Copy constructor.
	NodeSLD(const NodeSLD & node);

	inline virtual
	~NodeSLD(){};

	/// Copy data from a node. (Does not copy subtree.)
	inline
	NodeSLD & operator=(const NodeSLD & node){
		XML::xmlAssignNode(*this, node);
		return *this;
	}

	/// Copy data from a node. (Does not copy subtree.)
	inline
	NodeSLD & operator=(const std::initializer_list<Variable::init_pair_t > &l){
		set(l);
		return *this;
	}

	template <class T>
	inline
	NodeSLD & operator=(const T & arg){
		set(arg);
		return *this;
	}



	/// Set attribute value, handling units in string arguments, like in "50%" or "640px".
	virtual
	void setAttribute(const std::string & key, const std::string &value) override;

	/// Set attribute value, handling units in string arguments, like in "50%" or "640px".
	virtual
	void setAttribute(const std::string & key, const char *value) override;

	/// Tell if this element should always have an explicit closing tag even when empty, like <STYLE></STYLE>
	virtual inline
	bool isSingular() const override final {
		static const std::set<SLD::tag_t> singular = {SLD::ColorMapEntry};
		// Consider "complement" approach, non-singular
		return singular.find(static_cast<SLD::tag_t>(type)) != singular.end();
	}



	/// Write transform, in addition to XML::ClassList.
	/**
	 *  Writes special attributes:
	 *  - class="..." as defined XMK::specificAttributesToStream()
	 *  - transform="..." geometric transformations – only if defined.
	 *
	virtual
	void specificAttributesToStream(std::ostream & ostr) const override;
	 */

protected:


	///
	/**
	 *  Special: for TEXT and SPAN elements, links STYLE[font-size] to bbox.height?
	 */
	virtual
	void handleType(const SLD::tag_t & t) override final;

};


/*
template <typename P, typename A,typename V>
void NodeSLD::setAlign(const P & pos, const A & axis,  const V &value){
	alignments[p][a] = v;
}
*/

}  // image::

inline
std::ostream & operator<<(std::ostream &ostr, const image::NodeSLD & node){
	return node.nodeToStream(ostr);
}


}  // drain::


inline
std::ostream & operator<<(std::ostream &ostr, const drain::image::TreeSLD & tree){
	//return drain::NodeXML<const drain::image::NodeSLD>::docToStream(ostr, tree);
	return drain::image::NodeSLD::docToStream(ostr, tree);
	//return drain::image::TreeSLD::node_data_t::docToStream(ostr, tree);
}



namespace drain {

DRAIN_TYPENAME(image::NodeSLD);
DRAIN_TYPENAME(image::SLD::tag_t);


template <>
const NodeXML<image::SLD::tag_t>::xml_default_elem_map_t NodeXML<image::SLD::tag_t>::xml_default_elems;

/*
template <>
template <typename K, typename V>
image::TreeSLD & image::TreeSLD::operator=(std::initializer_list<std::pair<K,V> > args){
	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.attention("initlist pair<K,V>: ", args);
	data.set(args); // what about TreeSLD & arg
	return *this;
}
*/

/*
template <> // referring to Tree<NodeSLD>
image::TreeSLD & image::TreeSLD::operator=(std::initializer_list<std::pair<const char *,const char *> > l);
*/

template <> // referring to Tree<NodeSLD>
inline
image::TreeSLD & image::TreeSLD::operator=(std::initializer_list<std::pair<const char *,const Variable> > l){
//image::TreeSLD & image::TreeSLD::operator=(std::initializer_list<std::pair<const char *,const char *> > l){
	XML::xmlAssign(*this, l);
	return *this;
}


template <>
template <class T>
inline
image::TreeSLD & image::TreeSLD::operator=(const T & arg){
	XML::xmlAssign(*this, arg);
	return *this;
}

template <>
template <>
inline
image::TreeSLD & image::TreeSLD::operator=(const std::string & arg){
	XML::xmlAssignString(*this, arg);
	return *this;
}

/*
template <>
template <>
inline
image::TreeSLD & image::TreeSLD::operator=(const char * arg){
	XML::xmlAssignString(*this, arg);
	return *this;
}
*/


/*
template <>
template <>
image::TreeSLD & image::TreeSLD::operator=(const char *arg){
	return XML::xmlAssignString(*this, arg);
}
*/




// Important! Useful and widely used – but  fails with older C++ compilers ?
template <>
template <>
inline
image::TreeSLD & image::TreeSLD::operator()(const image::SLD::tag_t & type){
		return XML::xmlSetType(*this, type);
}

template <>
inline
image::TreeSLD & image::TreeSLD::addChild(const image::TreeSLD::key_t & key){
	return XML::xmlAddChild(*this, key);
}

/*
template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSLD::hasChild(const image::svg::tag_t & type) const;
*/

template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSLD & image::TreeSLD::operator[](const image::SLD::tag_t & type);

template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSLD & image::TreeSLD::operator[](const image::SLD::tag_t & type) const ;

} // drain::

#endif // DRAIN_TREE_SLD

