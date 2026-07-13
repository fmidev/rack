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

	// dst->setId("coordMove", dst->getId());
	if (dstElem.getId().empty()){
		dstElem.setId("mouseMonitor_", dstElem.getTag(), XML::getNewIndex());
	}

	const std::string &id = dstElem.getId(); // ->setId();

	if (scope.empty()) {
		controlElem.setAttribute(mouseEventOn,
				drain::StringBuilder<>("getElementById('", id, "').style.visibility='visible'"));
		controlElem.setAttribute(mouseEventOff,
				drain::StringBuilder<>("getElementById('", id, "').style.visibility='hidden'"));
	}
	else {
		controlElem.setAttribute(mouseEventOn,
				drain::StringBuilder<>(scope, ".getElementById('", id, "').style.visibility='visible'"));
		controlElem.setAttribute(mouseEventOff,
				drain::StringBuilder<>(scope, ".getElementById('", id, "').style.visibility='hidden'"));
	}
}


}

}
