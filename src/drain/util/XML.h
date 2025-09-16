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
// #include "UtilsXML.h"
// #include "Flags.h"
#include "ReferenceMap.h"
#include "StyleXML.h"

namespace drain {


/// Base class for XML "nodes", to be data elements T for drain::Tree<T>
class XML :  protected ReferenceMap2<FlexibleVariable> {
public:

	typedef int intval_t;

	// TODO:
	// static const intval_t flag_OPEN = 128;
	// static const intval_t flag_TEXT = 256;
	static const intval_t UNDEFINED = 0;
	static const intval_t COMMENT   = 1; // || flag_TEXT
	static const intval_t CTEXT     = 2; // || flag_TEXT
	static const intval_t SCRIPT    = 3; // || flag_EXPLICIT || flag_TEXT
	static const intval_t STYLE     = 4; // || flag_EXPLICIT
	static const intval_t STYLE_SELECT = 5;

	enum entity_t {
		ENTITY_LESS_THAN,
		ENTITY_GREATER_THAN,
		ENTITY_NONBREAKABLE_SPACE,
		ENTITY_AMPERSAND,
	};
	//OR: const std::string ENTITY_NONBREAKABLE_SPACE = "&#160;"

	/// User may optionally filter attributes and CTEST with this using StringTools::replace(XML::encodingMap);
	static
	const std::map<char,std::string> encodingMap;

	typedef ReferenceMap2<FlexibleVariable> map_t;

protected:

	virtual
	void handleType() = 0;

public:

	inline
	XML(){};

	inline
	XML(const XML &){

	};

	template <class T> // "final"
	void setType(const T &t){ // DANGER, without cast?
		const intval_t t2 = static_cast<intval_t>(t);
		if (type != t2){
			reset();
			type = t2; // also UNDEFINED ok here
			handleType(); // NOTE: problems, if copy constructor etc. calls setType on a base class – trying to link future members
		}
		// handleType(static_cast<T>(t)); REMOVED 2025/09
		// in derived classes, eg. drain::image::BaseGDAL
		// warning: case value ‘...’ not in enumerated type
	}

	// Consider this later, for user-defined (not enumerated) tag types.
	// virtual
	// void setType(const std::string & type);


	inline
	const intval_t & getType() const {
		return type;
	};

	/// Return true, if type is any of the arguments.
	inline
	bool typeIsSet() const {
		return type != UNDEFINED;
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

protected:

	intval_t type = XML::UNDEFINED;

	// String, still easily allowing numbers through set("id", ...)
	std::string id;
	// Consider either/or
	std::string url;
public:
	std::string ctext;

	// Could be templated, behind Static?
	static int nextID;

	inline
	static int getCount(){
		return nextID;
	}


	inline
	bool isUndefined() const {
		return type == UNDEFINED;
	}

	inline
	bool isComment() const {
		return type == COMMENT;
	}

	inline
	bool isCText() const {
		return type == CTEXT;
	}

	inline
	bool isStyle() const {
		return type == STYLE;
	}

	inline
	bool isScript() const {
		return type == SCRIPT;
	}

	/// Tell if this element should always have an explicit closing tag even when empty, like <STYLE></STYLE>
	virtual
	bool isSingular() const;

	/// Tell if this element should always have an explicit closing tag even when empty, like <STYLE></STYLE>
	virtual
	bool isExplicit() const;

	/// Keep the element type but clear style, class and string data.
	/**
	 *  Clears style, classList, cdata.
	 *
	 virtual
	 void clear();
	 */

	/// Clear type, style, class and string data.
	void reset();
	/**
	 *
	inline
	void reset(){
		type = UNDEFINED;
		clear();
	}
	 */

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

	/// Concatenates arguments to an id.
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


	/// Make this node a comment. Contained tree will not be deleted. In current version, attributes WILL be rendered.
	/**
	 *   \param text - if given, replaces current CTEXT.
	 *
	 */
	template <class ...T>
	inline
	void setComment(const T & ...args) {
		this->clear(); // what if also uncommenting needed?
		// this->clearClasses();
		type = COMMENT;
		setText(args...);
	}

	/// Assign the text content of this node. If the node type is undefined, set it to CTEXT.
	/**
	 *   \param text - assigned to text content.
	 *
	 *   TODO: strict/exclusive, i.e. non-element behavior
	 */
	virtual // redef shows variadic args, below?
	void setText(const std::string & s);

	template <class ...T>
	void setText(const T & ...args) {
		setText(StringBuilder<>(args...).str()); // str() to avoid infinite loop
	}

	virtual inline
	const std::string & getText() const {
		return ctext;
	}

	inline
	const std::string & getUrl(){
		return url;
	}

	inline
	void setUrl(const std::string & s){
		url = s;
		// ctext = s;
	}

	template <class ...T>
	inline
	void setName(const T & ...args){
		setAttribute("data-name", drain::StringBuilder<>(args...));
	}


	// ---------------- Attributes ---------------

	virtual inline // shadows - consider virtual
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

	// Rename getAttribute?
	inline
	const drain::FlexibleVariable & get(const std::string & key) const {
		return (*this)[key];
	}

	// Rename getAttribute?
	inline
	drain::FlexibleVariable & get(const std::string & key){
		return (*this)[key];
	}


	// Rename getAttribute?
	template <class V>
	inline
	V get(const std::string & key, const V & defaultValue) const {
		return map_t::get(key, defaultValue);
	}

	inline
	std::string get(const std::string & key, const char * defaultValue) const {
		return map_t::get(key, defaultValue);
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
	void removeAttribute(const std::string & s){
		iterator it = this->find(s);
		if (it != this->end()){
			this->erase(it);
		}
	}


	// ------------------ Style ---------------

protected:

	StyleXML style;

public:

	inline
	const StyleXML & getStyle() const {
		return style;
	}

	inline
	void setStyle(const StyleXML & s){
		style.clear();
		SmartMapTools::setValues(style, s);
	}

	void setStyle(const std::string & value){
		drain::Logger mout(__FILE__, __FUNCTION__);
		if (type == UNDEFINED){
			mout.reject<LOG_WARNING>("setting style for UNDEFINED elem: ", value);
			mout.unimplemented<LOG_WARNING>("future option: set type to STYLE_SELECT");
		}
		else if (type == STYLE){
			mout.reject<LOG_WARNING>("not setting style for STYLE elem: ", value); // , *this);
		}
		else {
			SmartMapTools::setValues(style, value, ';', ':', "; \t\n"); // sep, equal, trim (also ';' ?)
		}
	}

	inline
	void setStyle(const char *value){
		setStyle(std::string(value));
	}

	inline
	void setStyle(const std::string & key, const std::string & value){
		drain::Logger mout(__FILE__, __FUNCTION__);
		if (type == UNDEFINED){
			mout.reject<LOG_WARNING>("setting style for UNDEFINED elem: ", key, '=', value);
			mout.unimplemented<LOG_WARNING>("future option: set type to STYLE_SELECT");
		}
		else if (type == STYLE){
			mout.reject<LOG_WARNING>("not setting style for STYLE elem: ", value); // , *this);
		}
		else {
			this->style[key] = value;
		}
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

		if (type == STYLE){
			drain::Logger(__FILE__, __FUNCTION__).reject<LOG_WARNING>("Setting style of STYLE: ", key, "=", value);
		}
		else {
			this->style[key] = value;
		}
	}

	inline
	void setStyle(const std::initializer_list<std::pair<const char *,const drain::Variable> > &args){
		drain::SmartMapTools::setValues(style, args);
	}



protected:

	// ------------------ Style Class ---------------

	ClassListXML classList;

public:

	const ClassListXML & getClasses() const {
		return classList;
	}

	template <typename ... TT>
	inline
	void addClass(const TT &... args) {
		classList.add(args...);
	}

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


	virtual
	void specificAttributesToStream(std::ostream & ostr) const;


	enum tag_display_mode {
		FLEXIBLE_TAG = 0,  // <TAG>...</TAG> or <TAG/>
		OPENING_TAG= 1,   // <TAG>
		CLOSING_TAG = 2,   // </TAG>
		EMPTY_TAG = OPENING_TAG | CLOSING_TAG,  // element has no descendants:  <hr/>
		NON_EMPTY_TAG, // opening and closing tags must appear, even when empty: <script></script>
	};


	virtual
	std::ostream & nodeToStream(std::ostream & ostr, tag_display_mode mode=EMPTY_TAG) const = 0;

// ----------------- Static utilities for derived classes ----------------------


	template <class TR>
	static
	std::ostream & toStream(std::ostream & ostr, const TR & tree, const std::string & defaultTag="ELEM", int indent=0);


	template <class V>
	static inline
	void xmlAttribToStream(std::ostream &ostr, const std::string & key, const V &value){
		//StringTools::replace(XML::encodingMap, data.ctext, ostr);
		//ostr << ' ' << key << '=' << '"' << value << '"'; // << ' ';
		ostr << ' ';
		StringTools::replace(XML::encodingMap, key, ostr);
		ostr << '=' << '"';
		StringTools::replace(XML::encodingMap, value, ostr);
		ostr << '"';
		//<< key << '=' << '"' << value << '"'; // << ' ';
	}

	/// Assign another tree structure to another
	/**
	 *  \tparam XML - xml tree structure (TreeXML, TreeSVG, TreeHTML)
	 */
	template <typename T>
	static inline
	T & xmlAssign(T & dst, const T & src){

		if (&src != &dst){
			dst.clear(); // clears children...
			// ... but not copying src? (TreeUtils?)
			// also dst->clear();
			xmlAssignNode(dst.data, src);
			/*
			dst->setType(src->getType());
			dst->setText(src->ctext); //CTXX
			dst->getAttributes() = src->getAttributes();
			*/
		}

		return dst;
	}

	/// Copy node data to tree
	/**
	 *  \tparam XML - xml tree structure (TreeXML, TreeSVG, TreeHTML)
	 */
	template <typename TX>
	static inline
	TX & xmlAssign(TX & dst, const typename TX::xml_node_t & src){

		xmlAssignNode(dst.data, src);
		/*
		if (&src != &dst.data){
			dst->clear(); NOW reset()
			dst->getAttributes().importMap(src.getAttributes());
			dst->setStyle(src.getStyle());
			dst->setText(src.ctext);
		}
		*/
		return dst;
	}

	/// Assign tree node (data) to another
	/**
	 *  \tparam N - xml node (e.g. NodeXML, NodeSVG, NodeHTML)
	 *
	 *  \see clear()
	 */
	template <typename N>
	static inline
	N & xmlAssignNode(N & dst, const N & src){

		if (&src != &dst){
			//dst.clear(); // clear attributes,
			//if (!dst.typeIs(src.getNativeType())){
			if (dst.getType() != src.getType()){
				dst.reset(); // clear attributes, style, cstring and type.
				// Warning! Dangerous situation. does not create links
				dst.setType(src.getType());
			}
			// dst.setType(src.getType()); // important: creates links!
			// dst.handleType(src.getNativeType()); // NEW
			dst.getAttributes().importMap(src.getAttributes());
			dst.setStyle(src.getStyle());
			// dst.setText(src.ctext); // wrong! set type to CTEXT
			dst.ctext = src.ctext;
		}

		return dst;
	}

	/// Assign property to a XML tree node
	/**
	 *  \tparam T - XML tree
	 */
	template <typename T, typename V>
	static inline
	T & xmlAssign(T & tree, const V & arg){
		tree->set(arg);
		return tree;
	}

	/// Tree
	/**
	 *  \tparam TX - xml tree
	 */
	template <typename T>
	static
	//T & xmlAssign(T & tree, std::initializer_list<std::pair<const char *,const char *> > l){
	T & xmlAssign(T & tree, std::initializer_list<std::pair<const char *,const Variable> > l){

		//switch (static_cast<intval_t>(tree->getType())){
		switch (tree->getType()){
		case STYLE:
			for (const auto & entry: l){
				T & elem = tree[entry.first];
				elem->setType(STYLE_SELECT);
				drain::SmartMapTools::setValues(elem->getAttributes(), entry.second, ';', ':', std::string(" \t\n"));
			}
			break;
		case UNDEFINED:
			tree->setType(STYLE_SELECT);
			// no break
		case STYLE_SELECT:
		default:
			tree->set(l);
			break;
		}

		return tree;
	};

	// UNDER CONSTRUCTION!
	/// When assigning a string, create new element unless the element itself is of type CTEXT.
	/**
	 *   \return - text element (CTEXT): current or child element of the current element
	 *
	 *   Forward definition – type can be set only upon construction of a complete class
	 */
	template <typename TX>
	static inline  // NOT YET as template specification of xmlAssign(...)
	TX & xmlAssignString(TX & tree, const std::string & s){
		if (tree->isUndefined()){
			tree->setType(CTEXT);
		}
		tree->ctext = s;
		return tree;
	}

	template <typename TX>
	static inline  // NOT YET as template specification of xmlAssign(...)
	TX & xmlAppendString(TX & tree, const std::string & s){
		if (tree->isCText()){
			tree->ctext += s;
			return tree;
		}
		else if (tree->isUndefined()){
			tree->setType(CTEXT);
			// tree->setText(s);
			tree->ctext += s;
			return tree;
		}
		else {
			// drain::Logger(__FILE__, __FUNCTION__).error("Assign string...");
			TX & child = tree.addChild();
			child->setType(CTEXT);
			child->setText(s);
			return child;
		}
	}

	///
	/**
	 *   Forward definition – type can be set only upon construction of a complete class
	 *
	 */
	template <typename TX>
	static inline
	TX & xmlSetType(TX & tree, const typename TX::node_data_t::xml_tag_t & type){
		tree->setType(type);
		return tree;
	}


	/**
	 *
	 *  TODO: add default type based on parent group? defaultChildMap TR->TD
	 *
	 */
	template <typename T>
	static
	T & xmlAddChild(T & tree, const std::string & key){
		typename T::node_data_t::xml_tag_t type = xmlRetrieveDefaultType(tree.data);

		if (!key.empty()){
			return tree[key](type);
		}
		else {
			std::stringstream k; // ("elem");
			k << "elem"; // number with 4 digits overwrites this?
			k.width(3);  // consider static member prefix
			k.fill('0');
			k << tree.getChildren().size();
			return tree[k.str()](type);
		}
	}

	template <typename N>
	static
	typename N::xml_tag_t xmlRetrieveDefaultType(const N & parentNode){
		typedef typename N::xml_default_elem_map_t map_t;
		const typename map_t::const_iterator it = N::xml_default_elems.find(parentNode.getNativeType());
		if (it != N::xml_default_elems.end()){
			return (it->second);
		}
		else {
			return static_cast<typename N::xml_tag_t>(0);
		}
	}

	/*
	template <typename T>
	static
	T & xmlGuessType(const typename T::node_data_t & parentNode, T & child){
		typedef typename T::node_data_t::xml_default_elem_map_t map_t;
		const typename map_t::const_iterator it = T::node_data_t::xml_default_elems.find(parentNode.getNativeType());
		if (it != T::node_data_t::xml_default_elems.end()){
			child->setType(it->second);
			drain::Logger(__FILE__, __FUNCTION__).experimental<LOG_WARNING>("Default type set: ", child->getTag());
		}
		return child;
	}
	*/



};


template <>
const drain::EnumDict<XML::entity_t>::dict_t drain::EnumDict<XML::entity_t>::dict;

DRAIN_ENUM_OSTREAM(XML::entity_t);

template <class TR>
std::ostream & XML::toStream(std::ostream & ostr, const TR & tree, const std::string & defaultTag, int indent){

	drain::Logger mout(__FILE__,__FUNCTION__);

	const typename TR::container_t & children = tree.getChildren();

	// const XML & data = tree.data; // template type forcing, keep here for programming aid.
	const typename TR::node_data_t & data = tree.data; // template used

	tag_display_mode mode = EMPTY_TAG;

	if (data.isCText()){ // this can be true only at root, and rarely so...? (because recursive call not reached, if ctext)
		data.nodeToStream(ostr, mode);
		// ostr << "<!--TX-->";
		return ostr;
	}

	if (!data.ctext.empty()){
		// mout.warn("Non-CTEXT-elem with ctext: <", data.getTag(), " id='", data.getId(), "' ...>, text='", data.ctext, "'");
		if (data.isSingular()){
			// mout.warn("Skipping CTEXT of a singular element <", tree->getTag(), " id='", data.getId(), "' ...> , CTEXT: '", data.ctext, "'");
			mode = EMPTY_TAG;
		}
		else {
			mode = OPENING_TAG;
		}
	}

	if (!children.empty()){
		mode = OPENING_TAG;
		if (data.isSingular()){
			mout.warn("Singular (hence normally empty) element <", tree->getTag(), " id='", data.getId(), "' ...> has ", children.size(), " children?");
		}
	}

	// mout.attention("Hey! ", TypeName<TR>::str(), " with ", TypeName<typename TR::node_data_t>::str(), "Explicit ", data.isExplicit(), " or implicit ", data.isSingular());
	if (data.isExplicit()){ // explicit
		mode = OPENING_TAG;
	}
	else if (data.isSingular()){ // <br/> <hr/>
		mode = EMPTY_TAG;
	}
	// Hence, is flexible, "bimodal", supports empty and open-close mode.

	// Indent
	// std::fill_n(std::ostream_iterator<char>(ostr), 2*indent, ' ');
	std::string fill(2*indent, ' ');
	ostr << fill;
	data.nodeToStream(ostr, mode);

	if (mode == EMPTY_TAG){
		//ostr << "<!--ET-->";
		ostr << '\n';
		return ostr;
	}
	else if (data.isStyle()){
		// https://www.w3.org/TR/xml/#sec-cdata-sect
		// ostr << "<![CDATA[ \n";
		// ostr << "<!-- STYLE -->"; WRONG!

		if (!data.ctext.empty()){
			// TODO: indent
			ostr << fill << data.ctext;
			StyleXML::commentToStream(ostr, " TEXT ");
			ostr << '\n';
		}

		if (!data.getAttributes().empty()){
			mout.warn("STYLE elem ", data.getId()," contains attributes, probably meant as style: ", sprinter(data.getAttributes()));
			ostr << "\n\t /" << "* <!-- DISCARDED attribs ";
			Sprinter::toStream(ostr, data.getAttributes()); //, StyleXML::styleRecordLayout
			ostr << " /--> *" << "/" << '\n';
		}

		if (!data.style.empty()){
			ostr << fill;
			StyleXML::commentToStream(ostr, "STYLE OBJ");
			ostr << '\n';
			for (const auto & attr: data.style){
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

		ostr << fill;
		StyleXML::commentToStream(ostr, "style ELEMS");
		ostr << '\n';

		for (const auto & entry: tree.getChildren()){

			if (entry.second->isComment()){
				// StringTools::replace();
				ostr << fill << "/* "<< entry.second->ctext << " */" << '\n';
				// XML::toStream(ostr, entry.second, defaultTag, indent); // indent not used?
				continue;
			}

			if (!entry.second->ctext.empty()){
				//ostr << fill << "<!-- elem("<< entry.first << ") ctext /-->" << '\n';
				ostr << fill << "  " << entry.first << " {" << entry.second->ctext << "} /* CTEXT */ \n";
			}

			if (!entry.second->getAttributes().empty()){
				ostr << fill << "  " << entry.first << " {\n";
				for (const auto & attr: entry.second->getAttributes()){
					ostr << fill  << "    ";
					ostr << attr.first << ':' << attr.second << ';';
					ostr << '\n';
				}
				ostr << fill << "  }\n";
				ostr << '\n';
			}

		}
		ostr << "\n"; // end CTEXT
		// ostr << " ]]>\n"; // end CTEXT
		// end STYLE defs
		ostr << fill;

	}
	else {

		// Elements "own" CTEXT will be always output first -> check for problems, if other elements added first.
		// ostr << data.ctext;

		if (data.isScript()){
			ostr << data.ctext; // let < and > pass through
		}
		else {
			ostr << data.ctext; // let < and > pass through
			// StringTools::replace(XML::encodingMap, data.ctext, ostr); // sometimes an issue? // any time issue?
		}

		// Detect if all the children are of type CTEXT, to be rendered in a single line.
		// Note: potential re-parsing will probably detect them as a single CTEXT element.
		bool ALL_CTEXT = true; // (!data.ctext.empty()) || !children.empty();

		for (const auto & entry: children){
			if (!entry.second->isCText()){
				ALL_CTEXT = false;
				break;
			}
		}

		// ALL_CTEXT = false;

		if (ALL_CTEXT){
			// ostr << "<!--ALL_CTEXT-->";
			char sep=0;
			for (const auto & entry: children){
				if (sep){
					ostr << sep;
				}
				else {
					sep = ' '; // consider global setting?
				}
				//ostr << entry.second->getText();
				StringTools::replace(XML::encodingMap, entry.second->getText(), ostr); // any time issue?
				// ostr << entry.second->getText();
			}
		}
		else {
			// ostr << "<!-- RECURSION -->";
			ostr << '\n';
			/// iterate children - note the use of default tag
			for (const auto & entry: children){
				toStream(ostr, entry.second, entry.first, indent+1); // Notice, no ++indent
				// "implicit" newline
			}
			ostr << fill; //  for CLOSING tag
		}

	}

	// ostr << "<!-- END "<< data.getId() << ' ' << data.getTag() << '(' << data.getType() << ')' << "-->";

	data.nodeToStream(ostr, CLOSING_TAG);
	ostr << '\n';  // Always after closing tag!

	return ostr;
}



}  // drain::

#endif /* DRAIN_XML */

