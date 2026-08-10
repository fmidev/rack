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
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
 */
/*
 * UtilsXML.h
 *
 *      Author: mpeura
 */

#ifndef DRAIN_JAVASCRIPT_XML
#define DRAIN_JAVASCRIPT_XML

// #include "XML.h"

#include "TreeXML.h"
#include "UtilsXML.h"

namespace drain {


/// Functions for manipulating XML structures.
class JavaScriptXML {

public:


	/*
	template <typename N>
	static
	DRAIN_XML_TREE(N) & getHeaderObject(DRAIN_XML_TREE(N) & root, typename N::xml_tag_t tag,
			const typename DRAIN_XML_TREE(N)::path_elem_t & key = typename DRAIN_XML_TREE(N)::path_elem_t()){

		const typename DRAIN_XML_TREE(N)::path_elem_t & finalKey = !key.empty() ? key : Enum<typename N::xml_tag_t>::getKey(tag);

		if (!root.hasChild(finalKey)){
			DRAIN_XML_TREE(N) & child = root.prependChild(finalKey); // consider path type! getDefaultObject
			child->setType(tag);
			return child;
		}
		else {
			return root[finalKey];
		}
	};

	template <typename N>
	static
	DRAIN_XML_TREE(N) & appendHeaderObject(DRAIN_XML_TREE(N) & root, typename N::xml_tag_t tag,
			const typename DRAIN_XML_TREE(N)::path_elem_t & key = typename DRAIN_XML_TREE(N)::path_elem_t()){

		const typename DRAIN_XML_TREE(N)::path_elem_t & finalKey = !key.empty() ? key : Enum<typename N::xml_tag_t>::getKey(tag);

		if (!root.hasChild(finalKey)){
			DRAIN_XML_TREE(N) & child = root.add(finalKey); // consider path type! getDefaultObject
			child->setType(tag);
			return child;
		}
		else {
			return root[finalKey];
		}
	};
	*/


	template <class N>
	static
	DRAIN_XML_TREE(N) & getOnLoadScript(DRAIN_XML_TREE(N) & root);


	template <class N>
	static inline
	DRAIN_XML_TREE(N) & ensureScript(DRAIN_XML_TREE(N) & root,  const std::string & label, const std::string & code){
		DRAIN_XML_TREE(N) & scriptElem = UtilsXML::getHeaderObject(root, N::xml_tag_t::SCRIPT, label);
		scriptElem = code;
		return scriptElem;
	}

	// See StringTools::getSafeKey void getSafeVariableName();

	///
	/**
	 *  Creates a function in a shared /SCRIPT/ slot.
	 *  Todo: rename
	 *
	 *  \tparam N - Tree node type (T::node_data_t)
	 *  \tparam TT - function parameter names (strings).
	 */

	/**
	 *   Creates...
	 */
	template <typename N, typename ...TT>
	static
	DRAIN_XML_TREE(N) & ensureJavaScriptFunction(DRAIN_XML_TREE(N) & root, const std::string & name, const TT & ...args){

		// typedef UnorderedMultiTree<N> T;
		// T & jsFunction = getHeaderObject(root, T::node_data_t::xml_tag_t::SCRIPT, name);
		// T & jsFunctionScope = jsFunction[N::xml_tag_t::SCOPE_CURLY](N::xml_tag_t::SCOPE_CURLY);
		DRAIN_XML_TREE(N) & jsFunction = UtilsXML::getHeaderObject(root, N::xml_tag_t::SCRIPT);

		DRAIN_XML_TREE(N) & jsFunctionScope = jsFunction[name](N::xml_tag_t::SCOPE_CURLY);

		if (jsFunctionScope.empty()){
			jsFunctionScope->setText("function ", name, '(', StringBuilder<','>(args...), ')');
			// jsFunction.addChild()->setComment("end of ", name);
			jsFunction.addChild()->setText('\n');
		}


		return jsFunctionScope; // [XML::SCOPE_CURLY];
	}

	/**
	 *  \param path
	 */
	template <typename N>
	static
	DRAIN_XML_TREE(N) & ensureJavaScriptUrl(DRAIN_XML_TREE(N) & root, const std::string & url){

		drain::Logger mout(__FILE__, __FUNCTION__);

		// typedef DRAIN_XML_TREE(N) T;

		std::string id(url);
		for (auto & c : id){
			// drain::PathSeparatorPolicy
			if (c == DRAIN_XML_TREE(N)::path_t::separator.character){
				c = '|';
			}
		}
		mout.note("mapped id:", id);

		DRAIN_XML_TREE(N) & treeJS = UtilsXML::getHeaderObject(root, N::xml_tag_t::SCRIPT, id);

		treeJS->setUrl(url);

		return treeJS; // [XML::SCOPE_CURLY];
	}


};

template <class N>
DRAIN_XML_TREE(N) & JavaScriptXML::getOnLoadScript(DRAIN_XML_TREE(N) & root){

	drain::Logger mout(__FILE__, __FUNCTION__);

	static
	const std::string onload_fnc_name("drain_onload");

	mout.experimental<LOG_INFO>("Adding mouse interaction: SVG onload=", onload_fnc_name, "()");

	root->setAttribute("onload", onload_fnc_name, "()"); // perhaps repeatedly

	return ensureJavaScriptFunction(root, onload_fnc_name); // [svg::SCOPE_CURLY](svg::SCOPE_CURLY);
	// return drain::UtilsXML::ensureJavaScriptFunctionScope(ctx.getSVG(), onload_fnc_name);


}

}  // drain::

#endif /* DRAIN_XML */


