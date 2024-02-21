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
		STYLE=NodeXML<>::STYLE,
		SCRIPT=NodeXML<>::SCRIPT,
		HTML, HEAD, BODY, A, DIV, H1, H2, IMG, LI, OL, P, SPAN, TABLE, TITLE, TR, TH, TD, UL}; // check CTEXT, maybe implement in XML

	// typedef NodeHTML xml_node_t;

	// typedef TreeHTML tree_t;

};


/**
 *  \tparam T - index type; may be enum.
 */
class NodeHTML : public BaseHTML, public NodeXML<BaseHTML::tag_t> {
//class NodeHTML : public NodeXML<BaseHTML::tag_t> {


public:

	/// Default constructor
	NodeHTML(const elem_t & t = elem_t(0));

	/// Copy constructor
	NodeHTML(const NodeHTML & node);

	inline
	~NodeHTML(){};

	virtual
	void setType(const elem_t &t);


	/*
	inline
    NodeHTML & operator=(const std::initializer_list<std::pair<const char *,const drain::Variable> > &l){
		set(l);
		return *this;
	}
	*/

	template <class T>
	inline
	NodeHTML & operator=(const T & s){
		set(s); // XML
		return *this;
	}


	static
	const FileInfo fileInfo;

};


/// "Publish" the
typedef NodeHTML::xml_tree_t TreeHTML;
// typedef drain::UnorderedMultiTree<NodeHTML,false, NodeHTML::path_t> TreeHTML;




inline
std::ostream & operator<<(std::ostream &ostr, const NodeHTML & node){
	return drain::Sprinter::toStream(ostr, node.getAttributes());
}


inline
std::ostream & operator<<(std::ostream &ostr, const TreeHTML & tree){
	return drain::NodeXML<>::docToStream(ostr, tree);
}


template <>
inline
const char* TypeName<NodeHTML>::get(){
	return "HTML";
}

template <>
inline
const char* TypeName<BaseHTML::tag_t>::get(){
	return "HTML";
}



/** Example/ experimental template specif
template <>
TreeHTML & TreeHTML::addChild(const TreeHTML::key_t & key);
*/



template <>
template <>
inline
TreeHTML & TreeHTML::operator()(const BaseHTML::tag_t & type){
	this->data.setType(type);
	return *this;
}

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

