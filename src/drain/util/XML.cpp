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



/// Uses spaces as separators.
const SprinterLayout ClassListXML::layout = {" ", "", "", ""}; // , "\n", "=", ""};

int XML::nextID = 0;

const XML::intval_t XML::UNDEFINED;
const XML::intval_t XML::COMMENT;
const XML::intval_t XML::CTEXT;
const XML::intval_t XML::STYLE;
const XML::intval_t XML::STYLE_SELECT;
const XML::intval_t XML::SCRIPT;

typedef drain::EnumDict<XML::entity_t> xml_entity;

#define DRAIN_XML_ENTITY(entity) {entity_t::entity, drain::EnumDict<XML::entity_t>::dict.getKey(entity_t::entity)}

/**
 *
 *  https://www.w3schools.com/charsets/ref_utf_basic_latin.asp
 */
template <>
const drain::EnumDict<XML::entity_t>::dict_t drain::EnumDict<XML::entity_t>::dict = {
		{"&#38;",  XML::AMPERSAND},  // &amp;
		{"&#60;",  XML::LESS_THAN},  // &lt;
		{"&#61;",  XML::EQUAL_TO},  // &lt;
		{"&#62;",  XML::GREATER_THAN},  // &gt;
		{"&#160;", XML::NONBREAKABLE_SPACE}, // &nbsp;
		{"&#34;",  XML::QUOTE}, // &quot;
		{"&#123;",  XML::CURLY_LEFT},
		{"&#125;",  XML::CURLY_RIGHT},
		// ---
		{"-",  XML::TAB},
		{"\\",  XML::NEWLINE},
};

// Consider 1) getKeyConversionMap

const std::map<char,std::string> & XML::getKeyConversionMap(){

	static const std::map<char,std::string> m = {
			DRAIN_XML_ENTITY(LESS_THAN),
			DRAIN_XML_ENTITY(GREATER_THAN),
			DRAIN_XML_ENTITY(NONBREAKABLE_SPACE),
			// --
			DRAIN_XML_ENTITY(TAB),
			DRAIN_XML_ENTITY(NEWLINE),
	};

	return m;

}

const std::map<char,std::string> & XML::getAttributeConversionMap(){

	static const std::map<char,std::string> m = {
			DRAIN_XML_ENTITY(QUOTE),
			// DRAIN_XML_ENTITY(GREATER_THAN),
			// DRAIN_XML_ENTITY(NONBREAKABLE_SPACE),
	};

	return m;

}

const std::map<char,std::string> & XML::getCTextConversionMap(){

	static const std::map<char,std::string> m = {
			{entity_t::LESS_THAN, "&#60;"},
			{entity_t::GREATER_THAN, "&#62;"},
			{entity_t::NONBREAKABLE_SPACE, "&#160;"}, // &nbsp;",
			//entity_t::AMPERSAND, "&amp;",
	};
	return m;

}


/*
const std::map<char,std::string> XML::encodingMap = {
		{'&', "&amp;"},
		{'<', "&lt;"},
		{'>', "&gt;"},
		{'"', "&quot;"},
		{' ', "&nbsp;"},
};
*/

// reset() clears also the type
//void XML::clear(){
void XML::reset(){
	type = UNDEFINED;
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
		drain::StringTools::import(s, ctext); // ? obsolete code, orig with templates
	}

}

const std::map<char,std::string> & XML::getEntityMap(){

	static std::map<char,std::string> m;

	if (m.empty()){
		for (const auto & entry: drain::EnumDict<drain::XML::entity_t>::dict){
			switch (entry.second) {
				case XML::entity_t::NONBREAKABLE_SPACE:
				case XML::entity_t::EQUAL_TO:
					break;
				default:
					m[entry.second] = entry.first;
					break;
			}
			/*
			if (entry.second != XML::entity_t::NONBREAKABLE_SPACE){
				m[entry.second] = entry.first;
			}
			*/
		}
	}

	return m;
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
