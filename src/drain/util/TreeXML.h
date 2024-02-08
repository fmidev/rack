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



#ifndef TREEXML_H_
#define TREEXML_H_

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

	typedef T elem_t;
	elem_t type;

	//typedef ReferenceMap map_t;
	typedef ReferenceMap2<FlexibleVariable> map_t;

	inline
	NodeXML() : id(++nextID){
		link("id", id);
	};


	NodeXML(const NodeXML & node) : id(++NodeXML::nextID) {
		copyStruct(node, node, *this, RESERVE); // This may corrupt (yet unconstructed) object?
	}

	inline
	~NodeXML(){};

	virtual
	void setType(const elem_t &t){
		type = t;
	}

	inline
	const elem_t & getType() const {
		return type;
	};


	inline
	const std::string & getTag() const {return tag;};


	//typedef FlexVariableMap map_t;
	// std::string id;  // int?
	// std::string name;
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

	typedef std::set<std::string> class_list;
	class_list classList;

	template <typename ... TT>
	inline
	void setClass(const std::string & s, const TT &... args) {
		classList.insert(s);
		setClass(args...);
	}

	inline
	void setClass(){}

	// TODO: handle attrib set("class", ...) -> setCLass

	// TODO: handle attrib set("style", ...) -> setStyle

	inline
    NodeXML & operator=(const std::initializer_list<std::pair<const char *,const drain::Variable> > &l){
		set(l);
		return *this;
	}



	template <class V>
	static inline
	void attribToStream(std::ostream &ostr, const std::string & key, const V &value){
		ostr << key << '=' << '"' << value << '"' << ' ';
	}

	/// "Forward definition" of Tree::toOstream
	template <class V>
	static
	std::ostream & toStream(std::ostream &ostr, const V & t, const std::string & defaultTag = "", int indent=0);

	std::string ctext;

	inline
	bool empty() const {
		return map_t::empty();
		//return FlexVariableMap::empty();
		//return this->(FlexVariableMap::empty)();
		//return ReferenceMap::empty();
	}

	/// Make this node commented.
	/**
	 *   \param text - if given, replaces current CTEXT.
	 *
	 */
	inline
	void comment(const std::string & text = "") {
		if (id > 0){
			id = -id;
		}
		if (!text.empty()){
			ctext = text;
		}
	}

	inline
	bool isComment() {
		return (id < 0);
	}

protected:

	static int nextID;
	std::string tag;
	int id;

};



// #define TreeXML drain::Tree<std::string,NodeXML>  // , std::less<std::basic_std::string<char>
// typedef drain::Tree<std::string,NodeXML> TreeXML;
typedef drain::UnorderedMultiTree<NodeXML<>,false> TreeXML;

template <>
TreeXML & TreeXML::addChild(const TreeXML::key_t & key);

template <class N>
int NodeXML<N>::nextID = 0;

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

	// OPEN TAG
	std::fill_n(std::ostream_iterator<char>(ostr), 2*indent, ' ');

	// isComment()
	if (tree->id < 0){
		ostr << "<!-- " << tree->getTag() << ' ' << tree->ctext << " /-->\n";
		return ostr;
	}


	ostr << '<';
	//<< tree.data.tag << ' ';
	if (tree->getTag().empty())
		ostr << defaultTag << ' ';
	else {
		ostr << tree->getTag() << ' ';
		// TODO if (tree.data.name.empty()) ?
		if (!defaultTag.empty())
			attribToStream(ostr, "name", defaultTag);
			//ostr << "name=\"" << defaultTag << '"' << ' ';
	}

	if (tree->id >= 0){
		if (!tree->classList.empty()){
			ostr << "class=\"";
			std::copy(tree->classList.begin(), tree->classList.end(), std::ostream_iterator<std::string>(ostr, " "));
			ostr << "\" ";
		}
		// attribToStream(ostr, "id", tree->id); // problem for palette?
		//ostr << "id=\"" << tree.data.id << '"' << ' ';
	}

	/// iterate attributes
	for (const typename T::node_data_t::key_t & key: tree.data.getKeyList()){
		std::stringstream sstr;
		sstr << tree.data[key];  // consider checking 0, not only empty string "".
		if (!sstr.str().empty()){
			attribToStream(ostr, key, sstr.str());
			// ostr << key << "=\"" << sstr.str() << '"' << ' ';
		}

	}

	if ((children.size() == 0) && tree->ctext.empty() ){ // OR no ctext!
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
		std::fill_n(std::ostream_iterator<char>(ostr), 2*indent, ' ');
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

