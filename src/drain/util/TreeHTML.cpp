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
DRAIN_TYPENAME_DEF(BaseHTML::tag_t);


template <>
NodeHTML::xmldoc_attrib_map_t NodeHTML::xml_node_t::xmldoc_attribs = {
		{"version",  "1.0"},
		{"encoding", "UTF-8"},
		{"standalone", "no"},
		{"data-remark", "html"}
};

// Applied by XML::xmlAddChild()
template <>
const NodeXML<BaseHTML::tag_t>::xml_default_elem_map_t NodeXML<BaseHTML::tag_t>::xml_default_elems = {
		{BaseHTML::STYLE,  BaseHTML::CTEXT},
		{BaseHTML::SCRIPT, BaseHTML::CTEXT},
		{BaseHTML::BODY, BaseHTML::P}, // not sure about this
		{BaseHTML::UL, BaseHTML::LI},
		{BaseHTML::OL, BaseHTML::LI},
		{BaseHTML::TABLE, BaseHTML::TR},
		{BaseHTML::TR, BaseHTML::TD},
};

// #define DRAIN_ENUM_ENTRY2(nspace, key) {std::tolower( ( #key ) ), nspace::key}



template <>
const drain::EnumDict<BaseHTML::tag_t>::dict_t & drain::EnumDict<BaseHTML::tag_t>::getDict(){

	/*
	static drain::EnumDict<BaseHTML::tag_t>::dict_t dict;

	if (dict.empty()){

		dict.add("undefined", drain::BaseHTML::UNDEFINED);
	}
	*/

	return dict;
}


template <>
const drain::EnumDict<BaseHTML::tag_t>::dict_t drain::EnumDict<BaseHTML::tag_t>::dict = { // drain::EnumDict<BaseHTML::tag_t>::getDict(); // maybe dangerous.

		{"undefined", drain::BaseHTML::UNDEFINED},
		{"#comment", drain::BaseHTML::COMMENT},
		{"#ctext", drain::BaseHTML::CTEXT},
		{"script", drain::BaseHTML::SCRIPT},
		{"style", drain::BaseHTML::STYLE},
		{"style_select", drain::BaseHTML::STYLE_SELECT},
		{"html", drain::BaseHTML::HTML},
		{"head", drain::BaseHTML::HEAD},
		{"body", drain::BaseHTML::BODY},
		{"a", drain::BaseHTML::A},
		{"base", drain::BaseHTML::BASE},
		{"br", drain::BaseHTML::BR},
		{"caption", drain::BaseHTML::CAPTION},
		{"div", drain::BaseHTML::DIV},
		{"h1", drain::BaseHTML::H1},
		{"h2", drain::BaseHTML::H2},
		{"h3", drain::BaseHTML::H3},
		{"hr", drain::BaseHTML::HR},
		{"img", drain::BaseHTML::IMG},
		{"li", drain::BaseHTML::LI},
		{"link", drain::BaseHTML::LINK},
		{"meta", drain::BaseHTML::META},
		{"ol", drain::BaseHTML::OL},
		{"p", drain::BaseHTML::P},
		{"span", drain::BaseHTML::SPAN},
		{"table", drain::BaseHTML::TABLE},
		{"title", drain::BaseHTML::TITLE},
		{"tr", drain::BaseHTML::TR},
		{"th", drain::BaseHTML::TH},
		{"td", drain::BaseHTML::TD},
		{"ul", drain::BaseHTML::UL},
};



// TODO: macro with lowerCaser
// TODO: mark some non-self-closing like <script/>
/*
template <>
std::map<NodeHTML::tag_t,std::string> NodeHTML::xml_node_t::tags = {
	{drain::BaseHTML::UNDEFINED,	"undefined"},
	{drain::BaseHTML::COMMENT,	"#comment"},
	{drain::BaseHTML::CTEXT,	"#ctext"},
	{drain::BaseHTML::STYLE,	"style"},
	{drain::BaseHTML::SCRIPT,	"script"},
	{drain::BaseHTML::HTML,	    "html"},
	{drain::BaseHTML::HEAD,	    "head"},
	{drain::BaseHTML::BODY,	    "body"},
	{drain::BaseHTML::A,	    "a"},
	{drain::BaseHTML::BASE,	    "base"},
	{drain::BaseHTML::BR,	    "br"},
	{drain::BaseHTML::CAPTION,	"caption"},
	{drain::BaseHTML::DIV,	    "div"},
	{drain::BaseHTML::H1,	    "h1"},
	{drain::BaseHTML::H2,	    "h2"},
	{drain::BaseHTML::H3,	    "h3"},
	{drain::BaseHTML::HR,	    "hr"},
	{drain::BaseHTML::IMG,	    "img"},
	{drain::BaseHTML::LI,  		"li"},
	{drain::BaseHTML::LINK,  	"link"},
	{drain::BaseHTML::META,  	"meta"},
	{drain::BaseHTML::OL,  		"ol"},
	{drain::BaseHTML::P,	    "p"},
	{drain::BaseHTML::SPAN,	    "span"},
	{drain::BaseHTML::TABLE,	"table"},
	{drain::BaseHTML::TITLE,  	"title"},
	{drain::BaseHTML::TR,  		"tr"},
	{drain::BaseHTML::TH,  		"th"},
	{drain::BaseHTML::TD,		"td"},
	{drain::BaseHTML::UL,  		"ul"},

};
*/

NodeHTML::NodeHTML(const tag_t & t) : xml_node_t() {
	// this->type = BaseHTML::UNDEFINED;
	setType(t);
};

NodeHTML::NodeHTML(const NodeHTML & node) : xml_node_t() { // NOTE: super class default constr -> does not call copyStruct
	copyStruct(node, node, *this, ReferenceMap2::extLinkPolicy::LINK);
	setType(node.getType());
	/*
	copyStruct(node, node, *this, xml_node_t::RESERVE); // This may corrupt (yet unconstructed) object?
	this->type = BaseHTML::UNDEFINED;
	setType(node.getType());
	*/
}

void NodeHTML::handleType(const tag_t &t){

	// link("id", id);

	//case NodeXML<>::CTEXT:
	//case NodeXML<>::UNDEFINED:
	//xml_node_t::setType(t);
	// type = t;

	switch (t) {


	case HTML:
	case HEAD:
	case TITLE:
		return;
	case BODY:
		break;
	case A:
		link("href", url = "");
		break;
	case P:
		break;
	case IMG:
		link("src", url = "");
		//link("style", style, "");
		break;
	case TABLE:
		//link("style", style, "");
		break;
	case TR:
		//link("style", style, "");
		break;
	case TD:
		break;
	default:
		return;
	}

	// link("style", style = "");
}

template <>
bool NodeXML<BaseHTML::tag_t>::isSelfClosing() const {	/// Set of NOT self.closing tags.

	/// Inclusive solution...
	static
	const std::set<BaseHTML::tag_t> l = {BaseHTML::BR, BaseHTML::HR};
	return (l.find((BaseHTML::tag_t)this->getType()) != l.end()); // = found in the set

	/*
	static
	const std::set<BaseHTML::tag_t> l = {BaseHTML::SCRIPT, BaseHTML::TITLE};
	return (l.find(this->getType()) == l.end()); // not in the set
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

TreeHTML & TreeUtilsHTML::initHtml(drain::TreeHTML & html, const std::string & heading){

	html(drain::NodeHTML::HTML);

	drain::TreeHTML & head  = html["head"](drain::NodeHTML::HEAD);

	drain::TreeHTML & encoding = head["encoding"](drain::BaseHTML::META);
	encoding->set("charset", "utf-8");

	// drain::TreeHTML & style =
	head["style"](drain::BaseHTML::STYLE);

	drain::TreeHTML & title = head["title"](drain::BaseHTML::TITLE);
	title = heading;

	drain::TreeHTML & body = html["body"](drain::BaseHTML::BODY);

	if (!heading.empty()){
		drain::TreeHTML & h1 = body["title"](drain::BaseHTML::H1);
		h1 = heading;
	}

	return body;
}

drain::TreeHTML & TreeUtilsHTML::addChild(drain::TreeHTML & elem, drain::BaseHTML::tag_t tagType, const std::string & key){
	if (!key.empty()){
		return elem[key](tagType);
	}
	else {
		std::stringstream k("elem");
		k.width(3);
		k.fill('0');
		k << elem.getChildren().size();
		return elem[k.str()](tagType);
	}
}


/*
drain::TreeHTML & TreeUtilsHTML::fillTableRow(drain::TreeHTML & table, drain::TreeHTML & tr, const std::string value){

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
*/
//int NodeXML::nextID = 0;


}  // drain::
