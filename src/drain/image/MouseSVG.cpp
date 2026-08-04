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


#include "drain/util/UtilsXML.h"

#include "MouseSVG.h"

namespace drain {

/*
DRAIN_ENUM_DICT(image::MouseXML::ElemClass) = {
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MOUSE),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MOUSE_LISTENER),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR_MOVE),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR_DOWN),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR_UP),
		DRAIN_ENUM_ENTRY(image::MouseXML::ElemClass, MONITOR_DRAG),
};
*/

namespace image {

TreeSVG & MouseSVG::getListenerInitScope() const {

	TreeSVG & mouseInit = setListenerInit();

	TreeSVG & installer = mouseInit[mouseEvent+"INSTALLER"]; // (svg::SCOPE_CURLY);
	installer->setText("document.querySelectorAll('.", MouseXML::MOUSE ,"').forEach(\n");

	TreeSVG & scope = installer[svg::SCOPE_CURLY](svg::SCOPE_CURLY);
	if (scope.empty()){
		scope->setText("group =>");
		scope.addChild()->setProgramComment("MouseSVG");
		// TreeSVG & forEach2b = scope.addChild()
		scope.addChild()->setText("var listener = group.querySelector('.", MouseXML::MOUSE_LISTENER ,"');");
		//scope.addChild() = "if (listener) with (listener)";
		scope.addChild() = "if (listener)";
		//scope.addChild() = "ctx = listener; ";
		scope[svg::SCOPE_CURLY](svg::SCOPE_CURLY);
		scope[svg::SCOPE_CURLY]["ensure_CTX"] = "if (!('ctx' in listener))  listener.ctx = {};";
		// installer.addChild() = "}";
		//TreeSVG & subScope = scope[svg::SCOPE_CURLY](svg::SCOPE_CURLY);
	}
	installer["end-foreach"]->setText(")"); // end-foreach, indented


	return scope[svg::SCOPE_CURLY];
}

TreeSVG & MouseSVG::getListenerInitSubScope() const {
	TreeSVG & scope = getListenerInitScope();
	TreeSVG & subScope = scope[svg::SCOPE_CURLY](svg::SCOPE_CURLY);
	if (subScope.empty()){
		subScope->setText("with(listener.ctx)");
	}
	return subScope;
}

void MouseSVG::addListenerInitGeoConf() const {
	TreeSVG & scope = getListenerInitScope();
	// BBOX requires
	drain::UtilsXML::getHeaderObject(root, svg::SCRIPT, "BBox") =
			"function BBox(bbox){"
			"if (typeof(bbox) === 'string'){ bbox = bbox.split(',') }"
			"  this.left   = parseFloat(bbox[0]);"
			"  this.top    = parseFloat(bbox[3]);"
			"  this.width  = parseFloat(bbox[2]) - this.left;"
			"  this.height = parseFloat(bbox[1]) - this.top;"
			"}";
	scope["bbox"]->setText("if (listener.hasAttribute('data-bbox')){ listener.bbox = new BBox(listener.getAttribute('data-bbox'))}");
	scope["epsg"]->setText("if (listener.hasAttribute('data-epsg')){ listener.epsg = listener.getAttribute('data-epsg') }");

}

void MouseSVG::connectClass(const drain::ClassXML & cls) const {
	Logger mout(__FILE__, __FUNCTION__);

	mout.debug("connecting CLASS ", cls , " = ", cls.strPrefixed());
	TreeSVG & scope = getListenerInitScope();
	scope[cls]->setText("listener.ctx['", cls, "'] = group.querySelector('", cls.strPrefixed(), "')");
	connectedObjects.insert(cls);
}

void MouseSVG::connectTag(const std::string & arg) const {
	// Logger mout(__FILE__, __FUNCTION__);

	TreeSVG & scope = getListenerInitScope();
	scope[arg]->setText("listener.ctx['", arg, "'] = group.querySelector('", arg, "')");
	connectedObjects.insert(arg);
}



} // image::

} // drain::
