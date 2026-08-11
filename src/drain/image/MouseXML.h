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
#include "drain/util/EnumFlagger.h"

#include "drain/StringBuilder.h"
#include "drain/util/TreeXML.h"
#include "drain/util/UtilsXML.h"
#include "drain/util/JavaScriptXML.h"
#include "drain/js/coords.h"

namespace drain {

namespace image {

/**
 *  In running JavaScript on the rendered (SVG or HTML) document,
 *  the processing is divided in two sections:
 *
 *  - initialisation of the listener routine
 *  - running the listener routine
 *
 *  The idea in initialisation is to minimize processing time/load of the actual listener routine.
 *  For example, the final routine should not repeatedly call query functions whose return value is constant.
 *  Examples of such functions are getElementById() and queryElem().
 *
 *  Also, coordinate handling is optimised such that georeferenced coordinates are not computed unless
 *  they are explicitly requested by a routine.
 *
 */
class MouseXML {

public:

	/**
	 *  \see rack::RackSVG::ElemClass
	 *
	 *  Note: Event based entries will be deprecated as they are listed by EventClass
	 */
	enum ElemClass {
		MOUSE,	     // A group marked for interaction (mouse event listeners)
		MOUSE_LISTENER,   // Area inside which mouse events will be tracked.
		MONITOR,         // Display of interactive operations
		MONITOR_MOVE,    // Display something when mouse is moving, e.g. cursor coordinates.
		MONITOR_DOWN,    // Display something when mouse is pressed
		MONITOR_UP,      // Display something when mouse is released
		MONITOR_DRAG,    // Display something when mouse is dragged
		// new:
		// DATA_ARRAY,		 // Image used as data array only, not to be displayed
		//SELECTOR,        // Interactive element illustrating a selection by the user
	};

	// NEW
	enum EventClass {
		ENTER,
		LEAVE,
		MOVE,
		DOWN,
		UP,
		CLICK,
	};

	// bool cursorCoord = false;

	/// Show an element only if the mouse is on another specified element (listenerElem).
	/**
	 *
	 */
	static
	void addVisibilitySwitch(XML &dstElem, XML &listenerElem, const std::string & scope = "document",
			const std::string & mouseEventOn  = "onmouseenter",
			const std::string & mouseEventOff = "onmouseleave");



	enum CoordinateProcessing {
		UNDEFINED = 0,
		//BASIC = 1,
		IMAGE_COORDS      = 1,
		RELATIVE_COORDS   = 3,
		GEOGRAPHIC_COORDS = 7,
	};

	typedef drain::EnumFlagger<drain::MultiFlagger<CoordinateProcessing> > CoordFlagger;

protected:

	/// Modifies short mouse event name like "click" to "onmouseclick", or prefixed form "handleClick".
	/**
	 *   Examples:
	 *   - "CLICK" -> "onmouseclick"
	 *   - "CLICK","handle" -> "handleClick"
	 *
	 */
	static
	std::string getEventFunctionName(MouseXML::EventClass eventKey);

	static
	std::string getHandlerFunctionName(MouseXML::EventClass eventKey);

	static
	std::string getInitialiserFunctionName(MouseXML::EventClass eventKey);

	/// Return internal ("top-level") routine for mouse event. The routine calls subroutine (that has a sub scope).
	template <class N>
	static // , XML & elem , const EventClass & eventName
	DRAIN_XML_TREE(N) & getListenerScope(DRAIN_XML_TREE(N) & root, const std::string & handlerName);

	template <class N>
	void adjustListenerGeoRef(DRAIN_XML_TREE(N) & handlerScopeJS, CoordFlagger::ivalue_t  coords) const ;


	/// Return ("top-level") initialisation script scope for a mouse event. The routine calls subroutine (that has a sub scope).
	template <class N>
	static inline // const EventClass & mouseEventKey
	DRAIN_XML_TREE(N) & getListenerInitScope(DRAIN_XML_TREE(N) & root, const std::string & initFnctName);

	// template <class N>
	//inline
	//DRAIN_XML_TREE(N) & adjustListenerInitGeoRef(DRAIN_XML_TREE(N) & root, const EventClass & mouseEventKey) const;

protected:

	/// Create internal ("top-level") routine for mouse event, if it does not exist.
	template <class N>
	static
	DRAIN_XML_TREE(N) & ensureListenerUNUSED(DRAIN_XML_TREE(N) & root, XML & elem, const EventClass & eventCls);

	// template <class N>
	static inline
	void attachListener(XML & elem, const std::string & eventName, const std::string & handlerName){
		elem.setAttribute(eventName, handlerName, "(evt)");
	}

	template <class N>
	static
	DRAIN_XML_TREE(N) & ensureListenerHandler(DRAIN_XML_TREE(N) & root, const std::string & handlerName);


	/// Create internal ("top-level") initialisation script scope for mouse event, if it does not exist.
	template <class N>
	static
	DRAIN_XML_TREE(N) & ensureListenerInit(DRAIN_XML_TREE(N) & root, const std::string & initFnctName);
	//const EventClass & eventName, CoordinateProcessing proc = UNDEFINED); //const std::string & eventName);

	static
	const std::string COORD_INIT_SCOPE;
};


}

DRAIN_ENUM_DICT(image::MouseXML::ElemClass);
DRAIN_ENUM_DICT(image::MouseXML::EventClass);
DRAIN_ENUM_DICT(image::MouseXML::CoordinateProcessing);

DRAIN_ENUM_CLASSXML(image::MouseXML::ElemClass);


namespace image {

DRAIN_ENUM_OSTREAM(image::MouseXML::ElemClass);
DRAIN_ENUM_OSTREAM(image::MouseXML::EventClass);


template <class N>
void MouseXML::adjustListenerGeoRef(DRAIN_XML_TREE(N) & handlerScopeJS, CoordFlagger::ivalue_t  coords) const {

	const CoordFlagger flags(coords);

	DRAIN_XML_TREE(N) & coordScopeJS = handlerScopeJS(COORD_INIT_SCOPE);

	if (flags.isSet(IMAGE_COORDS)){
		coordScopeJS[flags.str()] -> setProgramComment("Coord request: ", flags.str());
		// TODO: detect if MOVE has already been defined, and drop (duplicated) coordinate processing from here.
		coordScopeJS["ctx.bbox"] = "ctx.bbox = evt.target.getBoundingClientRect();";
		coordScopeJS["COORDS"] -> setProgramComment("Image coordinates");
		coordScopeJS["ctx.x"] = "ctx.x = Math.floor(evt.clientX - ctx.bbox.left);";
		coordScopeJS["ctx.y"] = "ctx.y = Math.floor(evt.clientY - ctx.bbox.top);";
		if (flags.isSet(RELATIVE_COORDS)){
			coordScopeJS["RELATIVE_COORDS"] -> setProgramComment("Relative coordinates");
			coordScopeJS["ctx.rx"] = "ctx.rx = ctx.x / ctx.bbox.width;";
			coordScopeJS["ctx.ry"] = "ctx.ry = ctx.y / ctx.bbox.height;";
			if (flags.isSet(GEOGRAPHIC_COORDS)){
				coordScopeJS["GEOGRAPHIC_COORDS"] -> setProgramComment("Geographic coordinates");
				// geo_bbox.left + rx*geo_bbox.width
				coordScopeJS["ctx.gx"] = "ctx.gx = ctx.georef.bbox.left + ctx.rx*ctx.georef.bbox.width;";
				coordScopeJS["ctx.gy"] = "ctx.gy = ctx.georef.bbox.top  + ctx.ry*ctx.georef.bbox.height;";
			}
		}
	}

}


template <class N>
DRAIN_XML_TREE(N) & MouseXML::ensureListenerUNUSED(DRAIN_XML_TREE(N) & root, XML & elem, const EventClass & eventKey){

	drain::Logger mout(__FILE__, __FUNCTION__);

	std::string handlerName = getHandlerFunctionName(eventKey);
	ensureListenerHandler(root, handlerName);

	std::string eventName = getEventFunctionName(eventKey);
	attachListener(elem, eventName, handlerName);

}

/*
void MouseXML::attachListener(XML & elem, const std::string & eventName, const std::string & handlerName){
	elem.setAttribute(eventName, handlerName, "(evt)");
}
*/

template <class N>
DRAIN_XML_TREE(N) & MouseXML::ensureListenerHandler(DRAIN_XML_TREE(N) & root, const std::string & handlerName){

	// mout.attention("ensure ",eventName, '/',handlerName);

	// evt is a standard name?
	DRAIN_XML_TREE(N) & handlerFunctionScope = JavaScriptXML::ensureJavaScriptFunction(root, handlerName, "evt");

	if (!handlerFunctionScope.hasChild(__FUNCTION__)){
		handlerFunctionScope[__FUNCTION__]->setProgramComment("Std init by ", __FUNCTION__);
		handlerFunctionScope["const_ctx"] = "const ctx = evt.target.ctx;";
		DRAIN_XML_TREE(N) & coordInitSubScope = handlerFunctionScope[COORD_INIT_SCOPE];
		coordInitSubScope->setType(N::SCOPE_CURLY);
		coordInitSubScope->setText("if (true)");
		coordInitSubScope.addChild()->setProgramComment("Reserved for coordinates by ", __FUNCTION__);
	}
	return handlerFunctionScope;

	/*
	if (proc == UNDEFINED){
		if (eventKey == EventClass::MOVE){ // "move"){
			proc = GEOGRAPHIC_COORDS;
		}
		else { // if ( eventKey == "move")
			proc = BASIC;
		}
	}

	if (!scopeJS.hasChild(__FUNCTION__)){
		scopeJS.addChild(__FUNCTION__)->setProgramComment("Std init by ", __FUNCTION__);
		scopeJS++ = "const ctx = evt.target.ctx;";
		// scopeJS++ = "ctx.listerer = evt.target;";
		int level = static_cast<int>(proc);
		if (level >= COORDS){
			// TODO: detect if MOVE has already been defined, and drop (duplicated) coordinate processing from here.
			scopeJS++ = "ctx.bbox = evt.target.getBoundingClientRect();";
			scopeJS++ -> setProgramComment("Image coordinates");
			scopeJS++ = "ctx.x = Math.floor(evt.clientX - ctx.bbox.left);";
			scopeJS++ = "ctx.y = Math.floor(evt.clientY - ctx.bbox.top);";
			if (level >= RELATIVE_COORDS){
				scopeJS++ -> setProgramComment("Relative coordinates");
				scopeJS++ = "ctx.rx = ctx.x / ctx.bbox.width;";
				scopeJS++ = "ctx.ry = ctx.y / ctx.bbox.height;";
				if (level >= GEOGRAPHIC_COORDS){
					scopeJS++ -> setProgramComment("Geographic coordinates");
					// geo_bbox.left + rx*geo_bbox.width
					scopeJS++ = "ctx.gx = ctx.georef.bbox.left + ctx.rx*ctx.georef.bbox.width;";
					scopeJS++ = "ctx.gy = ctx.georef.bbox.top  + ctx.ry*ctx.georef.bbox.height;";
				}
			}
		}
		// scopeJS++ = "ctx.elem = null;";
		// scopeJS++ = "ctx.ctx = evt.target.ctx;"; // remove
		// scopeJS++ = "/ * end init * /" ;
	}
	*/

}

template <class N>
DRAIN_XML_TREE(N) & MouseXML::getListenerScope(DRAIN_XML_TREE(N) & root, const std::string & handlerName){ //const EventClass & eventKey){
	// , CoordinateProcessing proc){ // , const std::string & handlerName){

	drain::Logger mout(__FILE__, __FUNCTION__);

	DRAIN_XML_TREE(N) & scopeJS = ensureListenerHandler(root, handlerName); //  ensureListener(root, elem, eventKey);

	DRAIN_XML_TREE(N) & subScopeJS =  scopeJS["SUBSCOPE"];
	subScopeJS->setType(XML::SCOPE_CURLY);
	// subScopeJS->setId();
	subScopeJS->setText("with(ctx)");
	// subScopeJS["COMMENT"]->setProgramComment(" local scope ");

	// TODO: support direct scope?

	return subScopeJS;
	// return scopeJS;
}

template <class N>
DRAIN_XML_TREE(N) & MouseXML::ensureListenerInit(DRAIN_XML_TREE(N) & root,
		const std::string & initFnctName){ // , const std::string & eventKey){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//std::string initFnctName = getInitialiserFunctionName(eventKey);
	// mout.attention("init ", eventName, '/', handlerName);

	DRAIN_XML_TREE(N) & scopeJS =  JavaScriptXML::ensureJavaScriptFunction(root, initFnctName);
	scopeJS->setId(initFnctName);
	if (!scopeJS.hasChildren()){
		scopeJS.addChild()->setText("/* Added by ", __FUNCTION__, "*/");
		scopeJS.addChild()->setText("console.info('", __FUNCTION__, ':', initFnctName,"')");
		/*
		int level = static_cast<int>(proc);
		scopeJS++ -> setProgramComment("GeoCoords:");
		if (level >= GEOGRAPHIC_COORDS){
			scopeJS++ -> setProgramComment("todo: GeoCoords");
		}
		*/
	}

	// Ensure init
	DRAIN_XML_TREE(N) & onLoadScope = JavaScriptXML::getOnLoadScript(root);
	onLoadScope[initFnctName]->setText(initFnctName, "();");

	return scopeJS;
}

/**
 * \tparam N - Node type for the tree, DRAIN_XML_TREE(N)
 */
template <class N>
inline //const EventClass & mouseEventKey
DRAIN_XML_TREE(N) & MouseXML::getListenerInitScope(DRAIN_XML_TREE(N) & root, const std::string & initFnctName) {

	DRAIN_XML_TREE(N) & mouseInit = ensureListenerInit(root, initFnctName);
	//setListenerInit();

	std::string installerKey = initFnctName+"Installer"; // drain::Enum<MouseXML::EventClass>::getKey(mouseEventKey) + "INSTALLER";

	DRAIN_XML_TREE(N) & installer = mouseInit[installerKey];
	installer->setText("document.querySelectorAll('.", MouseXML::MOUSE, "').forEach(\n");

	DRAIN_XML_TREE(N) & scope = installer[N::tag_t::SCOPE_CURLY](N::tag_t::SCOPE_CURLY);
	if (scope.empty()){
		scope->setText("group =>");
		scope.addChild()->setProgramComment("MouseXML");
		scope.addChild()->setText("var listener = group.querySelector('.", MouseXML::MOUSE_LISTENER, "');");
	}
	installer["end-foreach"]->setText(")");

	DRAIN_XML_TREE(N) & initScope = scope[N::tag_t::SCOPE_CURLY];
	if (initScope.empty()){
		initScope->setType(N::tag_t::SCOPE_CURLY);
		initScope->setText("if (listener)");
		initScope["ensure_CTX"] = "if (!('ctx' in listener))  listener.ctx = {};";
	}

	// if GEOREF
	if (!initScope.hasChild("georef")){
		initScope["georef_comment"]->setProgramComment("Georeference added by ", __FUNCTION__, " (unconditional for now)");
		initScope["georef"]->setText("if (!('georef' in listener.ctx)){ listener.ctx.georef = {}; }");
		initScope["georef_bbox"]->setText("if (listener.hasAttribute('data-bbox')){ listener.ctx.georef.bbox = new BBox(listener.getAttribute('data-bbox'))};");
		// consider other element (IMAGE title)
		initScope["georef_epsg"]->setText("if (listener.hasAttribute('data-epsg')){ listener.ctx.georef.epsg = listener.getAttribute('data-epsg') };");
	}

	return initScope;
	//return scope[N::tag_t::SCOPE_CURLY];
}

/*
template <class N>
inline
DRAIN_XML_TREE(N) & MouseXML::adjustListenerInitGeoRef(DRAIN_XML_TREE(N) & root, const EventClass & mouseEventKey) const {

	DRAIN_XML_TREE(N) & initScope = getListenerInitScope(root, mouseEventKey);

	if (!initScope.hasChild("georef")){
		initScope["georef"]->setText("if (!('georef' in listener.ctx)){ listener.ctx.georef = {}; }");
		initScope["georef_bbox"]->setText("if (listener.hasAttribute('data-bbox')){ listener.ctx.georef.bbox = new BBox(listener.getAttribute('data-bbox'))};");
		// consider other element (IMAGE title)
		initScope["georef_epsg"]->setText("if (listener.hasAttribute('data-epsg')){ listener.ctx.georef.epsg = listener.getAttribute('data-epsg') };");
	}

}
*/

} // image::

// DRAIN_ENUM_DICT(image::MouseXML::ElemClass);
// DRAIN_ENUM_DICT(image::MouseXML::EventClass);

} // drain::

#endif // DRAIN_IMAGE_MOUSEXML_H_



 // 

