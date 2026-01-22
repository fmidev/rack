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
DRAIN_TYPENAME_DEF(Html);
DRAIN_TYPENAME_DEF(Html::tag_t);


template <>
NodeHTML::xmldoc_attrib_map_t NodeHTML::xml_node_t::xmldoc_attribs = {
		{"version",  "1.0"},
		{"encoding", "UTF-8"},
		{"standalone", "no"},
		{"data-remark", "html"}
};

// Applied by XML::xmlAddChild()
template <>
const NodeXML<Html::tag_t>::xml_default_elem_map_t NodeXML<Html::tag_t>::xml_default_elems = {
		{Html::STYLE,  Html::CTEXT},
		{Html::SCRIPT, Html::CTEXT},
		{Html::BODY, Html::P}, // not sure about this
		{Html::UL, Html::LI},
		{Html::OL, Html::LI},
		{Html::PRE, Html::CTEXT},
		{Html::TABLE, Html::TR},
		{Html::TR, Html::TD},
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

// template <>
// const drain::EnumDict<Html::tag_t>::dict_t drain::EnumDict<Html::tag_t>::dict
DRAIN_ENUM_DICT(Html::tag_t) = { // drain::EnumDict<Basehtml::tag_t>::getDict(); // maybe dangerous.

		{"undefined", drain::Html::UNDEFINED},
		{"#comment", drain::Html::COMMENT},
		{"#ctext", drain::Html::CTEXT},
		{"script", drain::Html::SCRIPT},
		{"style", drain::Html::STYLE},
		{"style_select", drain::Html::STYLE_SELECT},
		{"html", drain::Html::HTML},
		{"head", drain::Html::HEAD},
		{"body", drain::Html::BODY},
		{"a", drain::Html::A},
		{"base", drain::Html::BASE},
		{"br", drain::Html::BR},
		{"caption", drain::Html::CAPTION},
		{"div", drain::Html::DIV},
		{"h1", drain::Html::H1},
		{"h2", drain::Html::H2},
		{"h3", drain::Html::H3},
		{"hr", drain::Html::HR},
		{"img", drain::Html::IMG},
		{"li", drain::Html::LI},
		{"link", drain::Html::LINK},
		{"meta", drain::Html::META},
		{"ol", drain::Html::OL},
		{"p", drain::Html::P},
		{"pre", drain::Html::PRE},
		{"span", drain::Html::SPAN},
		{"table", drain::Html::TABLE},
		{"title", drain::Html::TITLE},
		{"tr", drain::Html::TR},
		{"th", drain::Html::TH},
		{"td", drain::Html::TD},
		{"ul", drain::Html::UL},
};


template <> // for T (Tree class)
template <> // for K (path elem arg)
TreeHTML & TreeHTML::operator[](const Html::tag_t & type){
	return (*this)[EnumDict<Html::tag_t>::dict.getKey(type, false)];
}

template <> // for T (Tree class)
template <> // for K (path elem arg)
const TreeHTML & TreeHTML::operator[](const Html::tag_t & type) const {
	return (*this)[EnumDict<Html::tag_t>::dict.getKey(type, false)];
}

template <> // for T (Tree class)
template <> // for K (path elem arg)
TreeHTML & TreeHTML::operator[](const ClassXML & cls){
	return (*this)[cls.strPrefixed()];
}

/// Automatic conversion of element type (enum value) to a string.
template <> // for T (Tree class)
template <> // for K (path elem arg)
const TreeHTML & TreeHTML::operator[](const ClassXML & cls) const {
	return (*this)[cls.strPrefixed()];
}


template <> // for T (Tree class)
template <> // for K (path elem arg)
bool TreeHTML::hasChild(const ClassXML & cls) const {
	return hasChild(cls.strPrefixed());
}


// TODO: macro with lowerCaser
// TODO: mark some non-self-closing like <script/>

NodeHTML::NodeHTML(const tag_t & t) : xml_node_t() {
	setType(t);
};

NodeHTML::NodeHTML(const NodeHTML & node) : xml_node_t() { // NOTE: super class default constr -> does not call copyStruct
	XML::xmlAssignNode(*this, node);
	/*
	copyStruct(node, node, *this, ReferenceMap2::extLinkPolicy::LINK);
	setType(node.getType());
	ctext = node.ctext;
	*/
}

void NodeHTML::handleType(){ // const tag_t &t

	switch (getNativeType()) {
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
			Html::BR,
			Html::HR,
			Html::META
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

}


const FileInfo NodeHTML::fileInfo("html");



}  // drain::
