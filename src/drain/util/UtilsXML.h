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
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
 */
/*
 * UtilsXML.h
 *
 *      Author: mpeura
 */

#ifndef DRAIN_UTILS_XML
#define DRAIN_UTILS_XML

#include "XML.h"
#include "SelectorXML.h" // Only for StyleUtils :-/

namespace drain {


/// Functions for manipulating XML structures.
class UtilsXML {

public:

	///
	/**
	 *   Forward definition – type can be set only upon construction of a complete class
	 *
	 */
	template <typename TX>
	static inline
	TX & setType(TX & tree, const typename TX::node_data_t::xml_tag_t & type){
		tree->setType(type);
		return tree;
	}

	template <typename TX, typename ...TT>
	static TX& setTypeAndID(TX &tree, const typename TX::node_data_t::xml_tag_t &type, const TT & ...args);


	/// Find node type (tag id) from a predefined list of default types for (some) parent types.
	/**
	 *
	 */
	template <typename N>
	static
	typename N::xml_tag_t retrieveDefaultType(const N & parentNode){
		typedef typename N::xml_default_elem_map_t map_t;
		const typename map_t::const_iterator it = N::xml_default_elems.find(parentNode.getNativeType());
		if (it != N::xml_default_elems.end()){
			return (it->second);
		}
		else {
			return static_cast<typename N::xml_tag_t>(0);
		}
	}

	template <typename T>
	static
	bool initChildWithDefaultType(const T & tree, T & child){
		typename T::node_data_t::xml_tag_t type = retrieveDefaultType(tree.data);
		if (static_cast<int>(type) != 0){
			child->setType(type);
			return true;
		}
		else {
			return false;
		}
	}

	/// Assign another tree structure to another
	/**
	 *  \tparam XML - xml tree structure (TreeXML, TreeSVG, TreeHTML)
	 */
	template <typename T>
	static inline
	T & assign(T & dst, const T & src){

		if (&src != &dst){
			dst.clear(); // clears children...
			// ... but not copying src? (TreeUtils?)
			// also dst->clear();
			assignNode(dst.data, src); // removed - so is this polymorphic method ever used.
		}

		return dst;
	}

	/// Copy node data to tree
	/**
	 *  \tparam XML - xml tree structure (TreeXML, TreeSVG, TreeHTML)
	 */
	template <typename TX>
	static inline
	TX & assign(TX & dst, const typename TX::xml_node_t & src){
		assignNode(dst.data, src);
		return dst;
	}


	/// Assign property to a XML tree node
	/**
	 *  \tparam T - XML tree
	 */
	template <typename T, typename V>
	static inline
	T & assign(T & tree, const V & arg){
		tree->set(arg);
		return tree;
	}

	/// Tree
	/**
	 *  \tparam TX - xml tree
	 */
	template <typename T>
	static
	//T & assign(T & tree, std::initializer_list<std::pair<const char *,const char *> > l){
	T & assign(T & tree, std::initializer_list<std::pair<const char *,const Variable> > l){

		//switch (static_cast<intval_t>(tree->getType())){
		switch (tree->getType()){
		case XML::STYLE:
			for (const auto & entry: l){
				T & elem = tree[entry.first];
				elem->setType(XML::STYLE_SELECT);
				drain::MapTools::setValues(elem->getAttributes(), entry.second, ';', ':', std::string(" \t\n"));
			}
			break;
		case XML::UNDEFINED:
			tree->setType(XML::STYLE_SELECT);
			// no break
		case XML::STYLE_SELECT:
		default:
			tree->set(l);
			break;
		}

		return tree;
	};

	// UNDER CONSTRUCTION!
	/// When assigning a string, create new element unless the element itself is of type CTEXT.
	/**
	 *   \return - text element (CTEXT): current or child element of the current element
	 *
	 *   Forward definition – type can be set only upon construction of a complete class
	 */
	template <typename TX>
	static inline  // NOT YET as template specification of assign(...)
	TX & assignString(TX & tree, const std::string & s){
		if (tree->isUndefined()){
			tree->setType(XML::CTEXT);
		}
		tree->ctext = s;
		return tree;
	}

	template <typename TX>
	static inline  // NOT YET as template specification of assign(...)
	TX & appendString(TX & tree, const std::string & s){
		if (tree->isCText()){
			tree->ctext += s;
			return tree;
		}
		else if (tree->isUndefined()){
			tree->setType(XML::CTEXT);
			// tree->setText(s);
			tree->ctext += s;
			return tree;
		}
		else {
			// drain::Logger(__FILE__, __FUNCTION__).error("Assign string...");
			TX & child = tree.addChild();
			child->setType(XML::CTEXT);
			child->setText(s);
			return child;
		}
	}



	template <typename T>
	static
	T & getHeaderObject(T & root, typename T::node_data_t::xml_tag_t tag, const typename T::path_elem_t & key = typename T::path_elem_t()){

		const typename T::path_elem_t & finalKey = !key.empty() ? key : Enum<typename T::node_data_t::xml_tag_t>::getKey(tag);

		if (!root.hasChild(finalKey)){
			T & child = root.prependChild(finalKey); // consider path type! getDefaultObject
			child->setType(tag);
			return child;
		}
		else {
			return root[finalKey];
		}
	};

	template <typename T>
	static
	T & appendHeaderObject(T & root, typename T::node_data_t::xml_tag_t tag, const typename T::path_elem_t & key = typename T::path_elem_t()){

		const typename T::path_elem_t & finalKey = !key.empty() ? key : Enum<typename T::node_data_t::xml_tag_t>::getKey(tag);

		if (!root.hasChild(finalKey)){
			T & child = root.add(finalKey); // consider path type! getDefaultObject
			child->setType(tag);
			return child;
		}
		else {
			return root[finalKey];
		}
	};

	/// If element is of type STYLE, return it. If not, return header object of type STYLE
	/**
	 *   The header object is in standard
	 *
	 *   \tparam T - XML tree type
	 *   \param elem
	 */
	template <typename T>
	static inline
	T & ensureStyleElem(T & elem){
		if (elem->typeIs(T::node_data_t::xml_tag_t::STYLE)){
			return elem;
		}
		else {
			return getHeaderObject(elem, T::node_data_t::xml_tag_t::STYLE);
		}
	}

	template <typename T>
	static
	T & ensureStyle(T & elem, const SelectXML<typename T::node_data_t::xml_tag_t> & selector, const std::initializer_list<std::pair<const char *,const Variable> > & styleDef){

		//T & style = getHeaderObject(elem, T::node_data_t::xml_tag_t::STYLE);
		T & style = ensureStyleElem(elem);

		T & styleEntry = style[selector];
		if (styleEntry.empty()){
			styleEntry = styleDef;
			// styleEntry->setStyle(styleDef); WRONG (did not work)
		}
		return styleEntry;
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
	template <typename N, typename ...TT>
	static
	UnorderedMultiTree<N> & ensureJavaScriptFunctionScope(UnorderedMultiTree<N> & root, const std::string & name, const TT & ...args){

		typedef UnorderedMultiTree<N> T;
		T & scriptElem = getHeaderObject(root, T::node_data_t::xml_tag_t::SCRIPT);

		if (!scriptElem.hasChild(name)){
			T & jsFunction = scriptElem[name];
			//jsFunction->setType(N::xml_tag_t::JAVASCRIPT_SCOPE);
			jsFunction->setText("var ", name, " = function(", args..., ')');
			// T & jsFunctionScope = jsFunction.addChild();
			T & jsFunctionScope = jsFunction[N::xml_tag_t::JAVASCRIPT_SCOPE]; // maybe string best?
			jsFunctionScope->setType(N::xml_tag_t::JAVASCRIPT_SCOPE);
			return jsFunctionScope;
		}
		else {
			return scriptElem[name][N::xml_tag_t::JAVASCRIPT_SCOPE];
		}
		// T & jsFunction = getHeaderObject(root, T::node_data_t::xml_tag_t::SCRIPT);

	}

	/**
	 *   Creates...
	 */
	template <typename N, typename ...TT>
	static
	UnorderedMultiTree<N> & ensureJavaScriptFunction(UnorderedMultiTree<N> & root, const std::string & name, const TT & ...args){

		typedef UnorderedMultiTree<N> T;
		T & jsFunction = getHeaderObject(root, T::node_data_t::xml_tag_t::SCRIPT, name);

		if (jsFunction.empty()){
			jsFunction->setText("function ", name, '(', args..., ')');
		}

		return jsFunction; // [XML::JAVASCRIPT_SCOPE];
	}

	/**
	 *  \param path
	 */
	template <typename N>
	static
	UnorderedMultiTree<N> & ensureJavaScriptUrl(UnorderedMultiTree<N> & root, const std::string & url){

		drain::Logger mout(__FILE__, __FUNCTION__);

		typedef UnorderedMultiTree<N> T;

		std::string id(url);
		for (auto & c : id){
			// drain::PathSeparatorPolicy
			if (c == T::path_t::separator.character){
				c = '|';
			}
		}
		mout.note("mapped id:", id);

		T & treeJS = getHeaderObject(root, T::node_data_t::xml_tag_t::SCRIPT, id);

		treeJS->setUrl(url);

		return treeJS; // [XML::JAVASCRIPT_SCOPE];
	}


};

template<typename TX, typename ...TT>
inline
TX& UtilsXML::setTypeAndID(TX &tree, const typename TX::node_data_t::xml_tag_t &type, const TT & ...args) {
	tree->setType(type);
	tree->setId(type, drain::StringBuilder<'_'>(TX::node_data_t::getCount(), args...));
	return tree;
}

/// Utility for automatically setting child element types. Uses xml_default_elems .
/**
 *   \b Usage
 *
 *   In header files:
 *   \code{.cpp}
 *   namespace drain {
 *   DRAIN_XML_DEFAULT_INIT(image::TreeSvg);
 *   }
 *   \endcode
 *
 *   In code:
 *   \code{.cpp}
 *   // Assume TreeSvg script is of type svg::SCRIPT.
 *   image::TreeSvg & line1 = script.addChild(); // Set type xml::CTEXT.
 *   image::TreeSvg & line2 = script["l2"];      // Set type xml::CTEXT.
 *   \endcode
 */
#define DRAIN_XML_DEFAULT_INIT(xml_tree) template <> inline void xml_tree::initChild(xml_tree & child) const { UtilsXML::initChildWithDefaultType(*this, child); }

/// Set type with simple operator().
/**
 *   \b Usage
 *
 *   In header files:
 *   \code
 *   namespace drain {
 *   DRAIN_XML_DEFAULT_ELEMS_INIT(image::TreeSvg);
 *   }
 *
 *   In code:
 *   \code
 *   image::TreeSvg & tree = tree["image1"](svg::IMAGE); // Set type svg::IMAGE.
 *   \endcode
 */
#define DRAIN_XML_EASY_TYPE(xml_tree) template <> template <> inline xml_tree & xml_tree::operator()(const xml_tree::node_data_t::tag_t & type){ return UtilsXML::setType(*this, type); }


}  // drain::

#endif /* DRAIN_XML */


