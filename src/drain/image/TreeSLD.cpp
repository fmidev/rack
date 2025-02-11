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
 * TreeSLD.cpp
 *
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#include "TreeSLD.h"

namespace drain {

DRAIN_TYPENAME_DEF(image::NodeSLD);
DRAIN_TYPENAME_DEF(image::SLD::tag_t);


template <>
const EnumDict<image::SLD::tag_t>::dict_t EnumDict<image::SLD::tag_t>::dict = {
		DRAIN_ENUM_ENTRY(drain::image::SLD, Abstract),
		DRAIN_ENUM_ENTRY(drain::image::SLD, ColorMap),
		DRAIN_ENUM_ENTRY(drain::image::SLD, ColorMapEntry),
		DRAIN_ENUM_ENTRY(drain::image::SLD, CssParameter),
		DRAIN_ENUM_ENTRY(drain::image::SLD, FeatureTypeStyle),
		DRAIN_ENUM_ENTRY(drain::image::SLD, Fill),
		DRAIN_ENUM_ENTRY(drain::image::SLD, Graphic),
		DRAIN_ENUM_ENTRY(drain::image::SLD, Mark),
		DRAIN_ENUM_ENTRY(drain::image::SLD, Name),
		DRAIN_ENUM_ENTRY(drain::image::SLD, NamedLayer),
		DRAIN_ENUM_ENTRY(drain::image::SLD, Opacity),
		DRAIN_ENUM_ENTRY(drain::image::SLD, PointSymbolizer),
		DRAIN_ENUM_ENTRY(drain::image::SLD, RasterSymbolizer),
		DRAIN_ENUM_ENTRY(drain::image::SLD, Rule),
		DRAIN_ENUM_ENTRY(drain::image::SLD, Size),
		DRAIN_ENUM_ENTRY(drain::image::SLD, StyledLayerDescriptor),
		DRAIN_ENUM_ENTRY(drain::image::SLD, Title),
		DRAIN_ENUM_ENTRY(drain::image::SLD, UserStyle),
		DRAIN_ENUM_ENTRY(drain::image::SLD, WellKnownName),
};



// Applied by XML::xmlAddChild()
template <>
const NodeXML<drain::image::SLD::tag_t>::xml_default_elem_map_t NodeXML<drain::image::SLD::tag_t>::xml_default_elems = {
};


namespace image {

template <>
NodeSLD::xmldoc_attrib_map_t NodeSLD::xml_node_t::xmldoc_attribs = {
		{"version",  "1.0"},
		{"encoding", "UTF-8"},
		{"standalone", "no"},
		// {"data-remark", "SLD"} debugging? inkview claims
};

const drain::FileInfo NodeSLD::fileInfo("sld");

std::string NodeSLD::xlink("http://www.w3.org/1999/xlink");
// std::string NodeSLD::sld("http://www.w3.org/2000/SLD");


NodeSLD::NodeSLD(SLD::tag_t t){
	if (t != SLD::UNDEFINED){
	}
	setType(t);
}

//NodeSLD::NodeSLD(const NodeSLD & node) : xml_node_t(), x(0), y(0), width(0), height(0), radius(0) {
NodeSLD::NodeSLD(const NodeSLD & node) : xml_node_t(){
	// RISKY references! copyStruct(node, node, *this, ReferenceMap2::extLinkPolicy::LINK); // <-- risky! may link Variable contents?
	XML::xmlAssignNode(*this, node);
	// Logger mout(__FILE__, __FUNCTION__);
	 //setType(node.getType());
	// mout.warn("node type: ", node.getType(), " -> type: ", type);
}

//template <>
// void NodeXML<image::SLD::tag_t>::handleType(const image::SLD::tag_t & t){
void NodeSLD::handleType(const SLD::tag_t & t) { // setType(const elem_t & t) {

	// drain::Logger mout(drain::TypeName<NodeSLD>::str().c_str(), __FUNCTION__);
	// mout.attention(__FUNCTION__, ": current type=", type, " arg=", t);
	/*
	YES: how to handle unchanged type properly? And reset bbox upon type change?
	if (type == t){
		return; // lazy
	}
	*/
	// type = t;

	switch (t) {
	case image::SLD::UNDEFINED:
		// case UNDEFINED:
		break;
	case image::SLD::COMMENT:
		// setComment();
		break;
	case image::SLD::CTEXT:
		//case XML::CTEXT:
		// setText();
		// tag = "";
		break;
	case SLD::ColorMapEntry:
		link("color", url); // rename
	default:
		return;
	}

	// link("pos", box.getLocation().tuple());
	// link("frm", box.getFrame().tuple());
	// DEPRECATING: see separate STYLE and CLASS?
	// link("style", style = "");
	// link("fill", fill = "");
	// link("opacity", opacity = ""); // string, so silent if empty


}

/// Needed for handling units in strings, like "50%" or "640px".
void NodeSLD::setAttribute(const std::string & key, const std::string &value){
	(*this)[key] = value;
}

/// Needed for handling units in strings, like "50%" or "640px".
void NodeSLD::setAttribute(const std::string & key, const char *value){
	(*this)[key] = value; // -> handleString()
}

}  // image::



template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSLD::hasChild(const image::SLD::tag_t & type) const {
	return hasChild(EnumDict<image::SLD::tag_t>::dict.getKey(type, true)); // no error on non-existent dict entry
}


/// Automatic conversion of elem classes to strings.
/**
 *
 */
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSLD & image::TreeSLD::operator[](const image::SLD::tag_t & type) const {
	return (*this)[EnumDict<image::SLD::tag_t>::dict.getKey(type, false)];
}


template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSLD & image::TreeSLD::operator[](const image::SLD::tag_t & type){
	return (*this)[EnumDict<image::SLD::tag_t>::dict.getKey(type, false)];
}


}  // drain::


