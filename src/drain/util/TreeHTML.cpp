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
 * TreeHTML.cpp
 *
 *  Created on:2024
 *      Author: mpeura
 */

#include "TreeHTML.h"


namespace drain {

DRAIN_TYPENAME_DEF(NodeHTML);
DRAIN_TYPENAME_DEF(html);
DRAIN_TYPENAME_DEF(html::tag_t);


template <>
NodeHTML::xmldoc_attrib_map_t NodeHTML::xml_node_t::xmldoc_attribs = {
		{"version",  "1.0"},
		{"encoding", "UTF-8"},
		{"standalone", "no"},
		{"data-remark", "html"}
};

// Applied by XML::xmlAddChild()
template <>
const NodeXML<html::tag_t>::xml_default_elem_map_t NodeXML<html::tag_t>::xml_default_elems = {
		{html::STYLE,  html::CTEXT},
		{html::SCRIPT, html::CTEXT},
		{html::BODY, html::P}, // not sure about this
		{html::UL, html::LI},
		{html::OL, html::LI},
		{html::PRE, html::CTEXT},
		{html::TABLE, html::TR},
		{html::TR, html::TD},
};

// #define DRAIN_ENUM_ENTRY2(nspace, key) {std::tolower( ( #key ) ), nspace::key}



/*
template <>
const drain::EnumDict<Basehtml::tag_t>::dict_t & drain::EnumDict<Basehtml::tag_t>::getDict(){
	//// static drain::EnumDict<Basehtml::tag_t>::dict_t dict;

	//if (dict.empty()){ dict.add("undefined", drain::Basehtml::UNDEFINED);}
	return dict;
}
*/


template <>
const drain::EnumDict<html::tag_t>::dict_t drain::EnumDict<html::tag_t>::dict = { // drain::EnumDict<Basehtml::tag_t>::getDict(); // maybe dangerous.

		{"undefined", drain::html::UNDEFINED},
		{"#comment", drain::html::COMMENT},
		{"#ctext", drain::html::CTEXT},
		{"script", drain::html::SCRIPT},
		{"style", drain::html::STYLE},
		{"style_select", drain::html::STYLE_SELECT},
		{"html", drain::html::HTML},
		{"head", drain::html::HEAD},
		{"body", drain::html::BODY},
		{"a", drain::html::A},
		{"base", drain::html::BASE},
		{"br", drain::html::BR},
		{"caption", drain::html::CAPTION},
		{"div", drain::html::DIV},
		{"h1", drain::html::H1},
		{"h2", drain::html::H2},
		{"h3", drain::html::H3},
		{"hr", drain::html::HR},
		{"img", drain::html::IMG},
		{"li", drain::html::LI},
		{"link", drain::html::LINK},
		{"meta", drain::html::META},
		{"ol", drain::html::OL},
		{"p", drain::html::P},
		{"pre", drain::html::PRE},
		{"span", drain::html::SPAN},
		{"table", drain::html::TABLE},
		{"title", drain::html::TITLE},
		{"tr", drain::html::TR},
		{"th", drain::html::TH},
		{"td", drain::html::TD},
		{"ul", drain::html::UL},
};

template <> // for T (Tree class)
template <> // for K (path elem arg)
TreeHTML & TreeHTML::operator[](const html::tag_t & type){
	return (*this)[EnumDict<html::tag_t>::dict.getKey(type, false)];
}

template <> // for T (Tree class)
template <> // for K (path elem arg)
const TreeHTML & TreeHTML::operator[](const html::tag_t & type) const {
	return (*this)[EnumDict<html::tag_t>::dict.getKey(type, false)];
}


// TODO: macro with lowerCaser
// TODO: mark some non-self-closing like <script/>

NodeHTML::NodeHTML(const tag_t & t) : xml_node_t() {
	// this->type = BaseHTML::UNDEFINED;
	setType(t);
};

NodeHTML::NodeHTML(const NodeHTML & node) : xml_node_t() { // NOTE: super class default constr -> does not call copyStruct
	copyStruct(node, node, *this, ReferenceMap2::extLinkPolicy::LINK);
	setType(node.getType());
	ctext = node.ctext;
}

void NodeHTML::handleType(const tag_t &t){

	switch (t) {
	case A:
		link("href", url = "");
		break;
	case IMG:
		link("src", url = "");
		break;
	default:
		return;
	}

}

/// These elements MUST appear empty, ie. without child elements.
bool NodeHTML::isSingular() const {

	return typeIs(
			html::BR,
			html::HR,
			html::META
	);

	/*
	// std::cout << __FILE__ << '/' << __FUNCTION__ << this->getTag() << " singular=" << (l.find((Basehtml::tag_t)this->getType()) == l.end()) << '\n';

	/// "Inclusive" solution...
	static
	const std::set<html::tag_t> l = {html::BR, html::HR, html::META};

	// If is (BR, HR, ...), return true, forcing "self-closing" element.
	return (l.find((html::tag_t)this->getType()) != l.end()); // = found in the set
	*/
}

/// These elements MUST appear open, even when having no child elements.
bool NodeHTML::isExplicit() const {

	// Adjust later
	return !isSingular();
	/*
	return typeIs(
			html::A,
			html::IMG
	);
	*/

	// std::cout << __FILE__ << '/' << __FUNCTION__ << this->getTag() << " explicit=" << (l.find(type) == l.end()) << '\n';

	/// These tags can appear "empty", without child elements.
	/*
	static
	const std::set<intval_t> l = {html::A, html::IMG};

	// Not in the set, meaning: not allowed to be empty.
	return (l.find(type) == l.end());
	*/
}


const FileInfo NodeHTML::fileInfo("html");

// Experimental
/*
template <>
TreeHTML & TreeHTML::addChild(const TreeHTML::key_t & key){
	if (key.empty()){
		std::stringstream k("elem");
		k.width(3);
		k.fill('0');
		k << getChildren().size();
		return (*this)[k.str()];
	}
	else {
		return (*this)[key];
	}
}
*/
/*
	drain::Logger mout(__FILE__,__FUNCTION__);
	mout.unimplemented("replace TreeHTML::addChild");
	return *this;
 */


}  // drain::
