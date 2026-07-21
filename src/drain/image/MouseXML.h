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
 * MouseXML.h
 *
 *  Created on: Jul 12, 2026
 *      Author: mpeura
 */

#ifndef DRAIN_IMAGE_MOUSEXML_H_
#define DRAIN_IMAGE_MOUSEXML_H_

#include "drain/Enum.h"
#include "drain/StringBuilder.h"
#include "drain/util/TreeXML.h"

namespace drain {

namespace image {

class MouseXML {

public:

	/**
	 *  \see rack::RackSVG::ElemClass
	 */
	enum ElemClass {
		//MOUSE,	     // A group marked for interaction (mouse event listeners)
		MOUSE_LISTENER,   // Area inside which mouse events will be tracked.
		MONITOR,         // Display of interactive operations
		MONITOR_MOVE,    // Display something when mouse is moving, e.g. cursor coordinates.
		MONITOR_DOWN,    // Display something when mouse is pressed
		MONITOR_UP,      // Display something when mouse is released
		MONITOR_DRAG,    // Display something when mouse is dragged
	};

	// bool cursorCoord = false;
	/**
	 *   Future option: other scope them document
	template <class T>
	static void addVisibilitySwitch(NodeXML<T> &dstElem,
			NodeXML<T> &controlElem, const std::string &scope = "document",
			const std::string &mouseEventOn = "onmouseenter",
			const std::string &mouseEventOff = "onmouseleave");
	 */
	static
	void addVisibilitySwitch(XML &dstElem, XML &controlElem, const std::string &scope = "document",
			const std::string &mouseEventOn = "onmouseenter",
			const std::string &mouseEventOff = "onmouseleave");

};

// Display something when mouse is dragged
/**
 *   Future option: other scope them document
 */
/*
template<class T>
inline void MouseXML::addVisibilitySwitch(NodeXML<T> &dstElem,
		drain::NodeXML<T> &controlElem, const std::string &scope,
		const std::string &mouseEventOn, const std::string &mouseEventOff) {

	// dst->setId("coordMove", dst->getId());
	if (dstElem.getId().empty()){
		dstElem.setId("mouseMonitor_", dstElem.getTag(), NodeXML<T>::getNewIndex());
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
*/

}

DRAIN_ENUM_DICT(image::MouseXML::ElemClass);

}

 // 

#endif // DRAIN_IMAGE_MOUSEXML_H_ 
