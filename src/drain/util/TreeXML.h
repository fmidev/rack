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
#include "ReferenceMap.h"
//#include "VariableMap.h"
//#include "FlexibleVariable.h"
//#include "SmartMap.h"
#include "TreeUnordered.h"
//#include "FlexibleVariable.h"

namespace drain {


class StyleXML : public ReferenceMap2<FlexibleVariable> {

public:

	inline
	StyleXML(){};

};

/**
 *  \tparam T - index type; may be enum.
 */
template <class T=int>
class NodeXML : protected ReferenceMap2<FlexibleVariable> {


public:


	/// Tag type, CTEXT or COMMENT.
	typedef T elem_t;
	elem_t type;


	static const int UNDEFINED;
	static const int COMMENT;
	static const int CTEXT;

	/// Tree path type.
	typedef drain::Path<std::string,'/'> path_t;

	//typedef ReferenceMap map_t;
	typedef ReferenceMap2<FlexibleVariable> map_t;

	inline
	NodeXML(const elem_t & t = elem_t(0)) : id(++nextID){
		type = t;
		// link("id", id);
	};

	typedef std::map<T,std::string> tag_map_t;

	static
	tag_map_t tags;


	NodeXML(const NodeXML & node) : id(++NodeXML::nextID) {
		copyStruct(node, node, *this, RESERVE); // This may corrupt (yet unconstructed) object?
	}

	inline
	~NodeXML(){};


	// Consider either/or
	std::string ctext;

	virtual
	void setType(const elem_t &t){
		type = t;
	}

	inline
	const elem_t & getType() const {
		return type;
	};

	inline
	bool typeIs(const elem_t &t) const {
		return type == t;
	};

	/*
	inline
	const std::string & getTag2() const {
		return tag;
	};
	*/

	inline
	const std::string & getTag() const {
		typename tag_map_t::const_iterator it = tags.find(type);
		if (it != tags.end()){
			return it->second;
		}
		else {
			throw std::runtime_error(drain::StringBuilder<>(__FILE__, __FUNCTION__, ": unknown TAG enum value: ", static_cast<int>(type)));
		}
		// return tags[type];
	};


	inline
	const map_t & getMap() const {
		return *this;
	};

	// Maybe controversial. Helps importing sets of variables.
	inline
	map_t & getMap(){
		return *this;
	};


	template <class V>
	inline
	void set(const std::string & key, const V & value){
		(*this)[key] = value;
	}

	inline
	void set(const std::initializer_list<std::pair<const char *, const drain::Variable> > &l){
		drain::SmartMapTools::setValues<map_t,true>((map_t &)*this, l);       // add new keys
		// drain::SmartMapTools::setValues<map_t,false>((map_t &)*this, l);   // update only
	}

	inline
	std::string get(const std::string & key, const char * defaultValue) const {
		return map_t::get(key, defaultValue);
	}

	template <class V>
	inline
	V get(const std::string & key, const V & defaultValue) const {
		return map_t::get(key, defaultValue);
	}

	inline
	const drain::FlexibleVariable & get(const std::string & key) const {
		return (*this)[key];
	}

	/*
	inline
	const drain::FlexibleVariable & get(const std::string & key) const {
		return (*this)[key];
	}
	*/

	template <typename ... TT>
	inline
	void addClass(const std::string & s, const TT &... args) {
		classList.insert(s);
		setClass(args...);
	}

	inline
	bool hasClass(const std::string & cls) const {
		return (classList.find(cls) != classList.end());
	}

	inline
	void clearClasses(){
		classList.clear();
	}


	inline
    NodeXML & operator=(const std::initializer_list<std::pair<const char *,const drain::Variable> > &l){
		set(l);
		return *this;
	}


	typedef std::list<path_t> path_list_t;

	/// "Forward definition" of Tree::toOstream
	template <class V>
	static
	const path_list_t & findByClass(const V & t, const std::string & tag,
			path_list_t & result = path_list_t(), const path_t & path = path_t());

	inline
	bool empty() const {
		return map_t::empty();
	}

	/// Make this node a comment. Contained tree will not appear.
	/**
	 *   \param text - if given, replaces current CTEXT.
	 *
	 */
	inline
	void setComment(const std::string & text = "") {

		setType((elem_t)COMMENT);

		if ((int)getType() != COMMENT){ //cross-check

			throw std::runtime_error(drain::TypeName<NodeXML<T> >::str() + ": COMMENT not supported");
		}
		/*
		if (id > 0){
			id = -id;
		}
		if (!text.empty()){
			ctext = text
		}
		*/
		ctext = text;
	}

	/// Make this node commented.
	/**
	 *   \param text - if given, replaces current CTEXT.
	 *
	 */
	inline
	void setText(const std::string & text = "") {

		setType((elem_t)CTEXT);

		if ((int)getType() != CTEXT){ //cross-check
			throw std::runtime_error(drain::TypeName<NodeXML<T> >::str() + ": CTEXT not supported");
		}

		ctext = text;

	}

	inline
	bool isComment() const {
		return typeIs((elem_t)COMMENT);
	}

	inline
	bool isText() const {
		return typeIs((elem_t)CTEXT);
	}

	/// "Forward definition" of Tree::toOstream
	template <class V>
	static
	std::ostream & toStream(std::ostream &ostr, const V & t, const std::string & defaultTag = "", int indent=0);

protected:

	inline
	void setClass(){}

	template <class V>
	static inline
	void attribToStream(std::ostream &ostr, const std::string & key, const V &value){
		ostr << key << '=' << '"' << value << '"' << ' ';
	}

	static int nextID;

	// TODO: change to string, still allowing numerics
	int id;

	// TODO: consider TAG from dict?
	// std::string tag;

	typedef std::set<std::string> class_list;
	class_list classList;

};

template <class T>
const int NodeXML<T>::UNDEFINED(0);

template <class T>
const int NodeXML<T>::COMMENT(1);

template <class T>
const int NodeXML<T>::CTEXT(2);



typedef drain::UnorderedMultiTree<NodeXML<>,false, NodeXML<>::path_t> TreeXML;

// NOTE: template will not match for subclasses of NodeXML<E>
template <class E, bool EX, class P>
struct TypeName< drain::UnorderedMultiTree<NodeXML<E>,EX,P> > {

    static const std::string & str(){
    	static const std::string name = drain::StringBuilder<>("TreeXML<", TypeName<E>::get(), ">");
    	return name;
    }

    static const char* get(){
    	//static const std::string name = drain::StringBuilder<>("Tree<", TypeName<E>::get(), ">");
    	return str().c_str();
    }
};

//typedef drain::UnorderedMultiTree<NodeXML<>,false> TreeXML;

template <>
TreeXML & TreeXML::addChild(const TreeXML::key_t & key);

template <class N>
int NodeXML<N>::nextID = 0;


template <class N>
template <class T>
const NodeXML<>::path_list_t & NodeXML<N>::findByClass(const T & t, const std::string & cls,
		NodeXML<>::path_list_t & result, const path_t & path){

	if (t->classList.find(cls) != t->classList.end()){
		result.push_back(path);
	}

	for (const auto & entry: t){
		findByClass(entry.second, cls, result, path_t(path, entry.first));
	}
	return result;
}

/**
 *   \param defaultTag - important for
 *
 *   Consider indentation: std::fill_n(std::ostream_iterator<char>(std::cout), 2*i, ' '); ?
 *
 */
template <class N>
template <class T>
std::ostream & NodeXML<N>::toStream(std::ostream & ostr, const T & tree, const std::string & defaultTag, int indent){

	const typename T::container_t & children = tree.getChildren();

	// Indent
	std::fill_n(std::ostream_iterator<char>(ostr), 2*indent, ' ');

	// Start dag
	if (tree->isComment()){
		ostr << "<!-- " << tree->getTag() << ' ' << tree->ctext; // << " /-->\n";
	}
	else if (tree->getTag().empty())
		ostr << '<' << defaultTag << ' ';
	else {
		ostr << '<' << tree->getTag() << ' ';
		// TODO check GDAL XML
		if (!defaultTag.empty())
			attribToStream(ostr, "name", defaultTag);
	}

	// if (tree->id >= 0){
	if (!tree->classList.empty()){
		ostr << "class=\"";
		std::copy(tree->classList.begin(), tree->classList.end(), std::ostream_iterator<std::string>(ostr, " "));
		ostr << "\" ";
	}
		// attribToStream(ostr, "id", tree->id); // problem for palette?
		//ostr << "id=\"" << tree.data.id << '"' << ' ';
	// }

	/// iterate attributes - note: also for comment
	for (const typename T::node_data_t::key_t & key: tree.data.getKeyList()){
		std::stringstream sstr;
		sstr << tree.data[key];  // consider checking 0, not only empty string "".
		if (!sstr.str().empty()){
			attribToStream(ostr, key, sstr.str());
			// ostr << key << "=\"" << sstr.str() << '"' << ' ';
		}

	}

	if (tree->isComment()){
		ostr << " /-->\n";
	}
	else if ((children.size() == 0) && tree->ctext.empty() ){ // OR no ctext!
		// close TAG
		ostr << '/' << '>';
		ostr << '\n';
	}
	else {
		// close starting TAG
		ostr << '>';
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
		if (!children.empty()){
			std::fill_n(std::ostream_iterator<char>(ostr), 2*indent, ' ');
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
	  return TreeXML::node_data_t::toStream(ostr, t, "");
}


}  // drain::

#endif /* TREEXML_H_ */

