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

const std::map<char,std::string> XML::encodingMap = {
		{'&', "&amp;"},
		{'<', "&lt;"},
		{'>', "&gt;"},
		{'"', "&quot;"}
};

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

const XML::intval_t XML::UNDEFINED;
const XML::intval_t XML::COMMENT;
const XML::intval_t XML::CTEXT;
const XML::intval_t XML::STYLE;
const XML::intval_t XML::STYLE_SELECT;
const XML::intval_t XML::SCRIPT;

// reset() clears also the type
void XML::clear(){
	map_t::clear();
	style.clear();
	ctext.clear();
}

bool XML::isSingular() const {
	// std::cerr << __FUNCTION__ << " always FALSE!\n";
	return false;
}

/// Tell if this element should always have an explicit closing tag even when empty, like <STYLE></STYLE>
bool XML::isExplicit() const {

	// Recommended implementation: with typeIs()
	return typeIs(SCRIPT, STYLE);

	/*
	static
	const std::set<intval_t> l = {SCRIPT, STYLE}; // todo, append, and/or generalize...
	return (l.find(type) != l.end()); // not in the set
	*/
}

void XML::setText(const std::string & s) {

	drain::Logger mout(__FILE__,__FUNCTION__);

	// mout.accept<LOG_WARNING>("setting text for elem type=", type, ", text: ", s); // *
	// TODO: warn if elem supports no ctext (like svg::CIRCLE or svg::RECT)

	switch (type){
	case XML::STYLE:
		mout.reject<LOG_WARNING>("setting style for STYLE elem: ", s); // *this);
		/*
		mout.warn("setting text to STYLE group - preferably use element-wise"); // remove later, keeping warning in output (write) phase
		//drain::StringTools::import(s, ctext);
		drain::SmartMapTools::setValues(style, s, ';', ':', " \t\n");
		mout.special(s, " -> ", style);
		*/
		break;
	case XML::STYLE_SELECT: // OK!
		mout.debug("parsing text to CSS definition");
		drain::SmartMapTools::setValues(getAttributes(), s, ';', ':', " \t\n");
		mout.special<LOG_DEBUG>(s, " -> ", getAttributes());
		break;
	case XML::UNDEFINED:
		type = CTEXT;
		// no break
	case XML::CTEXT:
	case XML::COMMENT:
	default:
		// TODO: check types, somehow...
		drain::StringTools::import(s, ctext);
	}

}

void XML::specificAttributesToStream(std::ostream & ostr) const {

	if (!classList.empty()){
		ostr << " class=\"";
		drain::Sprinter::toStream(ostr, classList, ClassListXML::layout);
		// std::copy(tree->classList.begin(), tree->classList.end(), std::ostream_iterator<std::string>(ostr, " "));
		ostr << '"'; //ostr << "\"";
	}

}


}  // drain::
