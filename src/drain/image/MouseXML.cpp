/*

MIT License

Copyright (c) 2026 FMI Open Development / Markus Peura, first.last@fmi.fi

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
 * MouseXML.cpp
 *
 *  Created on: Jul 12, 2026
 *      Author: mpeura
 */


#include "MouseXML.h"

namespace drain {


DRAIN_ENUM_DICT(image::MouseXML::ElemClass) = {
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MOUSE),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MOUSE_LISTENER),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR_MOVE),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR_DOWN),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR_UP),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR_DRAG),
		// NEW
		// DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, DATA_ARRAY),
		// DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, SELECTOR),
};

DRAIN_ENUM_DICT(image::MouseXML::EventClass) = {
		DRAIN_ENUM_ENTRY(image::MouseXML::EventClass, ENTER),
		DRAIN_ENUM_ENTRY(image::MouseXML::EventClass, LEAVE),
		DRAIN_ENUM_ENTRY(image::MouseXML::EventClass, MOVE),
		DRAIN_ENUM_ENTRY(image::MouseXML::EventClass, DOWN),
		DRAIN_ENUM_ENTRY(image::MouseXML::EventClass, UP),
		DRAIN_ENUM_ENTRY(image::MouseXML::EventClass, CLICK),
};

DRAIN_ENUM_DICT(image::MouseXML::CoordinateProcessing) = {
		DRAIN_ENUM_ENTRY(image::MouseXML::CoordinateProcessing, UNDEFINED),
		DRAIN_ENUM_ENTRY(image::MouseXML::CoordinateProcessing, IMAGE_COORDS),
		DRAIN_ENUM_ENTRY(image::MouseXML::CoordinateProcessing, RELATIVE_COORDS),
		DRAIN_ENUM_ENTRY(image::MouseXML::CoordinateProcessing, GEOGRAPHIC_COORDS),
};


namespace image {

const std::string MouseXML::COORD_INIT_SCOPE("COORD_INIT_SCOPE");

std::string MouseXML::getEventFunctionName(MouseXML::EventClass eventKey){
	std::string s(drain::Enum<EventClass>::getKey(eventKey));
	drain::StringTools::lowerCase(s);
	return drain::StringBuilder<>("onmouse", s);
}

std::string MouseXML::getHandlerFunctionName(MouseXML::EventClass eventKey){
	std::string s(drain::Enum<EventClass>::getKey(eventKey));
	drain::StringTools::lowerCase(s);
	drain::StringTools::upperCase(s, 1);
	return drain::StringBuilder<>("handleMouse", s);
}

std::string MouseXML::getInitialiserFunctionName(MouseXML::EventClass eventKey){
	std::string s(drain::Enum<EventClass>::getKey(eventKey));
	drain::StringTools::lowerCase(s);
	drain::StringTools::upperCase(s, 1);
	return drain::StringBuilder<>("initMouse", s);
}

void MouseXML::addVisibilitySwitch(XML &dstElem,
		XML & listenerElem, const std::string &scope,
		const std::string &mouseEventOn, const std::string &mouseEventOff) {

	if (dstElem.getId().empty()){
		dstElem.setId("switchVisible_", dstElem.getTag(), XML::getNewIndex());
	}

	const std::string &id = dstElem.getId(); // ->setId();

	const std::string fct = scope.empty() ? "document.getElementById" : scope+".getElementById";

	// controlElem.setAttribute(mouseEventOn, drain::StringBuilder<>("setVisibile(", ,",true)"));
	// Consider setVisibility(value=[true|false], elemId, scope=null)
	listenerElem.setAttribute(mouseEventOn,
			drain::StringBuilder<>(fct, "('", id, "').style.visibility='visible'"));
	listenerElem.setAttribute(mouseEventOff,
			drain::StringBuilder<>(fct, "('", id, "').style.visibility='hidden'"));

}


}

}
