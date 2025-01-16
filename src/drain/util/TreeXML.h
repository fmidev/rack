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

	typedef T xml_tag_t;
	typedef NodeXML<T> xml_node_t;
	typedef drain::Path<std::string,'/'> path_t;
	// typedef path_t::elem_t path_elem_t;
	typedef UnorderedMultiTree<xml_node_t,false, path_t> xml_tree_t;
	// typedef UnorderedMultiTree<xml_node_t,false, drain::Path<std::string,'/'> > xml_tree_t;
	// typedef std::list<path_t> path_list_t; // could be?

	inline
	NodeXML(const intval_t & t = intval_t(0)){
		setType(t);
		//setId(++nextID);
		drain::StringTools::import(++nextID, id);
	};

	// Note: use default constructor in derived classes.
	inline
	NodeXML(const NodeXML & node){
		//drain::StringTools::import(++nextID, id);
		// RISKY copyStruct(node, node, *this, map_t::RESERVE); // Needed? setType will handle? And this may corrupt (yet unconstructed) object?
		// type = node.getType();
		XML::xmlAssignNode(*this, node);
		drain::StringTools::import(++nextID, id);
		// setType(node.getType());
		// setId(++nextID);
		// drain::StringTools::import(++nextID, id);
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

	/// Return true, if type is any of the arguments.
	/**
	 *
	 */
	template <class T2, class ...T3>
	inline
	bool typeIs(const T2 & arg, const T3... args) const {
		if (type == static_cast<intval_t>(arg)){
			return true;
		}
		else {
			return typeIs(args...);
		}
	};

protected:

	inline
	bool typeIs() const {
		return false;
	};

public:

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
	bool isSelfClosing() const { // rename explicit closing

		static
		const std::set<intval_t> l = {SCRIPT, STYLE}; // todo, append, and/or generalize...

		return (l.find(this->getType()) == l.end()); // not in the set

		//return false;
	}

	/**
	 *   This function should not be called upon construction.
	 */
	virtual
	const std::string & getTag() const {
		// std::cout << __FILE__ << ':' << __FUNCTION__ << "(const) dict: " << drain::TypeName<T>::str()  << std::endl; // << drain::EnumDict<T>::dict
		// std::cout << __FILE__ << ':' << __FUNCTION__ << ' ' << sprinter(drain::EnumDict<T>::dict) << std::endl; // <<
		//return drain::EnumDict<T>::dict.getKey((T)type, false); // throw error
		return drain::EnumDict<T>::getDict().getKey((T)type, false);
 		//static std::string dummyTag = "DUMMY";
		//return dummyTag;

	}

	/**
	 *   This function should not be called upon construction. Consider dynamic getDict() to skip
	 */
	static inline // needed?
	const std::string & getTag(const T & type){
		// std::cout << __FILE__ << ':' << __FUNCTION__ << " non-const dict: "  << drain::TypeName<T>::str() << std::endl; // << drain::EnumDict<T>::dict
		// std::cout << __FILE__ << ':' << __FUNCTION__ << ' '<< sprinter(drain::EnumDict<T>::dict) << std::endl; // <<
		// return drain::EnumDict<T>::dict.getKey((T)type, false); // throw error
		return drain::EnumDict<T>::getDict().getKey((T)type, false);
		// static std::string dummyTag = "DUMMY";
		// return dummyTag;
	}



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
			// Modify collection
			// setStyle(value);
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.unimplemented("Setting style as attribute: \"style\"=", value);
			//setStyle(key, value);
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
	typedef std::map<std::string,std::string> xmldoc_attrib_map_t;
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
	if (!node.getClasses().empty()){
		//ostr << '['; // has already braces []
		//drain::Sprinter::toStream(ostr, node.classList, drain::Sprinter::pythonLayout);
		drain::Sprinter::toStream(ostr, node.getClasses(), ClassListXML::layout);
		//ostr << ']' << ' ';
		ostr << ' ';
	}
	if (!node.getStyle().empty()){
		ostr << '{';
		drain::Sprinter::toStream(ostr, node.getStyle());
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

	// TODO: delegate to XML node start/end function, maybe xmlNodeToStream ?

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
		xmlAttribToStream(ostr, "data-mode", "experimental");
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
					xmlAttribToStream(ostr, key, sstr.str());
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

