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



#ifndef TREE_XML
#define TREE_XML

#include <ostream>

#include <drain/Sprinter.h>
#include <drain/FlexibleVariable.h>

#include "ClassXML.h"
#include "Flags.h"
#include "ReferenceMap.h"
#include "TreeUnordered.h"

namespace drain {


class StyleXML : public ReferenceMap2<FlexibleVariable> {

public:

	inline
	StyleXML(){};

	static const SprinterLayout styleLineLayout;
	static const SprinterLayout styleRecordLayout;
	static const SprinterLayout styleRecordLayoutActual;

};


inline
std::ostream & operator<<(std::ostream &ostr, const StyleXML & style){
	drain::Sprinter::toStream(ostr, style.getMap(), drain::Sprinter::xmlAttributeLayout);
	return ostr;
}




/**
 *  \tparam T - index type; may be enum.
 */
template <class T=int>
class NodeXML : protected ReferenceMap2<FlexibleVariable> {


public:

	static const int UNDEFINED;
	static const int COMMENT;
	static const int CTEXT;
	static const int STYLE;
	static const int SCRIPT;

	/// Tag type, CTEXT or COMMENT.
	typedef T elem_t;  // consider xml_elem_t
	elem_t type;

	typedef NodeXML<T> xml_node_t;

	/// Tree path type.
	typedef drain::Path<std::string,'/'> path_t; // consider xml_path_t
	typedef path_t::elem_t path_elem_t;

	typedef UnorderedMultiTree<NodeXML<T>,false, path_t> xml_tree_t;

	typedef ReferenceMap2<FlexibleVariable> map_t;

	inline
	NodeXML(const elem_t & t = elem_t(0)){
		type = t;
		drain::StringTools::import(++nextID, id);
		id = getTag()+id;
	};

	// Note: use default constructor in derived classes.
	inline
	NodeXML(const NodeXML & node){
		//drain::StringTools::import(++nextID, id);
		copyStruct(node, node, *this, RESERVE); // This may corrupt (yet unconstructed) object?
		type = node.getType();
		drain::StringTools::import(++nextID, id);
		id = getTag()+id;
	}


	inline
	~NodeXML(){};


	typedef std::map<T,std::string> tag_map_t;

	static
	tag_map_t tags;


	/// Some general-purpose

	// Consider either/or
	std::string ctext;

	std::string url;

	// drain::Rectangle<double> frame;

	inline
	bool empty() const {
		return map_t::empty();
	}

	virtual
	void setType(const elem_t &t){
		type = t;
		// in derived classes, eg. drain::image::BaseGDAL
		// warning: case value ‘...’ not in enumerated type
		/*
		switch (type){
		case STYLE:
			link("href", url);
			break;
		case SCRIPT:
			link("href", url);
			break;
		default:
			// Note: not all values need handling, and others are handled in subclass setType()
			break;
		};
		*/
	}

	/* Consider this later, for user-defined (not enumerated) tag types.
	virtual
	void setType(const std::string & type);
	*/

	inline
	const elem_t & getType() const {
		return type;
	};

	inline
	bool typeIs(const elem_t &t) const {
		return type == t;
	};

	/*
	template <elem_t E>
	inline
	bool typeIs() const {
		return type == E;
	};*/


	inline
	bool isComment() const {
		return typeIs((elem_t)COMMENT);
	}

	inline
	bool isCText() const {
		return typeIs((elem_t)CTEXT);
	}

	inline
	bool isUndefined() const {
		return typeIs((elem_t)UNDEFINED);
		//return ((int)getType() == UNDEFINED);
	}

// protected:

	// virtual
	bool isSelfClosing() const {

		static
		const std::set<elem_t> l = {(elem_t)SCRIPT};
		return (l.find(this->getType()) == l.end()); // not in the set

		//return false;
	}

	// template <int N>
	static inline
	const std::string & getTag(unsigned int index){
		typename tag_map_t::const_iterator it = tags.find((elem_t)index);
		if (it != tags.end()){
			return it->second;
		}
		else {
			// consider empty
			static std::string dummy;
			dummy = drain::StringBuilder<>("UNKNOWN-", index);
			return dummy;
			// throw std::runtime_error(drain::StringBuilder<':'>(__FILE__, __FUNCTION__, " tag [", static_cast<int>(index), "] missing from dictionary"));
		}
	};



	inline
	const std::string & getTag() const {
		return getTag(type); // risky? converts enum to int .. and back.
	};


	inline
	const map_t & getAttributes() const {
		return *this;
	};

	// Maybe controversial. Helps importing sets of variables.
	inline
	map_t & getAttributes(){
		return *this;
	};


	// NEW: --------------
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
	void set(const elem_t & type){
		setType(type);
	}

	inline
	void set(const std::string & s){
		if (type == STYLE){
			SmartMapTools::setValues(style, StringTools::trim(s, "; \t\n"), ';', ':');
			//drain::SmartMapTools::setValues<map_t,true>(getAttributes(), s);
		}
		else {
			setText(s);
		}
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

	template <class X>
	inline
	void set(const std::map<std::string, X> & args){
		// TODO: redirect to set(key,value), for consistency
		if (type == STYLE){
			drain::SmartMapTools::setValues(style, args);
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
			setStyle(key, value);
		}
		else if (key == "style"){
			// Modify collection
			setStyle(value);
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

	/// Default implementation. Needed for handling units in strings, like "50%" or "640px".
	//   But otherways confusing?
	virtual inline
	void setAttribute(const std::string & key, const std::string &value){
		(*this)[key] = value;
	}

	/// Default implementation. Needed for handling units in strings, like "50%" or "640px".
	//   But otherways confusing?
	virtual inline
	void setAttribute(const std::string & key, const char *value){
		(*this)[key] = value; // -> handleString()
	}

	/// "Final" implementation.
	template <class V>
	inline
	void setAttribute(const std::string & key, const V & value){
		(*this)[key] = value; // -> handleString()
	}


	inline
	void remove(const std::string & s){
		iterator it = this->find(s);
		if (it != this->end()){
			this->erase(it);
		}
	}




	inline
	const drain::FlexibleVariable & get(const std::string & key) const {
		return (*this)[key];
	}

	inline
	drain::FlexibleVariable & get(const std::string & key){
		return (*this)[key];
	}


	template <class V>
	inline
	V get(const std::string & key, const V & defaultValue) const {
		return map_t::get(key, defaultValue);
	}

	inline
	std::string get(const std::string & key, const char * defaultValue) const {
		return map_t::get(key, defaultValue);
	}


	/// Style class

	template <typename ... TT>
	inline
	void addClass(const TT &... args) {
		classList.add(args...);
	}

	/*
	template <typename ... TT>
	inline
	void addClass(const std::string & s, const TT &... args) {
		classList.insert(s);
		addClass(args...);
	}
	*/

	/**
	 *  \tparam V – string or enum type
	 */
	template <class V>
	inline
	bool hasClass(const V & cls) const {
		return classList.has(cls);
	}

	inline
	void removeClass(const std::string & s) {
		classList.remove(s);
	}

	inline
	void clearClasses(){
		classList.clear();
	}


	// Check char *
	inline
	NodeXML & operator=(const T & x){
		set(x);
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

	StyleXML style;

	/// Make this node a comment. Contained tree will not be delete. In current version, attributes will be rendered.
	/**
	 *   \param text - if given, replaces current CTEXT.
	 *
	 */
	inline
	NodeXML & setComment(const std::string & text = "") {

		setType((elem_t)COMMENT);

		if ((int)getType() != COMMENT){ //cross-check
			throw std::runtime_error(drain::TypeName<NodeXML<T> >::str() + ": COMMENT not supported");
		}

		ctext = text;
		return *this;
	}

	/// Assign the text content of this node. If the node type is undefined, set it to CTEXT.
	/**
	 *   \param text - assigned to text content.
	 */
	template <class S>
	inline
	// 	NodeXML & setText(const std::string & text = "") {
	NodeXML & setText(const S & value) {
		if (isUndefined()){
			setType((elem_t)CTEXT);
			if (!isCText()){ //cross-check
				throw std::runtime_error(drain::TypeName<NodeXML<T> >::str() + ": CTEXT not supported");
			}
		}
		drain::StringTools::import(value, ctext);
		return *this;
	}

	inline
	NodeXML & setText(const char *value) {
		setText(std::string(value));
		return *this;
	}




	template <class S>
	inline
	void setStyle(const S &value){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.error("unsupported type ", drain::TypeName<S>::str(), " for value: ", value);
	}

	void setStyle(const std::string & value){
		drain::Logger mout(__FILE__, __FUNCTION__);
		if (isUndefined()){
			mout.suspicious("setting style for undefined elem: ", *this);
		}
		else if (type == STYLE){
			mout.warn("setting style for STYLE elem: ", *this);
		}
		SmartMapTools::setValues(style, StringTools::trim(value, "; \t\n"), ';', ':');
	}

	void setStyle(const char *value){
		setStyle(std::string(value));
	}

	inline
	void setStyle(const std::string & key, const std::string & value){
		this->style[key] = value;
	}


	template <class V>
	inline
	void setStyle(const std::string & key, const V & value){
		if (type == STYLE){
			// "reuse" style map as style record map
			std::stringstream sstr;
			drain::Sprinter::toStream(sstr, value, StyleXML::styleLineLayout);
			this->style[key] = value; //sstr.str();
		}
		else {
			this->style[key] = value;
		}
	}

	/* ?????????
	inline
	void setStyle(const std::string & key, const std::initializer_list<std::pair<const char *,const drain::Variable> > &l){
		drain::SmartMapTools::setValues(style, l);
	}
	*/

	inline
	void setStyle(const std::initializer_list<std::pair<const char *,const drain::Variable> > &l){
		drain::SmartMapTools::setValues(style, l);
	}

	/*
	inline
	void setStyle(const std::initializer_list<std::pair<const char *,const drain::Variable> > &l){

	}
	*/


	// typedef std::set<std::string> class_list;
	// class_list classList;

	ClassListXML classList;

	typedef std::list<path_t> path_list_t;

	/// Find the first occurrence of given id using recursive breath-first search.
	/**
	 *   By definition, id attributes should be unique.
	 *
	 *   \param tree - source element,
	 *   \param id   - id attribute to be searched for
	 *   \param result - path of the element with required ID, if found.
	 *   \param path - start path for the search.
	 *   \return - \c true, if an element was found.
	 *
	 *
	 */
	//   This could also be in TreeXMLutilities
	template <class V>
	static
	bool findById(const V & tree, const std::string & tag, typename V::path_t & result, const typename V::path_t & path = path_t());

	/// Find the occurrence(s) of given ID using recursive breath-first search.
	/**
	 *   By definition, id attributes should be unique. This function nevertheless returns a list
	 *   if user wants to handle more than one elements found.
	 *
	 *   \param tree - source element,
	 *   \param id   - id attribute to be searched for
	 *   \param result - path of pointing to
	 *   \param path - start path for the search.
	 *   \return - \c true, if an element was found.
	 */
	//   This could also be in TreeXMLutilities
	template <class V>
	static
	bool findById(const V & tree, const std::string & tag, path_list_t & result, const path_t & path = path_t());

	/// Find all the occurrences of given tag type using recursive breath-first search.
	/// This is a "forward definition" – this could also be in TreeXMLutilities.
	template <class V>
	static
	bool findByTag(const V & tree, const T & tag, path_list_t & result, const path_t & path = path_t());

	/// "Forward definition"
	//   This could also be in TreeXMLutilities
	/**
	 *   \tparam V - XML tree
	 *
	 */
	template <class V>
	static
	bool findByTags(const V & tree, const std::set<T> & tags, path_list_t & result, const path_t & path = path_t());

	/// Finds child elements in an XML structure by class name.
	/**
	 *   \tparam V - XML tree
	 *
	 *	In a way, this is a forward definition – this could also be in TreeXMLutilities.
	 *
	 */
	template <class T2, class C>
	static
	bool findByClass(const T2 & t, const C & cls, std::list<path_elem_t> & result);

	/// Finds elements in an XML structure by class name supplied as an enumeration type.
	/**
	 *   \tparam V - XML tree
	 *   \tparam C - enum type, for which a unique (static) EnumDict has been detected.
	 *
	 */
	//template <class V, class E>
	template <class V, class C>
	static inline
	bool findByClass(const V & t, const C & cls, path_list_t & result, const path_t & path = path_t());


	/// Finds elements in an XML structure by class name.
	/**
	 *   \tparam V - XML tree
	 *
	 *	In a way, this is a forward definition – this could also be in TreeXMLutilities.
	 *
	template <class V>
	static
	bool findByClass(const V & t, const std::string & cls, path_list_t & result, const path_t & path = path_t());
	 */


	/// Finds elements in an XML structure by class name. Redirects to findByClass(t, std::string(cls),
	/**
	 *   \tparam V - XML tree
	 *
	template <class V>
	static inline
	bool findByClass(const V & t, const char *cls, path_list_t & result, const path_t & path = path_t()){
		return findByClass(t, std::string(cls), result, path);
	}
	 */

	/*
	{
		return findByClass(t, drain::EnumDict<E>::dict.getKey(cls), result, path); // TODO: this only, with -> hasClass(cls) ?
	}
	*/



	template <class V>
	static inline
	std::ostream & docToStream(std::ostream & ostr, const V & tree){
		V::node_data_t::xml_node_t::docTypeToStream(ostr);
		V::node_data_t::xml_node_t::toStream(ostr, tree);
		return ostr;
	}

	/// "Forward definition" of Tree::toOstream
	template <class V>
	static
	std::ostream & toStream(std::ostream &ostr, const V & t, const std::string & defaultTag = "", int indent=0);

	/// Write the XML definition beginning any XML document.
	/**
	 *  Each derived class should have a definition compatible with the structure generated by the class.
	 *
	 */
	static // virtual
	inline
	std::ostream & docTypeToStream(std::ostream &ostr){
		ostr << "<?xml ";
		for (const auto & entry: xmldoc_attribs){
			attribToStream(ostr, entry.first, entry.second);
		}
		ostr << "?>";
		ostr << '\n';
		return ostr;
	}

	//drain::NodeHTML::HTML
	// typedef std::pair<key_t,xml_node_t> node_pair_t;
	template <int E>
	static inline
	const std::pair<key_t,NodeXML<T> > & entry(){
		static const elem_t elem = (elem_t)E;
		static const std::pair<key_t,NodeXML<T> > nodeEntry(getTag(E), elem); // note: converts tag (string) to key_t if needed.
		return nodeEntry;
	}

	inline
	static int getCount(){
		return nextID;
	}


	/// Makes ID a visible attribute.
	inline
	void setId(){
		link("id", id);
	}

	/// Makes ID a visible attribute, with a given value.
	inline
	void setId(const std::string & s){
		link("id", id = s);
	}


	/// Returns ID of this element. Hopefully a unique ID...
	inline
	const std::string & getId() const {
		return id;
	}


protected:

	typedef std::map<std::string,std::string> xmldoc_attrib_map_t;

	static xmldoc_attrib_map_t xmldoc_attribs;

	//inline	void addClass(){}

	template <class V>
	static inline
	void attribToStream(std::ostream &ostr, const std::string & key, const V &value){
		ostr << ' ' << key << '=' << '"' << value << '"'; // << ' ';
	}

	static int nextID;

	// String, still easily allowing numbers through set("id", ...)
	std::string id;

	// TODO: consider TAG from dict?
	// std::string tag;


};

template <class T>
const int NodeXML<T>::UNDEFINED(0);

template <class T>
const int NodeXML<T>::COMMENT(1);

template <class T>
const int NodeXML<T>::CTEXT(2);

template <class T>
const int NodeXML<T>::STYLE(3);

template <class T>
const int NodeXML<T>::SCRIPT(4);


typedef drain::UnorderedMultiTree<NodeXML<>,false, NodeXML<>::path_t> TreeXML;

// NOTE: template will not match for subclasses of NodeXML<E> because default template will match any class exactly.
template <class E, bool EX, class P>
struct TypeName< drain::UnorderedMultiTree<NodeXML<E>,EX,P> > {

    static const std::string & str(){
    	static const std::string name = drain::StringBuilder<>("TreeXML<", TypeName<E>::str(), ">");
    	return name;
    }

    static const char* get(){
    	return str().c_str();
    }
};


// Experimental.
template <>
TreeXML & TreeXML::addChild(const TreeXML::key_t & key);

template <class N>
int NodeXML<N>::nextID = 0;


template <class N>
template <class T>
bool NodeXML<N>::findById(const T & t, const std::string & id, typename T::path_t & result, const typename T::path_t & path){

	if (t->id == id){
		result = path;
		return true;
	}

	// Recursion
	for (const auto & entry: t){
		if (findById(entry.second, id, result, path_t(path, entry.first))){
			return true;
		}
	}

	return false;
	//return !result.empty();
}



/*
template <class V>
static
bool typename V::node_t::findById(const V & t, const std::string & tag, typename V::path_t & result, const typename V::path_t & path = typename V::path_t()){

	return false;
}
*/
/*
template <class T>
bool typename T::node_t::findById(const typename T & t, const std::string & id, typename T::path_t & result, const typename T::path_t & path){

	if (t->id == id){
		result = path;
		return true;
	}

	for (const auto & entry: t){
		return findByClass(entry.second, id, result, path_t(path, entry.first));
	}

	return false;
	//return !result.empty();
}
*/

template <class N>
template <class T>
bool NodeXML<N>::findById(const T & t, const std::string & id, NodeXML<>::path_list_t & result, const path_t & path){

	if (t->id == id){
		result.push_back(path);
	}

	for (const auto & entry: t){
		findById(entry.second, id, result, path_t(path, entry.first));
	}

	return !result.empty();
}

/**
 *  \tparam Tree
 */
template <class N>
template <class T>
bool NodeXML<N>::findByTag(const T & t, const N & tag, NodeXML<>::path_list_t & result, const path_t & path){

	// const T & t = tree(path);

	if (t->typeIs(tag)){
		result.push_back(path);
	}

	for (const auto & entry: t){
		findByTag(entry.second, tag, result, path_t(path, entry.first));
	}

	//return result;
	return !result.empty();
}

/**
 *  \tparam Tree
 */
template <class N>
template <class T>
bool NodeXML<N>::findByTags(const T & t, const std::set<N> & tags, NodeXML<>::path_list_t & result, const path_t & path){

	// const T & t = tree(path);

	//if (t->typeIs(tag)){
	if (tags.count(t->getType()) > 0){
		result.push_back(path);
	}

	for (const auto & entry: t){
		findByTags(entry.second, tags, result, path_t(path, entry.first));
	}

	//return result;
	return !result.empty();
}


template <class N>
// template <class T>
//bool NodeXML<N>::findByClass(const T & t, const std::string & cls, NodeXML<>::path_list_t & result, const path_t & path){

template <class T2, class C>
bool NodeXML<N>::findByClass(const T2 & t, const C & cls, NodeXML<>::path_list_t & result, const path_t & path){

	// drain::Logger mout(__FILE__,__FUNCTION__);

	if (t->classList.has(cls)){
		result.push_back(path);
	}

	for (const auto & entry: t){
		// mout.warn(t->get("name", "<name>"), "... continuing to: ", path_t(path, entry.first));
		findByClass(entry.second, cls, result, path_t(path, entry.first));
	}

	return !result.empty();
}

template <class N>
template <class T2, class C>
bool NodeXML<N>::findByClass(const T2 & t, const C & cls, std::list<path_elem_t> & result){

	for (const auto & entry: t){
		if (entry.second->hasClass(cls)){
			result.push_back(entry.first);
		}
	}

	return !result.empty();
}



/// Note: Not designed for XML output, this is more for debugging (in tree dumps),
/**
 *
 */
template <class N>
inline
std::ostream & operator<<(std::ostream &ostr, const NodeXML<N> & node){

	//ostr << node.getTag() << '<' << (unsigned int)node.getType() << '>' << ' ';
	ostr  << '<' << node.getTag() << '>' << ' ';

	// drain::Sprinter::toStream(ostr, node.getAttributes(), drain::Sprinter::jsonLayout);
	// drain::Sprinter::toStream(ostr, node.getAttributes().getMap(), drain::Sprinter::jsonLayout);
	//
	if (!node.getAttributes().empty()){
		drain::Sprinter::toStream(ostr, node.getAttributes().getMap(), drain::Sprinter::xmlAttributeLayout);
		ostr << ' ';
	}
	if (!node.classList.empty()){
		//ostr << '['; // has already braces []
		//drain::Sprinter::toStream(ostr, node.classList, drain::Sprinter::pythonLayout);
		drain::Sprinter::toStream(ostr, node.classList, ClassListXML::layout);
		//ostr << ']' << ' ';
		ostr << ' ';
	}
	if (!node.style.empty()){
		ostr << '{';
		drain::Sprinter::toStream(ostr, node.style);
		ostr << '}' << ' ';
	}
	if (!node.ctext.empty()){
		ostr << "'";
		if (node.ctext.length() > 20){
			ostr << node.ctext.substr(0, 15);
		}
		else {
			ostr << node.ctext;
		}
		ostr << "'";
	}
	return ostr;
}

/// XML output function shared for all XML documents, ie versions defined in subclasses.
/**
 *  TODO: preamble/prologToStream()
 *
 *   \param defaultTag - important for
 *
 */
template <class N>
template <class T>
std::ostream & NodeXML<N>::toStream(std::ostream & ostr, const T & tree, const std::string & defaultTag, int indent){


	const typename T::container_t & children = tree.getChildren();

	// Indent
	//std::fill_n(std::ostream_iterator<char>(ostr), 2*indent, ' ');
	std::string fill(2*indent, ' ');
	ostr << fill;

	// Start dag
	if (tree->isComment()){
		ostr << "<!-- " << tree->getTag() << ' ' << tree->ctext; // << " /-->\n";
	}
	else if (tree->isCText()){
		ostr << tree->ctext; // << " /-->\n";
	}
	else if (tree->getTag().empty())
		ostr << '<' << defaultTag; // << ' ';
	else {
		ostr << '<' << tree->getTag(); // << ' ';
		// TODO check GDAL XML
		//if (!defaultTag.empty())
		//	attribToStream(ostr, "name", defaultTag);
	}

	if (tree->typeIs((elem_t)STYLE)){
		//ostr << ' ';
		attribToStream(ostr, "data-mode", "experimental");
		// Sprinter::sequenceToStream(ostr, tree->style, StyleXML::styleRecordLayout);
		// ostr << "\n /-->";
	}
	else if (!tree->isCText()){
		//char sep=' ';
		if (!tree->classList.empty()){
			ostr << " class=\"";
			drain::Sprinter::toStream(ostr, tree->classList, ClassListXML::layout);
			// std::copy(tree->classList.begin(), tree->classList.end(), std::ostream_iterator<std::string>(ostr, " "));
			ostr << '"'; //ostr << "\"";
		}

		// Iterate attributes - note: also for comment
		// Checking empties, so Sprinter::toStream not applicable
		for (const typename T::node_data_t::key_t & key: tree.data.getKeyList()){
			if (!tree.data[key].empty()){
				std::stringstream sstr;
				sstr << tree.data[key];  // consider checking 0, not only empty string "".
				if (!sstr.str().empty()){
					attribToStream(ostr, key, sstr.str());
				}
			}
			//ostr << ' ';
		}

		// TAG style
		if (!tree->style.empty()){
			ostr << " style=\"";
			Sprinter::sequenceToStream(ostr, tree->style, StyleXML::styleLineLayout);
			//Sprinter::mapToStream(ostr, tree->style, StyleXML::styleLineLayout);
			ostr << '"'; // << ' ';
		}


	}
	else {
		if (!tree.data.empty()){
			// ??
		}
	}

	// END TAG
	if (tree->isComment()){
		ostr << " /-->\n";
	}
	else if (tree->isCText()){
		ostr << '\n';
	}
	else if (tree.data.isSelfClosing() &&
			(!tree->typeIs((elem_t)STYLE)) && (!tree->typeIs((elem_t)SCRIPT)) &&
			(children.empty()) && tree->ctext.empty() ){ // OR no ctext!
		// close TAG
		ostr << "/>\n";
		//ostr << '/' << '>';
		//ostr << '\n';
	}
	else {
		// close starting TAG ...
		ostr << '>';

		// ... and write contents

		/*
		if (!tree->style.empty()){
			ostr << "<!-- STYLE? ";
			drain::Sprinter::toStream(ostr, tree->style.getMap(), drain::Sprinter::xmlAttributeLayout);
			ostr << "/-->\n";
		}
		*/

		if (tree->typeIs((elem_t)STYLE)){
			// https://www.w3.org/TR/xml/#sec-cdata-sect
			// ostr << "<![CDATA[ \n";
			if (!tree->ctext.empty()){
				// TODO: indent
				ostr << fill << tree->ctext << '\n';
			}
			if (!tree->getAttributes().empty()){
				ostr << "\n\t /* <!-- DISCARDED attribs ";
				drain::Logger mout(__FILE__,__FUNCTION__);
				mout.warn("STYLE elem contains attributes, probably meant as style: ", tree.data);
				Sprinter::toStream(ostr, tree->getAttributes()); //, StyleXML::styleRecordLayout
				ostr << " /--> */" << '\n';
			}
			if (!tree->style.empty()){
				// ostr << "\n\t<!-- attribs /-->" << '\n';
				//ostr << fill << "<!-- style obj /-->" << '\n';
				ostr << fill << "/** style obj **/" << '\n';
				//ostr << fill;
				//Sprinter::sequenceToStream(ostr, tree->style, StyleXML::styleRecordLayout);
				for (const auto & attr: tree->style){
					ostr << fill << "  ";
					Sprinter::pairToStream(ostr, attr, StyleXML::styleRecordLayout); // {" :;"}
					//attr.first << ':' attr.first << ':';
					ostr << '\n';
				}
				// ostr << fill << "}\n";
				//Sprinter::sequenceToStream(ostr, entry.second->getAttributes(), StyleXML::styleRecordLayoutActual);
				// ostr << '\n';
			}
			ostr << '\n';
			// ostr << fill << "<!-- elems /-->" << '\n';
			ostr << fill << "/* elems */" << '\n';
			for (const auto & entry: tree.getChildren()){
				if (!entry.second->getAttributes().empty()){
					//ostr << fill << "<!-- elem("<< entry.first << ") attribs /-->" << '\n';
					ostr << fill << "  " << entry.first << " {\n";
					for (const auto & attr: entry.second->getAttributes()){
						ostr << fill  << "    ";
						ostr << attr.first << ':' << attr.second << ';';
						//Sprinter::pairToStream(ostr, attr, StyleXML::styleLineLayout); // {" :;"}
						// Sprinter::pairToStream(ostr, attr, StyleXML::styleRecordLayout); // {" :;"}
						// attr.first << ':' attr.first << ':';
						ostr << '\n';
					}
					ostr << fill << "  }\n";
					//Sprinter::sequenceToStream(ostr, entry.second->getAttributes(), StyleXML::styleRecordLayoutActual);
					ostr << '\n';
				}
				if (!entry.second->ctext.empty()){
					//ostr << fill << "<!-- elem("<< entry.first << ") ctext /-->" << '\n';
					ostr << fill << "  " << entry.first << " {" << entry.second->ctext << "}\n";
				}
				// Sprinter::sequenceToStream(ostr, entry.second->style, StyleXML::styleRecordLayout);
			}
			ostr << "\n"; // end CTEXT
			//ostr << " ]]>\n"; // end CTEXT
			// end STYLE defs
		}
		else {

			if (tree->ctext.empty())
				ostr << '\n'; // TODO nextline
			else
				ostr << tree->ctext;

			/// iterate children - note the use of default tag
			for (const auto & entry: children){
				toStream(ostr, entry.second, entry.first, indent+1); // no ++
				//ostr << *it;
			}
			// add end </TAG>

		}

		if (tree->typeIs((elem_t)STYLE) || !children.empty()){
			ostr << fill;
			//std::fill_n(std::ostream_iterator<char>(ostr), 2*indent, ' ');
		}

		ostr << '<' << '/' << tree->getTag() << '>';
		ostr << '\n';  // TODO nextline

		//if (tree.data.id >= 0)
		//	ostr << "<!-- " << tree.data.id << " /-->\n";
	}
	return ostr;
}




inline
std::ostream & operator<<(std::ostream &ostr, const TreeXML & t){
	// DOC def? TODO: preamble/prologToStream()
	TreeXML::node_data_t::docTypeToStream(ostr); // must be member, to support virtual?
	TreeXML::node_data_t::toStream(ostr, t, "");
	return ostr;
}


}  // drain::

#endif /* TREEXML_H_ */

