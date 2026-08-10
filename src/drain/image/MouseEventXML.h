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
 * MouseEventXML.h
 *
 *  Created on: Aug 6, 2026
 *      Author: mpeura
 */

#ifndef DRAIN_IMAGE_MOUSEXML2_H_
#define DRAIN_IMAGE_MOUSEXML2_H_

#include <set>
#include <string>

#include "MouseXML.h"
#include "TreeSVG.h"
#include "drain/js/coords.h"

namespace drain {

namespace image {

/**
 * \tparam N - Node type for the tree, DRAIN_XML_TREE(N)
 */
template <class N>
class MouseEventXML : public MouseXML {

public:

	typedef DRAIN_XML_TREE(N) tree_t;
	// typedef N node_t;

	MouseEventXML(tree_t & root, drain::image::MouseXML::EventClass mouseEvent, const std::string & owner) :
		root(root),
		eventName(getEventFunctionName(mouseEvent)),
		handlerName(getHandlerFunctionName(mouseEvent)),
		initName(getInitialiserFunctionName(mouseEvent)),
		scopeJS(ensureListenerHandler(root, handlerName)),
		owner(owner) {
		//: root(root), mouseEvent(mouseEvent), scopeJS(ensureListener(root, mouseEvent)), owner(owner) {
		scopeJS[owner]->setProgramComment("Set by: ", owner);
	}

	/*
	inline
	bool listenerIsSet(tree_t & elem) const {
		if (owner.empty()){
			drain::Logger(__FILE__, __FUNCTION__).warn("Owner undefined, potential duplicate listeners cannot be identified");
			return false;
		}
		else {
			tree_t & listenerScope = MouseXML::getListenerScope(root, handlerName); //  elem, mouseEvent);
			return ((elem->getAttribute(eventName, "") == handlerName) && listenerScope.hasChild(owner));
		}
	}
	*/

	inline
	bool routineIsSet(tree_t & elem) const {
		if (owner.empty()){
			drain::Logger(__FILE__, __FUNCTION__).warn("Owner undefined, potential duplicate listeners cannot be identified");
			return false;
		}
		else {
			tree_t & listenerScope = MouseXML::getListenerScope(root, handlerName); //  elem, mouseEvent);
			return listenerScope.hasChild(owner);
		}
	}

	// template <class ...TT>
	inline
	void setListenerNEW(tree_t & elem) const { //, const TT & ...args) const {
		attachListener(elem, eventName, handlerName);
	}

	inline // tree_t & elem
	tree_t & getListenerRoutine() const { //, const TT & ...args) const {
		// connectElement(args...);
		// tree_t & listenerScope = MouseXML::getListenerScope(root, elem, mouseEvent);
		tree_t & listenerScope = MouseXML::getListenerScope(root, handlerName);
		if (!owner.empty()){
			// Marker!
			listenerScope[owner]->setProgramComment(owner, ':', drain::sprinter(connectedObjects, drain::Sprinter::plainLayout));
			// attachListener(elem, eventName, handlerName);
		}
		else {
			listenerScope.addChild()->setProgramComment("undefined owner: ", drain::sprinter(connectedObjects, drain::Sprinter::plainLayout));
		}
		return listenerScope;
	}


	/// Scope inside ctx
	tree_t & getListenerInitSubScope() const;

	template <class V>
	inline
	void defineVariable(const std::string & name, const V & value) const;
	// void defineConstant(const std::string & name, const V & value) const;

	inline
	void useCoordinates(CoordinateProcessing level) const {
		adjustListenerGeoRef(scopeJS, level);
	}

	void connectClass(const drain::ClassXML & cls) const;

	void connectTag(const std::string & s) const;

	inline
	void connectTag(const typename N::tag_t & type) const {
		connectTag(drain::Enum<typename N::tag_t>::dict.getKey(type));
	}

	inline
	void connectElement(const drain::ClassXML & cls){
		connectClass(cls);
	}

	inline
	void connectElement(const typename N::tag_t & type) const {
		connectTag(type);
	}


	inline
	void connectElement(const std::string & arg) const {
		connectTag(arg);
	}

	inline
	void connectElement(const char *arg) const {
		connectTag(arg);
	}

	/*
	template <class ...TT>
	void connectElement(const drain::ClassXML & cls, const TT & ...args) const {
		connectClass(cls);
		connectElement(args...);
	}

	template <class ...TT>
	void connectElement(const std::string & arg, const TT & ...args) const {
		connectTag(arg);
		connectElement(args...);
	}

	template <class ...TT>
	void connectElement(const typename N::tag_t & type, const TT & ...args) const {
		connectTag(type);
		connectElement(args...);
	}

	template <class ...TT>
	void connectElement(const char *arg, const TT & ...args) const {
		connectTag(arg);
		connectElement(args...);
	}
	*/


protected:

	tree_t & root;

	//drain::image::MouseXML::EventClass mouseEvent;
	const std::string eventName;   // standard lower case, like "onmousemove"
	const std::string handlerName; // camel case, like "handleMouseMove"
	const std::string initName;    // camel case, like "initMouseMove"

	tree_t & scopeJS;

	std::string owner;

	// Terminal function for the variadic template
	// inline
	// void connectElement() const {}

	mutable
	std::set<std::string> connectedObjects;

};



/**
 * \tparam N - Node type for the tree, DRAIN_XML_TREE(N)
 */
template <class N>
inline
DRAIN_XML_TREE(N) & MouseEventXML<N>::getListenerInitSubScope() const {
	DRAIN_XML_TREE(N) & scope = getListenerInitScope(root, initName);
	DRAIN_XML_TREE(N) & subScope = scope[N::tag_t::SCOPE_CURLY](N::tag_t::SCOPE_CURLY);
	if (subScope.empty()){
		subScope->setText("with(listener.ctx)");
		subScope.addChild()->setProgramComment("Scope exposed by ", __FUNCTION__);
	}
	return subScope;
}

/*
template <class N>
void MouseEventXML<N>::useCoordinates(CoordinateProcessing level) const {
	adjustListenerGeoRef(scopeJS, level);
}
*/

/**
 * \tparam N - Node type for the tree, DRAIN_XML_TREE(N)
 */
template <class N>
inline
void MouseEventXML<N>::connectClass(const drain::ClassXML & cls) const {
	Logger mout(__FILE__, __FUNCTION__);

	mout.debug("connecting CLASS ", cls, " = ", cls.strPrefixed());
	DRAIN_XML_TREE(N) & scope = getListenerInitScope(root, initName);
	scope[cls]->setText("listener.ctx['", cls, "'] = group.querySelector('", cls.strPrefixed(), "')");
	connectedObjects.insert(cls);
}

/**
 * \tparam N - Node type for the tree, DRAIN_XML_TREE(N)
 */
template <class N>
template <class V>
inline
void MouseEventXML<N>::defineVariable(const std::string & name, const V & value) const {
	Logger mout(__FILE__, __FUNCTION__);

	// mout.debug("connecting CLASS ", cls, " = ", cls.strPrefixed());
	DRAIN_XML_TREE(N) & scope = getListenerInitScope(root, initName);
	scope[name]->setText("listener.ctx['", name, "'] = ", sprinter(value, Sprinter::jsLayout), ';');
	//connectedObjects.insert(cls);
}


/**
 * \tparam N - Node type for the tree, DRAIN_XML_TREE(N)
 */
template <class N>
inline
void MouseEventXML<N>::connectTag(const std::string & arg) const {
	DRAIN_XML_TREE(N) & scope = getListenerInitScope(root, initName);
	scope[arg]->setText("listener.ctx['", arg, "'] = group.querySelector('", arg, "')");
	connectedObjects.insert(arg);
}


// MOVE to MouseXML(1)
/**
 * \tparam N - Node type for the tree, DRAIN_XML_TREE(N)
 */
/*
template <class N>
inline
void MouseEventXML<N>::addListenerInitGeoConf() const {
	JavaScriptXML::ensureScript(root, "coords", javascript::coords);

	DRAIN_XML_TREE(N) & scope = getListenerInitScope(root, mouseEvent);
	scope["bbox"]->setProgramComment("bbox+epsg init removed");
	// scope["bbox"]->setText("if (listener.hasAttribute('data-bbox')){ listener.ctx.geo_bbox = new BBox(listener.getAttribute('data-bbox'))}");
	// consider other element (IMAGE title)
	// scope["epsg"]->setText("if (listener.hasAttribute('data-epsg')){ listener.ctx.geo_epsg = listener.getAttribute('data-epsg') }");
}
*/

} // namespace image

} // namespace drain

#endif // DRAIN_IMAGE_MOUSEXML2_H_
