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
#include "UtilsXML.h"


namespace drain {


/**
 *  \tparam T - index type; may be enum.
 */
template <class T=int>
class NodeXML : public XML {

public:

	typedef T xml_tag_t;
	typedef NodeXML<T> xml_node_t;
	typedef drain::Path<std::string,'/'> path_t; // basically, also path_elem_t could be a template.
	typedef UnorderedMultiTree<xml_node_t,false, path_t> xml_tree_t;

	inline
	NodeXML(const intval_t & t = intval_t(0)){
		setType(t);
		drain::StringTools::import(++nextID, id);
	};

	// Note: use default constructor in derived classes.
	inline
	NodeXML(const NodeXML & node){
		XML::xmlAssignNode(*this, node);
		drain::StringTools::import(++nextID, id);
	}

	inline
	~NodeXML(){};


	template <class T2> // "final"
	void setType(const T2 &t){ // DANGER, without cast?
		type = static_cast<intval_t>(t); // (elem_t)
		handleType(static_cast<T>(t));
		// in derived classes, eg. drain::image::BaseGDAL
		// warning: case value ‘...’ not in enumerated type
	}

	/* Consider this later, for user-defined (not enumerated) tag types.
	virtual
	void setType(const std::string & type);
	*/

	inline
	const intval_t & getType() const {
		return type;
	};

	inline
	T getNativeType() const {
		return static_cast<T>(type); // may fail! consider two-way conversion assert
	};


public:



	/**
	 *   This function should not be called upon construction, as the dictionary does not exist.
	 */
	virtual
	const std::string & getTag() const {
		// std::cout << __FILE__ << ':' << __FUNCTION__ << ' ' << drain::TypeName<T>::str() << " dict:"  << sprinter(drain::EnumDict<T>::dict) << std::endl; // <<
		return drain::EnumDict<T>::getDict().getKey((T)type, false);
	}

	/**
	 *   This function should not be called upon construction. Consider dynamic getDict() to skip
	 */
	static inline // needed?
	const std::string & getTag(const T & type){
		// std::cout << __FILE__ << ':' << __FUNCTION__ << ' ' << drain::TypeName<T>::str() << " dict:"  << sprinter(drain::EnumDict<T>::dict) << std::endl; // <<
		return drain::EnumDict<T>::getDict().getKey((T)type, false);
	}


	inline
	void set(const NodeXML & node){
		if (isUndefined()){
			// Should make sense, and be safe. Esp. when exactly same node type, by templating
			setType(node.getType());
		}
		else if (type == STYLE) {
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.suspicious("copying STYLE from node: ", node);
		}
		drain::SmartMapTools::setValues<map_t>(getAttributes(), node.getAttributes());
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

	/*
	inline
	void set(const std::initializer_list<Variable::init_pair_t > &l){
		// TODO: redirect to set(key,value), for consistency
		if (type == STYLE){
			drain::SmartMapTools::setValues(style, l);
		}
		else {
			drain::SmartMapTools::setValues<map_t,true>(getAttributes(), l);       // add new keys
			// drain::SmartMapTools::setValues<map_t,false>((map_t &)*this, l);   // update only
		}
	}
	*/

	inline
	//void set(const std::initializer_list<std::pair<const char *,const char *> > & args){
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
			mout.deprecating("Setting attributes/style of a STYLE element: ", args);
			//drain::SmartMapTools::setValues(style, args);
			setStyle(args);
			/*
			for (const auto & entry: args){
				style[entry.first] =  entry.second;
			}
			*/
		}
		else {
			drain::SmartMapTools::setValues<map_t,true>(getAttributes(), args);       // add new keys
			// drain::SmartMapTools::setValues<map_t,false>((map_t &)*this, l);   // update only
		}
	}


	template <class V>
	inline
	void set(const std::string & key, const V & value){
		if (type == STYLE){
			// Modify collection directly
			drain::Logger mout(__FILE__, __FUNCTION__);
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
			drain::StringTools::import(value, cls);
			addClass(cls);
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
		//set(x);
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
	std::ostream & nodeToStream(std::ostream & ostr, tag_display_mode mode=EMPTY_TAG) const;



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

	// drain::NodeHTML::HTML
	// typedef std::pair<key_t,xml_node_t> node_pair_t;
	// TODO: where is this needed?
	/*
	template <int E>
	static inline
	const std::pair<key_t,NodeXML<intval_t> > & entry(){
		static const intval_t elem = (intval_t)E;
		static const std::pair<key_t,NodeXML<intval_t> > nodeEntry(getTag(E), elem); // note: converts tag (string) to key_t if needed.
		return nodeEntry;
	}
	*/

	typedef std::map<xml_tag_t,xml_tag_t> xml_default_elem_map_t;
	static const xml_default_elem_map_t xml_default_elems;

protected:

	/// Internal function called after setType()
	/**
	 *
	 */
	virtual inline
	void handleType(const T & type){
		if ((int)type != 0){
			drain::Logger(__FILE__, __FUNCTION__).reject("handleType( ",  (int)type, " ) - this is available only as specialized, by inherited classed like SVG, HTML?");
		}
		// DANGER, without cast?
		// std::cerr << __FILE__ << ':' << __FUNCTION__ << " unimplemented? " << type << '=' << std::endl;
		// std::cerr << __FILE__ << ':' << __FUNCTION__ << " dict: " << drain::sprinter(drain::EnumDict<T>::dict) << std::endl;
	}


	/// NOTE: these could/should be templated, in TreeXML<...> right?
	//typedef std::map<std::string,std::string> xmldoc_attrib_map_t;
	typedef std::list<std::pair<std::string,std::string> > xmldoc_attrib_map_t;
	static xmldoc_attrib_map_t xmldoc_attribs;


};

/*
   Impossible. Cannot construct final object, for example members are not available for #link() .
template <>
inline
void NodeXML<int>::handle Type(const int & type){
 // DANGER, without cast?
	std::cerr << __FILE__ << ':' << __FUNCTION__ << " unimplemented? " << type << '=' << std::endl;
	std::cerr << __FILE__ << ':' << __FUNCTION__ << " dict: " << drain::sprinter(drain::EnumDict<int,XML>::dict) << std::endl;
}
*/

template <>
inline
const std::string & NodeXML<int>::getTag() const {
	return drain::EnumDict<int,XML>::dict.getKey(this->type, false);
}

typedef NodeXML<>::xml_tree_t TreeXML;
//typedef drain::UnorderedMultiTree<NodeXML<>,false, NodeXML<>::path_t> TreeXML;

// NOTE: template will not match for subclasses of NodeXML<E> because default template will match any class exactly.
template <class E, bool EX, class P>
struct TypeName< drain::UnorderedMultiTree<NodeXML<E>,EX,P> > {

    static const std::string & str(){
    	static const std::string name = drain::StringBuilder<>("TreeXML<", TypeName<E>::str(), ">");
    	return name;
    }

};

/*  This could be it, unless already defined...
template <class N, bool EX, class P>
inline
drain::UnorderedMultiTree<N,EX,P> & drain::UnorderedMultiTree<N,EX,P>::addChild(const drain::UnorderedMultiTree<N,EX,P>::key_t & key){
	return XML::xmlAddChild(*this, key);
}
*/

/* doesnt work
template <class E, bool EX, class P>
inline
drain::UnorderedMultiTree<NodeXML<E>,EX,P> & drain::UnorderedMultiTree<NodeXML<E>,EX,P>::addChild(const std::string & key){
	return XML::xmlAddChild(*this, key);
}
*/

/*
template <class N, bool EX, class P>
template <typename K> // for K (path elem arg)
const typename drain::UnorderedMultiTree<N,EX,P>::key_t & drain::UnorderedMultiTree<N,EX,P>::getKey(const K & key){
	return drain::EnumDict<K>::dict.getKey(key, false);
}
*/



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
	else {

		if (mode==CLOSING_TAG){
			ostr << "</";
		}
		else {
			ostr << '<';
		}

		if (getTag().empty()){
			drain::Logger(__FILE__, __FUNCTION__).unimplemented<LOG_ERR>("defaultTag for type=", getType(), " requested by for ID=", getId(), " attr=", getAttributes());

			ostr << "defaultTag"; // << ' ';  FIX! getDefaultTag?
		}
		else {
			ostr << getTag(); // << ' ';
			// TODO check GDAL XML
			// if (!defaultTag.empty())
			// attribToStream(ostr, "name", defaultTag);
		}
	}

	if (mode != CLOSING_TAG){

		if (typeIs(STYLE)){
			xmlAttribToStream(ostr, "data-mode", "experimental");
		}
		else { // if (!isCText())

			specificAttributesToStream(ostr);

			char sep=0;

			// Iterate attributes - note: also for comment
			for (const auto & key: getAttributes().getKeyList()){
				std::string v = get(key, "");
				// Skip empties (so Sprinter::toStream not applicable)
				if (!v.empty()){
					xmlAttribToStream(ostr, key, v);
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
		// warn if has children? or comment them??
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

template <>
const drain::EnumDict<int,XML>::dict_t drain::EnumDict<int,XML>::dict;


template <class E, bool EX, class P>
std::ostream & operator<<(std::ostream &ostr, const UnorderedMultiTree<NodeXML<E>,EX,P> & tree){
	// DOC def? TODO: preamble/prologToStream()
	NodeXML<E>::docTypeToStream(ostr); // must be member, to support virtual?
	NodeXML<E>::toStream(ostr, tree, "");
	return ostr;
}



/*
inline
std::ostream & operator<<(std::ostream &ostr, const TreeXML & t){
	// DOC def? TODO: preamble/prologToStream()
	TreeXML::node_data_t::docTypeToStream(ostr); // must be member, to support virtual?
	TreeXML::node_data_t::toStream(ostr, t, "");
	return ostr;
}
*/


}  // drain::

#endif /* TREEXML_H_ */

