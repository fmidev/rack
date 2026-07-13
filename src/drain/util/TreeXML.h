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
 * TreeXML.h
 *
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */



#ifndef DRAIN_TREE_XML
#define DRAIN_TREE_XML

#include <ostream>

#include "TreeUnordered.h"
#include "XML.h"


namespace drain {

/**
 *  \tparam T - index type; may be enum.
 */
template <class T=int>
class NodeXML : public XML {

public:

	typedef T xml_tag_t; // "final"
	//typedef T tag_t;
	typedef NodeXML<T> xml_node_t;
	typedef drain::Path<std::string,'/'> path_t; // basically, also path_elem_t could be a template.
	typedef UnorderedMultiTree<xml_node_t,false, path_t> xml_tree_t;

	/// A "handle" for specialized element classes, i.e. with members like \c width , \c height or \c radius .
	/**
	 *   The members should be implemented as drain::FlexibleVariable &, instantiated like \c height(node["height"] = 0) .
	 *
	 *   Example:
	 *   \code
	 *		template <>
			template <>
			class NodeXML<image::svg::tag_t>::Elem<image::svg::tag_t::CIRCLE>{

				public:

				inline
				Elem(image::NodeSVG & node) : x(node["x"]), y(node["y"]), radius(node["radius"]){
					node.setType(image::svg::tag_t::CIRCLE);
				};

				FlexibleVariable & x;
				FlexibleVariable & y;
				FlexibleVariable & radius;

			};
	 *   \endcode
	 *
	 *   \see drain::image::NodeSVG
	 */
	template <T ELEM>
	class Elem; // {} ?

	/// Provides access to ReferenceMap2 of XML elements, to link FlexibleVariables.
	/**
	 *  \see TreeElemUtilsHTML
	 *  \see TreeElemUtilsSVG
	 */
	template <T ELEM>
	friend class Elem;

	/// Helps creating child elements. Like children of HTML element UL should be LI.
	typedef std::map<xml_tag_t,xml_tag_t> xml_default_elem_map_t;

	static const xml_default_elem_map_t xml_default_elems;

	/// Default constructor
	/**
	 *  In derived classes, setting type should be always called by constructor.
	 */
	inline
	NodeXML(){
		// id = drain::StringBuilder<>('e', ++nextID);
	};

	//
	/// Copy constructor
	/**
	 *  In derived classes, handleType(static_cast<T>(t)) should be always called
	 *  by constructor.
	 *
	 *  Use always default constructor in derived classes? Assign node can violate/invalidate link() pointers?
	 */
	inline
	NodeXML(const NodeXML & node){
		id = drain::StringBuilder<>(node.getTag(), ++nextID);
		// drain::StringTools::import(++nextID, id);
		// XML::xmlAssignNode(*this, node); // RISKY!? Should be called by the copy constructors of derived classes.
	}

	virtual inline
	~NodeXML(){};

	/// Default implementation of tag name retrieval based on drain::Enum<t>.
	/**
	 *   This function should not be called upon construction, as the dictionary does not exist.
	 *
	 */
	virtual inline
	const std::string & getTag() const override {
		// std::cout << __FILE__ << ':' << __FUNCTION__ << ' ' << drain::TypeName<T>::str() << " dict:"  << sprinter(drain::Enum<T>::dict) << std::endl; // <<
		return drain::Enum<T>::getDict().getKey((T)type, false);
	}

	/**
	 *   This function should not be called upon construction. Consider dynamic getDict() to skip
	 */
	static inline
	const std::string & getTag(const T & type){
		// std::cout << __FILE__ << ':' << __FUNCTION__ << ' ' << drain::TypeName<T>::str() << " dict:"  << sprinter(drain::Enum<T>::dict) << std::endl; // <<
		return drain::Enum<T>::getDict().getKey((T)type, false);
	}

	/// Change attributes, styles and classes of a node.
	/**
	 *  Explicit naming to avoid shadowing map<>::swap().
	 *
	 *  TODO: check link (drain::Reference) stability
	 */
	// virtual void swapNode(NodeXML<T> & node);


	//
	inline
	T getNativeType() const {
		return static_cast<T>(type); // may fail! consider two-way conversion assert
	};


	inline
	void set(const NodeXML<T> & node){
		if (isUndefined()){
			// Should make sense, and be safe. Esp. when exactly same node type, by templating
			setType(node.getType());
		}
		else if (type == STYLE) {
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.suspicious("copying STYLE from node: ", node);
		}
		drain::MapTools::setValues<map_t>(getAttributes(), node.getAttributes());
	}


	inline
	void set(const intval_t & type){
		setType(type);
	}


	inline
	void set(const std::string & s){
		setText(s);
	}

	inline
	void set(const char *s){
		set(std::string(s));
	}

	inline
	void set(const drain::Castable & s){
		setText(s);
	}


	inline
	void set(const std::initializer_list<std::pair<const char *,const Variable> > & args){
		// TODO: redirect to set(key,value), for consistency?
		if (type == STYLE){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.deprecating("Setting attributes/style of a STYLE element.");
			setStyle(args);
			/*
			for (const auto & entry: l){
				style[entry.first] =  entry.second;
			}
			*/
			// drain::SmartMapTools::setValues(style, l);
		}
		else {
			for (const auto & entry: args){
				getAttributes()[entry.first] =  entry.second;
			}
			///drain::SmartMapTools::setValues(getAttributes(), l);       // add new keys
			// drain::SmartMapTools::setValues<map_t,false>((map_t &)*this, l);   // update only
		}
	}

	template <class V>
	inline
	void set(const std::map<std::string, V> & args){
		// TODO: redirect to set(key,value), for consistency
		if (type == STYLE){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.deprecating("Setting attributes/style of a STYLE element: "); //, args);
			//drain::SmartMapTools::setValues(style, args);
			setStyle(args);
			/*
			for (const auto & entry: args){
				style[entry.first] =  entry.second;
			}
			*/
		}
		else {
			drain::MapTools::setValues(getAttributes(), args);       // add new keys
			// drain::SmartMapTools::setValues<map_t,true>(getAttributes(), args);       // add new keys
			// drain::SmartMapTools::setValues<map_t,false>((map_t &)*this, l);   // update only
		}
	}


	template <class V>
	inline
	void set(const std::string & key, const V & value){

		if (this->isStyle()){
			// Modify collection directly
			drain::Logger mout(__FILE__, __FUNCTION__);
			// Well, this should actually be ok, as it is in the standard?
			mout.deprecating("Setting style as attributes of a STYLE element: ", key, ':', value);
			setStyle(key, value);
		}
		else if (key == "style"){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.obsolete("Setting style as attribute: \"style\"=", value);
		}
		else if (key == "class"){
			// mout.warn<LOG_DEBUG>("class");
			std::string cls;
			drain::ClassXML xls;
			drain::StringTools::import(value, cls); // Or Convert?
			addClass(cls);
			//drain::Logger(__FILE__, __FUNCTION__).revised("addClass:", value);
			// addClass(value);
		}
		else {
			setAttribute(key, value);
		}

	}





	inline
	NodeXML & operator=(const NodeXML & node){
		return XML::xmlAssignNode(*this, node);
	}

	// Check char *
	inline
	NodeXML & operator=(const xml_tag_t & x){ // needed? possibly with tree(int x) ?
		setType(x);
		return *this;
	}

	inline
	NodeXML & operator=(const Castable &c){
		setText(c);
		return *this;
	}


	inline
	NodeXML & operator=(const std::string &s){
		setText(s);
		return *this;
	}

	inline
	NodeXML & operator=(const char *s){
		setText(s);
		return *this;
	}

	inline
    NodeXML & operator=(const std::initializer_list<std::pair<const char *,const drain::Variable> > &l){
		set(l);
		return *this;
	}


	/// Dumps info. Future option: outputs leading and ending tag
	// std::ostream & nodeToStream(std::ostream & ostr, tag_display_mode mode=EMPTY_TAG) const;
	/**
	 *  \see XML::toStream()
	 */
	virtual
	std::ostream & nodeToStream(std::ostream & ostr, tag_display_mode mode=EMPTY_TAG) const override;


	template <class V>
	static inline
	std::ostream & docToStream(std::ostream & ostr, const V & tree){
		V::node_data_t::xml_node_t::docTypeToStream(ostr);
		V::node_data_t::xml_node_t::toStream(ostr, tree);
		// UtilsXML::toStream(ostr, tree);
		return ostr;
	}


	/// Write the XML definition beginning any XML document.
	/**
	 *  Each derived class should have a definition compatible with the structure generated by the class.
	 *
	 *  To override, use specialization of the template(d) base type:
	    \code
	    template <>
        std::ostream & NodeXML<BaseHTML::tag_t>::docTypeToStream(std::ostream &ostr){
		  ostr << "<!DOCTYPE html>\n";
		  return ostr;
        }
        \endcode
	 */
	static // virtual
	inline
	std::ostream & docTypeToStream(std::ostream &ostr){
		ostr << "<?xml ";
		for (const auto & entry: xmldoc_attribs){
			xmlAttribToStream(ostr, entry.first, entry.second);
		}
		ostr << "?>";
		ostr << '\n';
		return ostr;
	}


protected:



	/// Control constant variables of  (note: templated, through Node<T>::)
	typedef std::list<std::pair<std::string,std::string> > xmldoc_attrib_map_t;
	static xmldoc_attrib_map_t xmldoc_attribs;


};

/*
template <class T>
void NodeXML<T>::swapNode(NodeXML<T> & node){

	if (this != &node){

		// Swap attributes
		// Risky? FlexibleVariableMap contains references. Should be moved with copy-struct-kind of mechanism...
		map_t::swap(node);
		// Swap text content

		ctext.swap(node.ctext);

		// Swap classes
		classList.swap(node.classList);

		style.swap(node.style);
	}
	else {
		// // Consider conditional (strict => exception)
	}

}
*/




/*
template <>
inline
const std::string & NodeXML<int>::getTag() const {
	return drain::Enum<int,XML>::dict.getKey(this->type, false);
}
*/
#define DRAIN_XML_TREE(N) drain::UnorderedMultiTree<N,false, NodeXML<>::path_t>

//typedef NodeXML<>::xml_tree_t TreeXML;
typedef DRAIN_XML_TREE(NodeXML<>) TreeXML;

//typedef drain::UnorderedMultiTree<NodeXML<>,false, NodeXML<>::path_t> TreeXML;
//#define DRAIN_XML_TREE(N) NodeXML<N>::xml_tree_t

// NOTE: template will not match for subclasses of NodeXML<E> because default template will match any class exactly.
template <class E, bool EX, class P>
struct TypeName< drain::UnorderedMultiTree<NodeXML<E>,EX,P> > {

    static const std::string & str(){
    	static const std::string name = drain::StringBuilder<>("TreeXML<", TypeName<E>::str(), ">");
    	return name;
    }

};




template <class N>
std::ostream & NodeXML<N>::nodeToStream(std::ostream &ostr, tag_display_mode mode) const {


	if (isCText()){
		if (mode != CLOSING_TAG){ // or: OPENING
			ostr << ctext; //  << '(' << mode << ')'; // << " /-->\n";
		}
		return ostr;
	}
	else if (isComment()){
		// Remember: a large subtree can be commented out
		if (mode != CLOSING_TAG){
			ostr << "<!--"; // << ctext; // << " /-->\n";
		}
	}
	else if (isScopeJS()){
		if (mode != CLOSING_TAG){
			ostr << ctext << "{\n";
		}
		else {
			// Newline good in avoiding potential comment line
			ostr << "\n}";
		}
		return ostr;
	}
	else {

		/*
		if (isUndefined()){
			drain::Logger(__FILE__, __FUNCTION__).warn("Undefined TAG type for ", getName(), "  ID=", getId(), " attr=", drain::sprinter(getAttributes()));
		}
		*/

		/*
		if (getTag().empty()){
			drain::Logger(__FILE__, __FUNCTION__).unimplemented<LOG_ERR>("defaultTag for type=", getType(), " requested by ID=", getId(), " attr=", drain::sprinter(getAttributes()));

			ostr << "defaultTag"; // << ' ';  FIX! getDefaultTag?
		}*/
		if (isUndefined() || getTag().empty()){
			drain::Logger(__FILE__, __FUNCTION__).warn("Undefined TAG type for ", getName(), "  ID=", getId(), " attr=", drain::sprinter(getAttributes()));
			//if (mode != CLOSING_TAG){
			ostr << "<!-- " << getTag() << " tag? " << ctext << " /-->";
			//}
		}
		else {

			if (mode==CLOSING_TAG){
				ostr << "</";
			}
			else {
				ostr << '<';
			}

			ostr << getTag(); // << ' ';
			// TODO check GDAL XML
			// if (!defaultTag.empty())
			// attribToStream(ostr, "name", defaultTag);
		}
	}

	if (mode != CLOSING_TAG){

		if (typeIs(STYLE)){
			 // xmlAttribToStream(ostr, "data-mode", "experimental");
		}
		else { // if (!isCText())

			specificAttributesToStream(ostr);

			char sep=0;

			// Iterate attributes - note: also for comment
			/*
			for (const auto & key: getAttributes().getKeyList()){
				std::string v = get(key, "");
				// Skip empties (so Sprinter::toStream not applicable)
				if (!v.empty()){
					xmlAttribToStream(ostr, key, v);
					sep=' ';
				}
			}
			*/
			for (const auto & entry: getMap()){
				//std::string v = get(entry.first, std::string(""));
				std::string v = get(entry.first, "");
				// Skip empties (so Sprinter::toStream not applicable)
				if (!v.empty()){
					xmlAttribToStream(ostr, entry.first, v);
					sep=' ';
				}
			}

			// Handle CSS style separately
			if (!style.empty()){
				if (sep)
					ostr << sep;
				ostr << "style=\"";
				Sprinter::sequenceToStream(ostr, style, StyleXML::styleLineLayout);
				ostr << '"'; // << ' ';
			}

			/*
			if (!ctext.empty()){
				ostr << "  "; // debugging
			}
			*/
		}

	}

	// END
	if (isComment()){
		ostr << ' '; // prefix for free ctext
		if (mode != OPENING_TAG){
			// it is EMPTY_TAG or CLOSING TAG
			ostr << "/-->"; // \n
		}
		// warn if children?
	}
	else if (isUndefined() || getTag().empty()){
		/*
		ostr << ' '; // prefix for free ctext
		if (mode != OPENING_TAG){
			// it is EMPTY_TAG or CLOSING TAG
			ostr << "<!--  end /-->"; // \n
		}
		*/
	}
	else if (mode==EMPTY_TAG){ // OR no ctext!
		// close TAG
		ostr << "/>"; // \n <- check newline - add before indent?
	}
	else {
		ostr << ">";
	}

	return ostr;

}


template <class N>
inline
std::ostream & operator<<(std::ostream &ostr, const NodeXML<N> & node){
	return node.nodeToStream(ostr);
}

/// XML output function shared for all XML documents, ie versions defined in subclasses.
/**
 *  TODO: preamble/prologToStream()
 *
 *   \param defaultTag - important for
 *
 */

/**
 *  \tparam  int - type of the values in the dictionary
 *  \tparam  XML - the owner of the dictionary
 */
DRAIN_ENUM_DICT2(int,XML);



template <class E, bool EX, class P>
std::ostream & operator<<(std::ostream &ostr, const UnorderedMultiTree<NodeXML<E>,EX,P> & tree){
	// DOC def? TODO: preamble/prologToStream()
	NodeXML<E>::docTypeToStream(ostr); // must be member, to support virtual?
	NodeXML<E>::toStream(ostr, tree, "");
	return ostr;
}



}  // drain::

/// Utility for declaring a default tag type for children of elements of each tag type.
/**
 *   \b Usage
 *
 *   Header files:
 *   \code
 *   namespace drain {
 *   DRAIN_XML_DEFAULT_ELEMS(image::TreeSvg);
 *   }
 *   \endcode
 *
 *   Definition files:
 *   \code
 *   namespace drain {
 *   DRAIN_XML_DEFAULT_ELEMS(image::TreeSvg) = {
 *      {drain::image::svg::SCRIPT, drain::image::svg::CTEXT},
 *      ...
 *   }
 *   \endcode
 *
 *   \param xml_tree - XML Tree type, like TreeSVG, TreeSLD or TreeGDAL.
 */
#define DRAIN_XML_DEFAULT_ELEMS(xml_tree) template <> const NodeXML<xml_tree::node_data_t::tag_t>::xml_default_elem_map_t NodeXML<xml_tree::node_data_t::tag_t>::xml_default_elems


/// Enable using keys of enum_type to be used as path elements.
#define DRAIN_XML_ENUM_KEY(xml_tree, enum_type) template <> template <> inline const xml_tree::key_t & xml_tree::getKey(const enum_type & type){ return Enum<enum_type>::dict.getKey(type, false); }


#endif /* DRAIN_TREE_XML */

