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


/// Base class for XML "nodes", to be data elements T for drain::Tree<T>
class UtilsXML {

public:


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

	template <typename T>
	static
	T & getHeaderObject(T & root, typename T::node_data_t::xml_tag_t tag, const typename T::path_elem_t & key = typename T::path_elem_t()){

		const typename T::path_elem_t & finalKey = !key.empty() ? key : EnumDict<typename T::node_data_t::xml_tag_t>::getKey(tag);

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
	T & ensureStyle(T & root, const SelectXML<typename T::node_data_t::xml_tag_t> & selector, const std::initializer_list<std::pair<const char *,const Variable> > & styleDef){

		T & style = getHeaderObject(root, T::node_data_t::xml_tag_t::STYLE);

		T & styleEntry = style[selector];
		if (styleEntry.empty()){
			styleEntry = styleDef;
			// styleEntry->setStyle(styleDef); WRONG (did not work)
		}
		return styleEntry;
	}


};



}  // drain::

#endif /* DRAIN_XML */

/*
template <typename T>
static
T & xmlGuessType(const typename T::node_data_t & parentNode, T & child){
	typedef typename T::node_data_t::xml_default_elem_map_t map_t;
	const typename map_t::const_iterator it = T::node_data_t::xml_default_elems.find(parentNode.getNativeType());
	if (it != T::node_data_t::xml_default_elems.end()){
		child->setType(it->second);
		drain::Logger(__FILE__, __FUNCTION__).experimental<LOG_WARNING>("Default type set: ", child->getTag());
	}
	return child;
}
*/

/// Assign tree node (data) to another
/**
 *  \tparam N - xml node (e.g. NodeXML, NodeSVG, NodeHTML)
 *
 *  \see clear()
template <typename N>
static inline
N & assignNode(N & dst, const N & src){

	if (&src != &dst){
		//dst.clear(); // clear attributes,
		//if (!dst.typeIs(src.getNativeType())){
		if (dst.getType() != src.getType()){
			dst.reset(); // clear attributes, style, cstring and type.
			// Warning: does not create links.
			dst.setType(src.getType());
		}
		dst.getAttributes().importMap(src.getAttributes());
		dst.setStyle(src.getStyle());
		// dst.setText(src.ctext); // wrong! set type to CTEXT
		dst.ctext = src.ctext;
	}

	return dst;
}
 */


/**
 *
 *  TODO: add default type based on parent group? defaultChildMap TR->TD
 *
 */
/*
template <typename T>
static
T & addChild(T & tree){
	typename T::node_data_t::xml_tag_t type = retrieveDefaultType(tree.data);
	std::stringstream k; // ("elem");
	k << "elem"; // number with 4 digits overwrites this?
	k.width(3);  // consider static member prefix
	k.fill('0');
	k << tree.getChildren().size();
	return tree[k.str()](type);
}

template <typename T>
static
T & addChild(T & tree, const std::string & key){

	if (!key.empty()){
		typename T::node_data_t::xml_tag_t type = xmlRetrieveDefaultType(tree.data);
		return tree[key](type);
	}
	else {
		return addChild(tree);
	}
}
*/


// TX & xmlAppendString(TX & tree, const std::string & s){
/*
template <class V>
static inline
void attribToStream(std::ostream &ostr, const std::string & key, const V &value){
	//StringTools::replace(XML::encodingMap, data.ctext, ostr);
	//ostr << ' ' << key << '=' << '"' << value << '"'; // << ' ';

	static const std::map<char,char> keyMap = {
			{' ','_'},
			{'"','_'},
			{'=','_'},
	};
	ostr << ' ';
	StringTools::replace(key, keyMap, ostr); // XML::encodingMap
	//StringTools::replace(getEntityMap(), key, ostr); // XML::encodingMap

	static const std::map<char,std::string> valueMap = {
			{XML::entity_t::QUOTE, "'"},
			{XML::entity_t::LESS_THAN,"(("},
			{XML::entity_t::GREATER_THAN,"))"},
	};
	ostr << '=' << '"';
	StringTools::replace(value, valueMap, ostr); // XML::encodingMap
	//StringTools::replace(getEntityMap(), value, ostr); // XML::encodingMap
	ostr << '"';
	//<< key << '=' << '"' << value << '"'; // << ' ';
}
*/

