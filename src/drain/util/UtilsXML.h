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
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */



#ifndef DRAIN_UTILS_XML
#define DRAIN_UTILS_XML

#include <ostream>

#include <drain/Sprinter.h>
// #include <drain/FlexibleVariable.h>

// #include "ClassXML.h"

namespace drain {


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
	template <class T>
	static
	bool findById(const T & tree, const std::string & tag, path_list_t & result, const path_t & path = path_t());

	/// Find all the occurrences of given tag type using recursive breath-first search.
	/// This is a "forward definition" – this could also be in TreeXMLutilities.
	template <class T, class E>
	static
	bool findByTag(const T & tree, const E & tag, path_list_t & result, const path_t & path = path_t());

	/// "Forward definition"
	//   This could also be in TreeXMLutilities
	/**
	 *   \tparam V - XML tree
	 *
	 */
	template <class T, class E>
	static
	bool findByTags(const T & tree, const std::set<E> & tags, path_list_t & result, const path_t & path = path_t());

	/// Finds child elements in an XML structure by class name.
	/**
	 *   \tparam V - XML tree
	 *
	 *	In a way, this is a forward definition – this could also be in TreeXMLutilities.
	 *
	 */
	template <class T, class C>
	static
	bool findByClass(const T & t, const C & cls, std::list<typename T::path_elem_t> & result);

	/// Finds elements recursively in an XML structure by class name supplied as an enumeration type.
	/**
	 *  \tparam V - XML tree
	 *  \tparam C - enum type, for which a unique (static) EnumDict has been detected.
	 *	\param path - starting point
	 */
	//template <class V, class E>
	template <class T, class C>
	static inline
	bool findByClass(const T & t, const C & cls, path_list_t & result, const path_t & path = path_t());




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

	return !result.empty();

}

/**
 *  \tparam Tree
 */
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



template <class T, class C>
bool UtilsXML::findByClass(const T & t, const C & cls, UtilsXML::path_list_t & result, const UtilsXML::path_t & path){

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
template <class T, class C>
bool UtilsXML::findByClass(const T & t, const C & cls, std::list<typename T::path_elem_t> & result){

	for (const auto & entry: t){
		if (entry.second->hasClass(cls)){
			result.push_back(entry.first);
		}
	}

	return !result.empty();
}


}  // drain::

#endif /* DRAIN_UTILS_XML */
