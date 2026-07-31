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
#include "drain/util/UtilsXML.h"

namespace drain {

namespace image {

class MouseXML {

public:

	/**
	 *  \see rack::RackSVG::ElemClass
	 */
	enum ElemClass {
		MOUSE,	     // A group marked for interaction (mouse event listeners)
		MOUSE_LISTENER,   // Area inside which mouse events will be tracked.
		MONITOR,         // Display of interactive operations
		MONITOR_MOVE,    // Display something when mouse is moving, e.g. cursor coordinates.
		MONITOR_DOWN,    // Display something when mouse is pressed
		MONITOR_UP,      // Display something when mouse is released
		MONITOR_DRAG,    // Display something when mouse is dragged
	};


	//DRAIN_XML_TREE(N) & root;

	template <class N>
	static
	DRAIN_XML_TREE(N) & getOnLoadScript(DRAIN_XML_TREE(N) & root);


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
			const std::string &mouseEventOn  = "onmouseenter",
			const std::string &mouseEventOff = "onmouseleave");


	/// Modifies short mouse event name like "click" to "onmouseclick", or prefixed form "handleClick".
	/**
	 *   Examples:
	 *   - "CLICK" -> "onmouseclick"
	 *   - "CLICK","handle" -> "handleClick"
	 *
	 */
	static
	void getEventFunctionName(std::string & eventName, const std::string prefix="");


	template <class N>
	static
	DRAIN_XML_TREE(N) & ensureMouseListener(DRAIN_XML_TREE(N) & root, XML & elem, const std::string & eventName); // , const std::string & handlerName = "");

	template <class N>
	static
	DRAIN_XML_TREE(N) & ensureMouseListenerInit(DRAIN_XML_TREE(N) & root, const std::string & eventName); // , const std::string & handlerName = "");


};

template <class N>
DRAIN_XML_TREE(N) & MouseXML::getOnLoadScript(DRAIN_XML_TREE(N) & root){

	drain::Logger mout(__FILE__, __FUNCTION__);

	static const std::string onload_fnc_name("drain_onload");

	mout.experimental<LOG_INFO>("Adding mouse interaction: SVG onload=", onload_fnc_name, "()");

	root->setAttribute("onload", onload_fnc_name, "()"); // perhaps repeatedly

	return drain::UtilsXML::ensureJavaScriptFunction(root, onload_fnc_name); // [svg::JAVASCRIPT_SCOPE](svg::JAVASCRIPT_SCOPE);
	// return drain::UtilsXML::ensureJavaScriptFunctionScope(ctx.getSVG(), onload_fnc_name);


}
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

template <class N>
DRAIN_XML_TREE(N) & MouseXML::ensureMouseListener(DRAIN_XML_TREE(N) & root, XML & elem, const std::string & eventKey){ // , const std::string & handlerName){

	std::string eventName = eventKey; //StringBuilder<>("onmouse", eventName);
	getEventFunctionName(eventName); // "onmouse + move"

	std::string handlerName = eventKey;
	getEventFunctionName(handlerName, "handle");

	elem.setAttribute(eventName, handlerName, "(evt)");

	// evt is a standard name?
	DRAIN_XML_TREE(N) & scopeJS = UtilsXML::ensureJavaScriptFunction(root, handlerName, "evt");

	if (!scopeJS.hasChildren()){
		scopeJS.addChild()->setText("/* Std init by ", __FUNCTION__, "*/");
		scopeJS.addChild() = "const ctx = evt.target;";
		scopeJS.addChild() = "const ctx_bbox = ctx.getBoundingClientRect();";
		scopeJS.addChild() = "const x = Math.floor(evt.clientX - ctx_bbox.left);";
		scopeJS.addChild() = "const y = Math.floor(evt.clientY - ctx_bbox.top);";
		scopeJS.addChild() = "const rx = x / ctx_bbox.width;";
		scopeJS.addChild() = "const ry = y / ctx_bbox.height;";
		scopeJS.addChild() = "var elem;";
		scopeJS.addChild() = "/* end init */" ;
	}

	return scopeJS;
}

template <class N>
DRAIN_XML_TREE(N) & MouseXML::ensureMouseListenerInit(DRAIN_XML_TREE(N) & root, const std::string & eventKey){

	std::string eventName = eventKey; //StringBuilder<>("onmouse", eventName);
	getEventFunctionName(eventName); // "onmouse + move"

	std::string handlerName = eventKey;
	getEventFunctionName(handlerName, "initMouse");

	DRAIN_XML_TREE(N) & scopeJS = UtilsXML::ensureJavaScriptFunction(root, handlerName);
	scopeJS->setId(handlerName);
	if (!scopeJS.hasChildren()){
		scopeJS.addChild()->setText("/* Added by ", __FUNCTION__, "*/");
	}

	// Ensure init
	DRAIN_XML_TREE(N) & onLoadScope = MouseXML::getOnLoadScript(root);
	onLoadScope[handlerName]->setText(handlerName, "();");

	return scopeJS;
}



}

DRAIN_ENUM_DICT(image::MouseXML::ElemClass);

}

 // 

#endif // DRAIN_IMAGE_MOUSEXML_H_ 
