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

#include "XML.h"

namespace drain {

// array,map,pair,string,key, mapPair  [+AfillB:ared 2pxb-;+AopacityB:0.33-;+AscaleB:0.5-]
const SprinterLayout StyleXML::styleLineLayout(";", ";",  ":", "", ""); //, "\"\"", "''", ":");

// The record starts by \n, separates entries with \n, and finishes with a \n.
// Every key, like "table" is prefixed woth \t and suffixed with space ' '.
const SprinterLayout StyleXML::styleRecordLayout("\n\n\n", "", "","{\n}",  "\t ", "{ }"); // , "{>", "  ", ":"// ??? ("[,]", "{,}",  "(,)", "\"\"", "''", ":");

const SprinterLayout StyleXML::styleRecordLayoutActual("{\n}","(.)", " :;","","\t ");
// Consider making <STYLE-ITEM> elements, with name (composed of tag(s?) , and attribs like

/// Uses spaces as separators.
const SprinterLayout ClassListXML::layout = {" ", "", "", ""}; // , "\n", "=", ""};

int XML::nextID = 0;

const int XML::UNDEFINED;
const int XML::COMMENT;
const int XML::CTEXT;
const int XML::STYLE;
const int XML::STYLE_SELECT;
const int XML::SCRIPT;




}  // drain::
