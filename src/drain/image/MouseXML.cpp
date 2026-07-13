/*
 * MouseXML.cpp
 *
 *  Created on: Jul 12, 2026
 *      Author: mpeura
 */


#include "MouseXML.h"

namespace drain {


DRAIN_ENUM_DICT(image::MouseXML::ElemClass) = {
		// DRAIN_ENUM_ENTRY(rack::Interactive::ElemClass, MOUSE),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MOUSE_TRACKER),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR_MOVE),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR_DOWN),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR_UP),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR_DRAG),
};

namespace image {

void MouseXML::addVisibilitySwitch(XML &dstElem,
		XML &controlElem, const std::string &scope,
		const std::string &mouseEventOn, const std::string &mouseEventOff) {

	if (dstElem.getId().empty()){
		dstElem.setId("switchVisible_", dstElem.getTag(), XML::getNewIndex());
	}

	const std::string &id = dstElem.getId(); // ->setId();

	const std::string fct = scope.empty() ? "document.getElementById" : scope+".getElementById";

	// controlElem.setAttribute(mouseEventOn, drain::StringBuilder<>("setVisibile(", ,",true)"));
	// Consider setVisibility(value=[true|false], elemId, scope=null)
	controlElem.setAttribute(mouseEventOn,
			drain::StringBuilder<>(fct, "('", id, "').style.visibility='visible'"));
	controlElem.setAttribute(mouseEventOff,
			drain::StringBuilder<>(fct, "('", id, "').style.visibility='hidden'"));

}


}

}
