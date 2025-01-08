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



#ifndef TREE_HTML
#define TREE_HTML

#include <ostream>

#include "FileInfo.h"
#include "ReferenceMap.h"
#include "TreeXML.h"

namespace drain {


class NodeHTML;

// typedef drain::UnorderedMultiTree<NodeHTML,false, NodeXML<>::path_t> TreeHTML;

struct BaseHTML {

	enum tag_t {
		UNDEFINED=NodeXML<>::UNDEFINED,
		COMMENT=NodeXML<>::COMMENT,
		CTEXT=NodeXML<>::CTEXT,
		SCRIPT=NodeXML<>::SCRIPT,
		STYLE=NodeXML<>::STYLE,
		STYLE_SELECT=NodeXML<>::STYLE_SELECT,
		HTML,
		HEAD, BASE, LINK, META, TITLE,
		BODY, A, BR, CAPTION, DIV, H1, H2, H3, HR, IMG, LI, OL, P, SPAN, TABLE, TR, TH, TD, UL};

	// check CTEXT, maybe implement in XML
	// typedef NodeHTML xml_node_t;
	// typedef TreeHTML tree_t;

};


/**
 *  \tparam T - index type; may be enum.
 */
//class NodeHTML : public BaseHTML, public NodeXML<BaseHTML::tag_t> {
class NodeHTML : public BaseHTML, public NodeXML<BaseHTML::tag_t> {
//class NodeHTML : public NodeXML<BaseHTML::tag_t> {


public:

	// typedef int tag_t;

	/// Default constructor
	NodeHTML(const tag_t & t = tag_t(0));

	/// Copy constructor
	NodeHTML(const NodeHTML & node);

	inline
	~NodeHTML(){};



	/*
	inline
    NodeHTML & operator=(const std::initializer_list<std::pair<const char *,const drain::Variable> > &l){
		set(l);
		return *this;
	}
	*/
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
	virtual
	bool isSelfClosing() const;
	*/

	static inline
	std::ostream & docToStream(std::ostream &ostr, const xml_tree_t &  tree){
		ostr << "<!DOCTYPE html>\n";
		toStream(ostr, tree);
		return ostr;
	}


	static
	const FileInfo fileInfo;

protected:

	virtual
	void handleType(const tag_t &t) override final;

};


/// The  HTML data structure.
/**
 *
 */
typedef NodeHTML::xml_tree_t TreeHTML;

template <>
inline
TreeHTML & TreeHTML::operator=(std::initializer_list<std::pair<const char *,const char *> > l){
	return XML::xmlAssign(*this, l);
}

template <>
template <class T>
inline
TreeHTML & TreeHTML::operator=(const T & arg){
	return XML::xmlAssign(*this, arg);
}


inline
std::ostream & operator<<(std::ostream &ostr, const NodeHTML & node){
	return drain::Sprinter::toStream(ostr, node.getAttributes());
}


/*
inline
std::ostream & operator<<(std::ostream &ostr, const TreeHTML & tree){
	//ostr << "<!DOCTYPE html>\n";
	//return drain::NodeXML<>::docToStream(ostr, tree);
	drain::NodeHTML::toStream(ostr, tree);
	return ostr;
}
*/



template <>
bool NodeXML<BaseHTML::tag_t>::isSelfClosing() const;


DRAIN_TYPENAME(NodeHTML);
DRAIN_TYPENAME(BaseHTML::tag_t);


/** Example/ experimental template specif
*/
template <>
inline
TreeHTML & TreeHTML::addChild(const TreeHTML::key_t & key){
	return XML::xmlAddChild(*this, key);
}

template <>
const NodeXML<BaseHTML::tag_t>::xml_default_elem_map_t NodeXML<BaseHTML::tag_t>::xml_default_elems;


template <>
template <>
inline
TreeHTML & TreeHTML::operator()(const BaseHTML::tag_t & type){
	// this->data.setType(type);
	// return *this;
	return XML::xmlSetType(*this, type);
}


// UTILS... consider to XML ?

class TreeUtilsHTML {

public:

	/// Initialize a HTML object with "head" (including "title", "style") and "body" elements.
	/**
	 *  If a title is given, it will be also assigned as H1 element.
	 */
	static
	drain::TreeHTML & initHtml(drain::TreeHTML & html, const std::string & key = "");



	static inline
	drain::TreeHTML & getFirstElem(drain::TreeHTML & elem, drain::BaseHTML::tag_t tagType){
		if (elem.hasChildren()){
			return elem.getChildren().begin()->second; // last
		}
		else {
			//
			return elem.addChild()(tagType); // addChild(elem, tagType);
		}
	}



	/// Add element of given type. The path key is generated automatically, unless given.
	/**
	 *
	 *
	 */
	static  // compare with TreeHTML::addChild( - is needed?
	drain::TreeHTML & addChild(drain::TreeHTML & elem, drain::BaseHTML::tag_t tagType, const std::string & key);

	template <class T>
	static inline
	drain::TreeHTML & appendElem(drain::TreeHTML & elem, drain::BaseHTML::tag_t tagType, const T & arg){
		drain::TreeHTML & child = elem.addChild()(tagType); //  addChild(elem,tagType);
		child = arg;
		return child;
	};


	template <class T, class ...TT>
	static inline
	drain::TreeHTML & appendElem(drain::TreeHTML & elem, drain::BaseHTML::tag_t tagType, const T & arg, const TT & ...args) {
		appendElem(elem, tagType, arg);
		return appendElem(elem, tagType, args...);
	}

	static inline
	drain::TreeHTML & createTable(drain::TreeHTML & body, const std::list<std::string> & columnTitles){ // "var", "ref", no whitespace.

		drain::TreeHTML & table =  body.addChild()(drain::NodeHTML::TABLE);   //       drain::TreeUtilsHTML::addChild(body, drain::NodeHTML::TABLE);

		drain::TreeHTML & tr    = table["header"](drain::NodeHTML::TR);

		for (const auto & title: columnTitles){
			drain::TreeHTML & th = tr[title](drain::NodeHTML::TH);
			th = title;
		}

		return table;
	}

	/// Creates a new table row (TD) using first row as a template.
	/**
	 *   \param T - HTML object, string or TAG type.
	 */
	template <class T>
	static
	// drain::TreeHTML & fillTableRow(drain::TreeHTML & table, drain::TreeHTML & tr, const std::string value = "");
	drain::TreeHTML & fillTableRow(drain::TreeHTML & table, drain::TreeHTML & tr, const T & value){

		for (const auto & entry: table.getChildren()){
			// Using keys of the first row, create a new row. Often, it is the title row (TH elements).
			for (const auto & e: entry.second.getChildren()){
				tr[e.first]->setType(drain::NodeHTML::TD);
				tr[e.first] = value;
			}
			// Return after investigating the first row:
			return tr;
		}

		// If table is empty, also tr is.
		return tr;

	}

	static
	drain::TreeHTML & addTableRow(drain::TreeHTML & table, const std::string value = ""){
		drain::TreeHTML & tr = table.addChild()(BaseHTML::TR); // addChild(table, BaseHTML::TR);
		return fillTableRow(table, tr, value);
	}


protected:

	// Dummy end... TODO: redesign logic, perhaps addChild();
	template <class T>
	static inline
	drain::TreeHTML & appendElem(drain::TreeHTML & elem, drain::BaseHTML::tag_t tagType){
		if (elem.hasChildren()){
			return elem.getChildren().rbegin()->second; // last
		}
		else {
			//
			return elem;
		}
	}

};

/*
template <>
template <>
inline
TreeHTML & TreeHTML::operator()(const std::string & text){
	this->data.ctext = text;
	return *this;
}
*/




}  // drain::

#endif /* TREEXML_H_ */

