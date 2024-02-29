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

template <>
NodeHTML::xmldoc_attrib_map_t NodeHTML::xml_node_t::xmldoc_attribs = {
		{"version",  "1.0"},
		{"encoding", "UTF-8"},
		{"standalone", "no"},
		{"data-remark", "html"}
};


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
	{drain::BaseHTML::DIV,	    "div"},
	{drain::BaseHTML::H1,	    "h1"},
	{drain::BaseHTML::H2,	    "h2"},
	{drain::BaseHTML::H3,	    "h3"},
	{drain::BaseHTML::IMG,	    "img"},
	{drain::BaseHTML::LI,  		"li"},
	{drain::BaseHTML::LINK,  		"link"},
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

NodeHTML::NodeHTML(const elem_t & t) : xml_node_t() {
	this->type = BaseHTML::UNDEFINED;
	setType(t);
};

NodeHTML::NodeHTML(const NodeHTML & node) : xml_node_t() { // NOTE: super class default constr -> does not call copyStruct
	copyStruct(node, node, *this, xml_node_t::RESERVE); // This may corrupt (yet unconstructed) object?
	this->type = BaseHTML::UNDEFINED;
	setType(node.getType());
}

void NodeHTML::setType(const elem_t &t){

	link("id", id);

	//case NodeXML<>::CTEXT:
	//case NodeXML<>::UNDEFINED:
	//xml_node_t::setType(t);
	type = t;

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


const FileInfo NodeHTML::fileInfo("html");

// Experimental
template <>
TreeHTML & TreeHTML::addChild(const TreeHTML::key_t & key){
	drain::Logger mout(__FILE__,__FUNCTION__);
	mout.unimplemented("replace TreeHTML::addChild");
	return *this;
}

//int NodeXML::nextID = 0;


}  // drain::
