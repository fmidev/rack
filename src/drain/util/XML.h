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



#ifndef DRAIN_XML
#define DRAIN_XML

#include <ostream>

#include <drain/Sprinter.h>
#include <drain/FlexibleVariable.h>

#include "ClassXML.h"
// #include "Flags.h"
#include "ReferenceMap.h"

namespace drain {


/**
 *   Flexibility is used (at least) in:
 *   - linking box.height to font-size (in TEXT or TSPAN elems)
 */
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

// ------To be separated -------------------

class UtilsXML {

public:

	/// Tree path type. // TODO: extract from template
	typedef drain::Path<std::string,'/'> path_t; // consider xml_path_t
	typedef path_t::elem_t path_elem_t;

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
	template <class V, class T>
	static
	bool findByTag(const V & tree, const T & tag, path_list_t & result, const path_t & path = path_t());

	/// "Forward definition"
	//   This could also be in TreeXMLutilities
	/**
	 *   \tparam V - XML tree
	 *
	 */
	template <class V, class T>
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

	/// Finds elements recursively in an XML structure by class name supplied as an enumeration type.
	/**
	 *  \tparam V - XML tree
	 *  \tparam C - enum type, for which a unique (static) EnumDict has been detected.
	 *	\param path - starting point
	 */
	//template <class V, class E>
	template <class V, class C>
	static inline
	bool findByClass(const V & t, const C & cls, path_list_t & result, const path_t & path = path_t());




};

// -------------------------

class XML : public UtilsXML, protected ReferenceMap2<FlexibleVariable> {
public:

	typedef int intval_t;
	intval_t type = XML::UNDEFINED;



	static const int UNDEFINED = 0;
	static const int COMMENT = 1;
	static const int CTEXT = 2;
	static const int STYLE = 3;
	static const int STYLE_SELECT = 4;
	static const int SCRIPT = 5;

	// intval_t type = UNDEFINED;


	typedef ReferenceMap2<FlexibleVariable> map_t;

	/// Some general-purpose variables

	// String, still easily allowing numbers through set("id", ...)
	std::string id;

	// Consider either/or
	std::string ctext;

	std::string url;

	template <class V>
	static inline
	void attribToStream(std::ostream &ostr, const std::string & key, const V &value){
		ostr << ' ' << key << '=' << '"' << value << '"'; // << ' ';
	}

	static int nextID;


	inline
	static int getCount(){
		return nextID;
	}

	/// Makes ID a visible attribute.
	/**
	 *  Also a terminal function for
	 */
	inline
	void setId(){
		link("id", id);
	}

	/// Makes ID a visible attribute, with a given value.
	inline
	void setId(const std::string & s){
		link("id", id = s);
	}

	template <char C='\0', typename ...TT>
	inline
	void setId(const TT & ...args) {
		link("id", id = drain::StringBuilder<C>(args...));
	}

	/// Returns ID of this element. Hopefully a unique ID...
	inline
	const std::string & getId() const {
		return id;
	}

	inline
	bool empty() const {
		return map_t::empty();
	}

	inline
	const map_t & getAttributes() const {
		return *this;
	};

	// Maybe controversial. Helps importing sets of variables.
	inline
	map_t & getAttributes(){
		return *this;
	};


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

	// ------------------ Style Class ---------------


	ClassListXML classList;


	StyleXML style;


};




template <class T>
bool UtilsXML::findById(const T & t, const std::string & id, typename T::path_t & result, const typename T::path_t & path){

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



template <class T>
bool UtilsXML::findById(const T & t, const std::string & id, UtilsXML::path_list_t & result, const path_t & path){

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
//template <class N>
template <class T, class N>
bool UtilsXML::findByTag(const T & t, const N & tag, UtilsXML::path_list_t & result, const path_t & path){

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
//template <class N>
template <class T,class N>
bool UtilsXML::findByTags(const T & t, const std::set<N> & tags, UtilsXML::path_list_t & result, const UtilsXML::path_t & path){

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


//template <class N>
// template <class T>
//bool NodeXML<N>::findByClass(const T & t, const std::string & cls, NodeXML<>::path_list_t & result, const path_t & path){

template <class T2, class C> // NodeXML<N>
//bool XML::findByClass(const T2 & t, const C & cls, NodeXML<>::path_list_t & result, const path_t & path){
bool UtilsXML::findByClass(const T2 & t, const C & cls, UtilsXML::path_list_t & result, const UtilsXML::path_t & path){

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

/// Immediate descendants
template <class T2, class C> //  NodeXML<N>
bool UtilsXML::findByClass(const T2 & t, const C & cls, std::list<UtilsXML::path_elem_t> & result){

	for (const auto & entry: t){
		if (entry.second->hasClass(cls)){
			result.push_back(entry.first);
		}
	}

	return !result.empty();
}




}  // drain::

#endif /* DRAIN_XML */

