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
 * TreeXML.h
 *
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */



#ifndef DRAIN_TREE_HTML
#define DRAIN_TREE_HTML

#include <ostream>

#include "FileInfo.h"
#include "ReferenceMap.h"
#include "TreeXML.h"
#include "UtilsXML.h"

namespace drain {


class NodeHTML;

/// The  HTML data structure.
/*
  	typedef NodeHTML::xml_tree_t TreeHTML;

    Note: the typedef defined in XML is not the same, it is
    drain::UnorderedMultiTree<NodeXML<html::tag_t>,false, NodeXML<>::path_t> CoreTreeHTML;
 	That is, the first template is NodeXML<html::tag_t> and not the complete NodeHTML
 */
typedef drain::UnorderedMultiTree<NodeHTML,false, NodeXML<>::path_t> TreeHTML;


// typedef drain::UnorderedMultiTree<NodeHTML,false, NodeXML<>::path_t> TreeHTML;

struct Html {

	enum tag_t {
		UNDEFINED=NodeXML<>::UNDEFINED,
		COMMENT=NodeXML<>::COMMENT,
		CTEXT=NodeXML<>::CTEXT,
		SCRIPT=NodeXML<>::SCRIPT,
		STYLE=NodeXML<>::STYLE,
		STYLE_SELECT=NodeXML<>::STYLE_SELECT,
		HTML,
		HEAD, BASE, LINK, META, TITLE,
		BODY, A, BR, CAPTION, DIV, H1, H2, H3, HR, IMG, LI, OL, P, PRE, SPAN, TABLE, TR, TH, TD, UL};


};



// template <>
// const drain::EnumDict<Html::tag_t>::dict_t drain::EnumDict<Html::tag_t>::dict;

DRAIN_ENUM_DICT(Html::tag_t);

// Note: this specialization actually applies to all XML paths of default type, that is NodeXML<int> .
template <>
template <>
inline
void drain::NodeXML<>::path_t::appendElem(const Html::tag_t & type){
	// void TreeHTML::path_t::appendElem(const Html::tag_t & type){
	appendElem(EnumDict<Html::tag_t>::dict.getKey(type, false));
}

/**
 *  \tparam T - index type; may be enum.
 */
//class NodeHTML : public HTML, public NodeXML<html::tag_t> {
class NodeHTML : public Html, public NodeXML<Html::tag_t> {
// class NodeHTML : public NodeXML<html::tag_t> {


public:

	// typedef int tag_t;

	/// Default constructor
	NodeHTML(const Html::tag_t & t = Html::tag_t(0));

	/// Copy constructor
	NodeHTML(const NodeHTML & node);

	inline
	~NodeHTML(){};

	/// Tell if this element should always be a single tag, not separate opening and closing tags.
	virtual
	bool isSingular() const override final;

	/// Tell if this element should always have separate opening and closing tags even when empty, like <STYLE></STYLE>
	virtual
	bool isExplicit() const override final;

	inline
	NodeHTML & operator=(const NodeHTML & node){
		return XML::xmlAssignNode(*this, node);
	}

	template <class T>
	inline
	NodeHTML & operator=(const T & s){
		set(s); // XML
		return *this;
	}

	/*
	inline
    NodeHTML & operator=(const std::initializer_list<std::pair<const char *,const drain::Variable> > &l){
		set(l);
		return *this;
	}
	*/

	/*
	static // virtual
	inline
	std::ostream & docTypeToStream(std::ostream &ostr){
		ostr << "<!DOCTYPE html>\n";
		return ostr;
	}
	*/

	static
	const FileInfo fileInfo;

	// virtual	void handleType(const Html::tag_t &t) override final;

protected:

	virtual
	void handleType() override final;

};




inline
std::ostream & operator<<(std::ostream &ostr, const NodeHTML & node){
	return node.nodeToStream(ostr);
}


// This is the way!
template <> // for T (Tree class)
inline
std::ostream & NodeXML<Html::tag_t>::docTypeToStream(std::ostream &ostr){
		ostr << "<!DOCTYPE html>\n";
		return ostr;
}





// Important TAG type initialisations for elements.
template <>
const NodeXML<Html::tag_t>::xml_default_elem_map_t NodeXML<Html::tag_t>::xml_default_elems;

/// Specialization of default child elements in HTML. For example, \c UL has \c LI elements, by default.
/**
 *   Relates to the above list of default elements.
 */
template <>
inline
void TreeHTML::initChild(TreeHTML & child) const {
	const typename Html::tag_t type = UtilsXML::retrieveDefaultType(this->data);
	if (type){
		child->setType(type);
	}
}


// Preferred template specification
template <>
template <>
inline
TreeHTML & TreeHTML::operator()(const Html::tag_t & type){
	return UtilsXML::setType(*this, type);
}

// Preferred template specification
template <>
inline
TreeHTML & TreeHTML::addChild(){ // const TreeHTML::key_t & key){
	return UtilsXML::addChild(*this);
}

// Preferred template specification
template <>
template <>
inline
bool TreeHTML::hasChild(const Html::tag_t & type) const { // const TreeHTML::key_t & key){
	return TreeHTML::hasChild(EnumDict<Html::tag_t>::dict.getKey(type, false));
}


template <> // for T (Tree class)
template <> // for K (path elem arg)
TreeHTML & TreeHTML::operator[](const Html::tag_t & type);

template <> // for T (Tree class)
template <> // for K (path elem arg)
const TreeHTML & TreeHTML::operator[](const Html::tag_t & type) const;

// Same for ClassXML (designed for children of STYLE element)

template <> // for T (Tree class)
template <> // for K (path elem arg)
TreeHTML & TreeHTML::operator[](const ClassXML & cls);

/// Automatic conversion of element type (enum value) to a string.
template <> // for T (Tree class)
template <> // for K (path elem arg)
const TreeHTML & TreeHTML::operator[](const ClassXML & cls) const ;


template <> // for T (Tree class)
template <> // for K (path elem arg)
bool TreeHTML::hasChild(const ClassXML & cls) const;


template <>
inline
//TreeHTML & TreeHTML::operator=(std::initializer_list<std::pair<const char *,const char *> > l){
TreeHTML & TreeHTML::operator=(std::initializer_list<std::pair<const char *,const Variable> > l){
	return UtilsXML::assign(*this, l);
}

/// Create separate CTEXT element.
template <>
template <>
inline
TreeHTML & TreeHTML::operator=(const std::string & arg){
	UtilsXML::assignString(*this, arg);
	return *this;
}


template <>
template <class T>
inline
TreeHTML & TreeHTML::operator=(const T & arg){
	return UtilsXML::assign(*this, arg);
}

/*
inline
std::ostream & operator<<(std::ostream &ostr, const NodeHTML & node){
	return drain::Sprinter::toStream(ostr, node.getAttributes());
}
*/


inline
std::ostream & operator<<(std::ostream &ostr, const TreeHTML & tree){
	return NodeHTML::docToStream(ostr, tree);
}



DRAIN_TYPENAME(NodeHTML);
DRAIN_TYPENAME(Html);
DRAIN_TYPENAME(Html::tag_t);


}  // drain::

#endif // DRAIN_TREE_HTML

