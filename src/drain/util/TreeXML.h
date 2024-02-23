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
#include "Sprinter.h"
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

	typedef UnorderedMultiTree<NodeXML<T>,false, path_t> xml_tree_t;

	typedef ReferenceMap2<FlexibleVariable> map_t;

	inline
	NodeXML(const elem_t & t = elem_t(0)){
		type = t;
		drain::StringTools::import(++nextID, id);
		// link("id", id);
	};

	// Note: use default constructor in derived classes.
	inline
	NodeXML(const NodeXML & node){
		drain::StringTools::import(++nextID, id);
		copyStruct(node, node, *this, RESERVE); // This may corrupt (yet unconstructed) object?
		type = node.getType();
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
			(*this)[key] = value;
		}

	}





	inline
	const drain::FlexibleVariable & get(const std::string & key) const {
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
	void addClass(const std::string & s, const TT &... args) {
		classList.insert(s);
		addClass(args...);
	}

	inline
	bool hasClass(const std::string & cls) const {
		return (classList.find(cls) != classList.end());
	}

	inline
	void removeClass(const std::string & s) {
		class_list::iterator it = classList.find(s);
		if (it != classList.end()){
			classList.erase(it);
		}
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
		mout.error("unsupported type ", drain::TypeName<S>::get(), " for value: ", value);
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

	inline
	void setStyle(const std::string & key, const std::initializer_list<std::pair<const char *,const drain::Variable> > &l){
		drain::SmartMapTools::setValues(style, l);
	}

	typedef std::set<std::string> class_list;
	class_list classList;

	typedef std::list<path_t> path_list_t;

	/// "Forward definition"
	/**
	 *   By definition, id attributes should be unique. This function nevertheless returns a list
	 *   for easy handling of cases with zero or more than one elements found.
	 */
	//   This could also be in TreeXMLutilities
	template <class V>
	static
	const path_list_t & findById(const V & t, const std::string & tag, path_list_t & result = path_list_t(), const path_t & path = path_t());

	/// "Forward definition"
	//   This could also be in TreeXMLutilities
	template <class V>
	static
	const path_list_t & findByClass(const V & t, const std::string & tag, path_list_t & result = path_list_t(), const path_t & path = path_t());


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

protected:

	typedef std::map<std::string,std::string> xmldoc_attrib_map_t;

	static xmldoc_attrib_map_t xmldoc_attribs;

	inline
	void addClass(){}

	template <class V>
	static inline
	void attribToStream(std::ostream &ostr, const std::string & key, const V &value){
		ostr << key << '=' << '"' << value << '"' << ' ';
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
    	static const std::string name = drain::StringBuilder<>("TreeXML<", TypeName<E>::get(), ">");
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
const NodeXML<>::path_list_t & NodeXML<N>::findById(const T & t, const std::string & id, NodeXML<>::path_list_t & result, const path_t & path){

	if (t->id == id){
		result.push_back(path);
	}

	for (const auto & entry: t){
		findByClass(entry.second, id, result, path_t(path, entry.first));
	}

	return result;
}

template <class N>
template <class T>
const NodeXML<>::path_list_t & NodeXML<N>::findByClass(const T & t, const std::string & cls, NodeXML<>::path_list_t & result, const path_t & path){

	if (t->classList.find(cls) != t->classList.end()){
		result.push_back(path);
	}

	for (const auto & entry: t){
		findByClass(entry.second, cls, result, path_t(path, entry.first));
	}
	return result;
}


template <class N>
inline
std::ostream & operator<<(std::ostream &ostr, const NodeXML<N> & node){
	ostr << node.getTag() << '<' << (unsigned int)node.getType() << '>' << ' ';
	// drain::Sprinter::toStream(ostr, node.getAttributes(), drain::Sprinter::jsonLayout);
	// drain::Sprinter::toStream(ostr, node.getAttributes().getMap(), drain::Sprinter::jsonLayout);
	//
	if (!node.getAttributes().empty()){
		drain::Sprinter::toStream(ostr, node.getAttributes().getMap(), drain::Sprinter::xmlAttributeLayout);
		ostr << ' ';
	}
	if (!node.classList.empty()){
		ostr << '[';
		drain::Sprinter::toStream(ostr, node.classList, drain::Sprinter::pythonLayout);
		ostr << ']' << ' ';
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

/// Output function shared for all XML documents, ie versions defined in subclasses.
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
		ostr << '<' << defaultTag << ' ';
	else {
		ostr << '<' << tree->getTag() << ' ';
		// TODO check GDAL XML
		//if (!defaultTag.empty())
		//	attribToStream(ostr, "name", defaultTag);
	}

	if (tree->typeIs((elem_t)STYLE)){
		attribToStream(ostr, "data-mode", "experimental");
		// Sprinter::sequenceToStream(ostr, tree->style, StyleXML::styleRecordLayout);
		// ostr << "\n /-->";
	}
	else if (!tree->isCText()){
		//char sep=' ';
		if (!tree->classList.empty()){
			ostr << "class=\"";
			std::copy(tree->classList.begin(), tree->classList.end(), std::ostream_iterator<std::string>(ostr, " "));
			ostr << "\" ";
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
			ostr << "style=\"";
			Sprinter::sequenceToStream(ostr, tree->style, StyleXML::styleLineLayout);
			//Sprinter::mapToStream(ostr, tree->style, StyleXML::styleLineLayout);
			ostr << '"' << ' ';
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
	else if ((!tree->typeIs((elem_t)STYLE)) && (children.empty()) && tree->ctext.empty() ){ // OR no ctext!
		// close TAG
		ostr << "/>\n";
		//ostr << '/' << '>';
		//ostr << '\n';
	}
	else {
		// close starting TAG
		ostr << '>';

		/*
		if (!tree->style.empty()){
			ostr << "<!-- STYLE? ";
			drain::Sprinter::toStream(ostr, tree->style.getMap(), drain::Sprinter::xmlAttributeLayout);
			ostr << "/-->\n";
		}
		*/

		if (tree->typeIs((elem_t)STYLE)){
			ostr << "<![CDATA[ ";
			if (!tree->getAttributes().empty()){
				ostr << "\n\t<!-- DISCARDED attribs ";
				drain::Logger mout(__FILE__,__FUNCTION__);
				mout.warn("STYLE elem contains attributes, probably meant as style: ", tree.data);
				Sprinter::toStream(ostr, tree->getAttributes()); //, StyleXML::styleRecordLayout
				ostr << " /-->" << '\n';
			}
			if (!tree->style.empty()){
				// ostr << "\n\t<!-- attribs /-->" << '\n';
				ostr << fill << "  <!-- std style /-->" << '\n';
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
			ostr << "\n\t<!-- elems /-->" << '\n';
			for (const auto & entry: tree.getChildren()){
				if (!entry.second->getAttributes().empty()){
					ostr << "\n\t<!-- elem("<< entry.first << ") attribs /-->" << '\n';
					ostr << fill << "  " << entry.first << " {\n";
					for (const auto & attr: entry.second->getAttributes()){
						ostr << fill  << "    ";
						Sprinter::pairToStream(ostr, attr, StyleXML::styleRecordLayout); // {" :;"}
						//attr.first << ':' attr.first << ':';
						ostr << '\n';
					}
					ostr << fill << "  }\n";
					//Sprinter::sequenceToStream(ostr, entry.second->getAttributes(), StyleXML::styleRecordLayoutActual);
					ostr << '\n';
				}
				if (!entry.second->ctext.empty()){
					ostr << "\n\t<!-- elem("<< entry.first << ") ctext /-->" << '\n';
					ostr << fill  << "  " << entry.first << " {" << entry.second->ctext << "}\n";
				}
				// Sprinter::sequenceToStream(ostr, entry.second->style, StyleXML::styleRecordLayout);
			}
			ostr << "]]\n";
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

