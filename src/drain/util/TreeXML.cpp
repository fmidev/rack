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
 * TreeXML.cpp
 *
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#include "TreeXML.h"

namespace drain {


/**
 *  \tparam  int - type of the values in the dictionary
 *  \tparam  XML - the owner of the dictionary
 */
DRAIN_ENUM_DICT2(int,XML) = {
		{"UNDEFINED", XML::UNDEFINED},
		{"#ctext", XML::CTEXT},   // CTEXT     - the tag should never appear
		{"#comment", XML::COMMENT},  // COMMMENT  - the tag should never appear
		{"style", XML::STYLE},  // // Consider making <STYLE-ITEM> elements, with name (composed of tag(s?) , and attribs like
		{"script", XML::SCRIPT},
};



template <>
NodeXML<int>::xmldoc_attrib_map_t NodeXML<int>::xmldoc_attribs = {
		{"version",  "1.0"},
		{"encoding", "UTF-8"},
		// "standalone", "no");
		// {"data-remark", "xml"} // non-standard, debugging?

};

template <>
const NodeXML<int>::xml_default_elem_map_t NodeXML<int>::xml_default_elems = {
		{XML::STYLE,  XML::CTEXT},
		{XML::SCRIPT, XML::CTEXT},
};





}  // drain::
