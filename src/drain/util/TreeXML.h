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
// template <class T=int>
template <class T=int>
class NodeXML : public XML {

public:


	/// Tag type, CTEXT or COMMENT.
	//  typedef T elem_t;  // consider xml_elem_t
	//typedef int elem_t;  // consider xml_elem_t

	// intval_t type = XML::UNDEFINED;

	typedef NodeXML<T> xml_node_t;
	typedef UnorderedMultiTree<xml_node_t,false, path_t> xml_tree_t;



	inline
	NodeXML(const intval_t & t = intval_t(0)){
		// type = t;
		setType(t);
		setId(++nextID);
		// drain::StringTools::import(++nextID, id);
		// id = getTag()+id; UNDEF
	};

	// Note: use default constructor in derived classes.
	inline
	NodeXML(const NodeXML & node){
		//drain::StringTools::import(++nextID, id);
		copyStruct(node, node, *this, map_t::RESERVE); // Needed? setType will handle? And this may corrupt (yet unconstructed) object?
		// type = node.getType();
		setType(node.getType());
		setId(++nextID);
		// drain::StringTools::import(++nextID, id);
	}


	inline
	~NodeXML(){};


	template <class T2> // "final"
	void setType(const T2 &t){ // DANGER, without cast?
		//void setType(const T &t){
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

	template <class T2>
	inline
	bool typeIs(const T2 &t) const {
		return type == static_cast<intval_t>(t); //
		// return type == (elem_t)t; // change to static casts
	};
	/*
	template <elem_t E>
	inline
	bool typeIs() const {
		return type == E;
	};*/

	inline
	bool isUndefined() const {
		return type == UNDEFINED;
		// return typeIs((elem_t)UNDEFINED);
		//return ((int)getType() == UNDEFINED);
	}


	// Consider raising these to XML
	inline
	bool isComment() const {
		return type == COMMENT;
		//return typeIs((elem_t)COMMENT);
	}

	inline
	bool isCText() const {
		return type == CTEXT;
		// return typeIs((elem_t)CTEXT);
	}

	inline
	bool isStyle() const {
		return type == STYLE;
	}

// protected:

	// virtual
	// TODO: strictly open/closed/flexible?
	bool isSelfClosing() const {

		static
		const std::set<intval_t> l = {SCRIPT}; // todo, append, and/or generalize...

		return (l.find(this->getType()) == l.end()); // not in the set

		//return false;
	}

	virtual
	const std::string & getTag() const {
		return drain::EnumDict<T>::dict.getKey((T)type, false);
	}

	static inline
	const std::string & getTag(const T & type){
		return drain::EnumDict<T>::dict.getKey(type, false);
	}

	/* 2025
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
	*/



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
	void set(const intval_t & type){
		setType(type);
	}

	inline
	void set(const std::string & s){
		if (type == STYLE){
			// setStyle(s);
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
	void set(const std::map<std::string, V> & args){
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



	// Check char *
	inline
	NodeXML & operator=(const intval_t & x){
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

	/// Make this node a comment. Contained tree will not be delete. In current version, attributes will be rendered.
	/**
	 *   \param text - if given, replaces current CTEXT.
	 *
	 */
	inline
	NodeXML & setComment(const std::string & text = "") {

		setType((intval_t)COMMENT);

		if ((int)getType() != COMMENT){ //cross-check
			throw std::runtime_error(drain::TypeName<NodeXML<intval_t> >::str() + ": COMMENT not supported");
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
			setType(XML::CTEXT);
			if (!isCText()){ //cross-check
				throw std::runtime_error(drain::TypeName<NodeXML<intval_t> >::str() + ": CTEXT not supported");
			}
		}
		else if (isStyle()){ // typeIs(STYLE)
			setStyle(value); //
			// SmartMapTools::updateValues(this->style, value, ';', ':');
			//
		}
		drain::StringTools::import(value, ctext);
		return *this;
	}

	inline
	NodeXML & setText(const char *value) {
		setText(std::string(value));
		return *this;
	}

	// ------------------ Style ---------------


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
		// const std::initializer_list<Variable::init_pair_t > &l
		// drain::Logger mout(__FILE__, __FUNCTION__);
		// mout.special<LOG_WARNING>("Spezial str");
		this->style[key] = value;
	}


	/// Set style of an element.
	/**
	  \code
	     rect.setStyle("dash-array",{2,5,3});
	  \endcode
	 *
	 */
	template <class V>
	inline
	void setStyle(const std::string & key, const std::initializer_list<V> &l){
		// const std::initializer_list<Variable::init_pair_t > &l
		if (type == STYLE){  // typeIs(STYLE) fails
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.warn("Setting style of STYLE?  initializer_list<", drain::TypeName<V>::str(), "> = ", sprinter(l)); // , StyleXML::styleLineLayout ?
			//mout.revised<LOG_WARNING>("Element is not STYLE but ", getTag(), ", initializer_list<", drain::TypeName<V>::str(), "> = ", sprinter(l)); // , StyleXML::styleLineLayout ?
		}
		this->style[key] = l;
	}


	/// For element/class/id, assign ...
	/**
	 *
	 */
	template <class V>
	inline
	void setStyle(const std::string & key, const V & value){
		// drain::Logger mout(__FILE__, __FUNCTION__);
		if (type == STYLE){
			drain::Logger mout(__FILE__, __FUNCTION__);
			// "reuse" style map as style record map
			std::stringstream sstr;
			drain::Sprinter::toStream(sstr, value, StyleXML::styleLineLayout);
			mout.warn("Setting style of STYLE?, arg type=", drain::TypeName<V>::str());
			mout.warn("Check results: skipped ", sstr.str());
			mout.warn("Check results: applied ", value);
			this->style[key] = value; //sstr.str();
		}
		else {
			// mout.special<LOG_WARNING>("Spezial ", drain::TypeName<V>::str());
			this->style[key] = value;
		}
	}


	inline
	void setStyle(const std::initializer_list<std::pair<const char *,const drain::Variable> > &l){
		drain::SmartMapTools::setValues(style, l);
	}


	/// Finds elements in an XML structure by class name.
	/// Finds elements in an XML structure by class name. Redirects to findByClass(t, std::string(cls),



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

protected:

	/// Internal function called after setType()
	virtual
	void handleType(const T & type){ // DANGER, without cast?
	}

	/// NOTE: these could/should be templated, in TreeXML<...> right?
	typedef std::map<std::string,std::string> xmldoc_attrib_map_t;
	static xmldoc_attrib_map_t xmldoc_attribs;

};

template <>
inline
const std::string & NodeXML<int>::getTag() const {
	return drain::EnumDict<int,XML>::dict.getKey(this->type, false);
}


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
template <class T>
template <class TR>
std::ostream & NodeXML<T>::toStream(std::ostream & ostr, const TR & tree, const std::string & defaultTag, int indent){


	const typename TR::container_t & children = tree.getChildren();

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

	if (tree->typeIs((intval_t)STYLE)){
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
		//for (const typename T::node_data_t::key_t & key: tree.data.getKeyList()){
		for (const typename TR::node_data_t::key_t & key: tree->getAttributes().getKeyList()){
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
			(!tree->typeIs((intval_t)STYLE)) && (!tree->typeIs((intval_t)SCRIPT)) &&
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

		if (tree->isStyle()){
			// https://www.w3.org/TR/xml/#sec-cdata-sect
			// ostr << "<![CDATA[ \n";

			if (!tree->ctext.empty()){
				// TODO: indent
				ostr << fill << tree->ctext << " /* CTEXT? */" << '\n';
			}

			if (!tree->getAttributes().empty()){
				drain::Logger mout(__FILE__,__FUNCTION__);
				mout.warn("STYLE elem ", tree->getId()," contains attributes, probably meant as style: ", sprinter(tree->getAttributes()));
				ostr << "\n\t /* <!-- DISCARDED attribs ";
				Sprinter::toStream(ostr, tree->getAttributes()); //, StyleXML::styleRecordLayout
				ostr << " /--> */" << '\n';
			}

			if (!tree->style.empty()){
				ostr << fill << "/** style obj **/" << '\n';
				for (const auto & attr: tree->style){
					ostr << fill << "  ";
					Sprinter::pairToStream(ostr, attr, StyleXML::styleRecordLayout); // {" :;"}
					//attr.first << ':' attr.first << ':';
					ostr << '\n';
				}
				// ostr << fill << "}\n";
				// Sprinter::sequenceToStream(ostr, entry.second->getAttributes(), StyleXML::styleRecordLayoutActual);
				// ostr << '\n';
			}

			ostr << '\n';
			// ostr << fill << "<!-- elems /-->" << '\n';
			ostr << fill << "/* elems */" << '\n';
			for (const auto & entry: tree.getChildren()){
				if (!entry.second->ctext.empty()){
					//ostr << fill << "<!-- elem("<< entry.first << ") ctext /-->" << '\n';
					ostr << fill << "  " << entry.first << " {" << entry.second->ctext << "} /* CTEXT */ \n";
				}
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

		if (tree->typeIs((intval_t)STYLE) || !children.empty()){
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

