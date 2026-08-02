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

#ifndef DRAIN_IMAGE_MOUSE_SVG
#define DRAIN_IMAGE_MOUSE_SVG

/*
#include "drain/Enum.h"
#include "drain/StringBuilder.h"
#include "drain/util/TreeXML.h"
#include "drain/util/UtilsXML.h"
*/

#include "TreeSVG.h"
#include "MouseXML.h"

namespace drain {

namespace image {

class MouseSVG {

public:

	/**
	 *  \param owner - optional label for code comments and debugging
	MouseSVG(TreeSVG & root, const std::string & mouseEvent, const std::string & owner = "") : root(root), mouseEvent(mouseEvent), owner(owner) {
		// connectedObjects.insert("?");
	}
	 */

	/**
	 *  \param owner - optional label for code comments and debugging
	 */
	MouseSVG(TreeSVG & root, drain::image::MouseXML::EventClass mouseEvent, const std::string & owner = "") : root(root), mouseEvent(mouseEvent), owner(owner) {
	}


	inline
	bool listenerIsSet(TreeSVG & elem) const {
		if (owner.empty()){
			drain::Logger(__FILE__, __FUNCTION__).warn("Owner undefined, potential duplicate listeners cannot be identified");
			return false;
		}
		else {
			TreeSVG & listenerScope = MouseXML::getMouseListenerScope(root, elem, mouseEvent);
			return listenerScope.hasChild(owner);
		}
	}

	template <class ...TT>
	inline
	TreeSVG & setListener(TreeSVG & elem, const TT & ...args) const {
		connect(args...);
		TreeSVG & listenerScope = MouseXML::getMouseListenerScope(root, elem, mouseEvent);
		if (!owner.empty()){
			listenerScope[owner]->setProgramComment(owner, ':', drain::sprinter(connectedObjects, drain::Sprinter::plainLayout));
		}
		else {
			listenerScope.addChild()->setProgramComment("undefined owner: ", drain::sprinter(connectedObjects, drain::Sprinter::plainLayout));
		}
		return listenerScope;
		//return MouseXML::ensureMouseListener(root, elem, mouseEvent);
	};

	inline
	TreeSVG & setListenerInit() const {
		return MouseXML::ensureMouseListenerInit(root, mouseEvent);
	};

	TreeSVG & getListenerInitScope() const;

	void connectClass(const drain::ClassXML & cls) const;

	void connectTag(const std::string & s) const;

	inline
	void connectTag(const svg::tag_t & type) const {
		connectTag(drain::Enum<svg::tag_t>::dict.getKey(type));
	};

	template <class ...TT>
	void connect(const drain::ClassXML & cls, const TT & ...args) const {
		connectClass(cls);
		connect(args...);
	};

	template <class ...TT>
	void connect(const std::string & arg, const TT & ...args) const {
		connectTag(arg);
		connect(args...);
	};

	template <class ...TT>
	void connect(const svg::tag_t & type, const TT & ...args) const {
		connectTag(type);
		connect(args...);
	};

	template <class ...TT>
	void connect(const char *arg, const TT & ...args) const {
		connectTag(arg);
		// getListenerInitScope()->setText("ctx['", arg, "'] = group.querySelector('", arg, "')");
		connect(args...);
	};


protected:

	TreeSVG & root;

	// const std::string mouseEvent;
	drain::image::MouseXML::EventClass mouseEvent;
	std::string owner;

	inline
	void connect() const {};

	mutable
	std::set<std::string> connectedObjects;

};



}

DRAIN_XML_ENUM_KEY(image::TreeSVG, image::MouseXML::ElemClass);
DRAIN_XML_ENUM_KEY(image::TreeSVG, image::MouseXML::EventClass);

}

 // 

#endif // DRAIN_IMAGE_MOUSEXML_H_ 
