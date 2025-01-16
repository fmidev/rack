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

// ------------------------- public UtilsXML,

class XML :  protected ReferenceMap2<FlexibleVariable> {
public:

	typedef int intval_t;

	// TODO:
	// static const intval_t flag_OPEN = 128;
	// static const intval_t flag_TEXT = 256;

	static const intval_t UNDEFINED = 0;
	static const intval_t COMMENT = 1;  // || flag_TEXT
	static const intval_t CTEXT = 2;  // || flag_TEXT
	static const intval_t SCRIPT = 3; // || flag_EXPLICIT || flag_TEXT
	static const intval_t STYLE = 4;  // || flag_EXPLICIT
	static const intval_t STYLE_SELECT = 5;


	intval_t type = XML::UNDEFINED;

	typedef ReferenceMap2<FlexibleVariable> map_t;

	/// Some general-purpose variables

	// String, still easily allowing numbers through set("id", ...)
	std::string id;

	// Consider either/or
	std::string ctext;

	std::string url;

	// Could be templated, behind Static?
	static int nextID;

	inline
	static int getCount(){
		return nextID;
	}

	/// Clear style, class and string data but keep the element type.
	/**
	 *  Clears style, classList, cdata.
	 *
	 */
	virtual
	void clear();

	/// Clear style, class and string data as well as the element type.
	/**
	 *  Clears style, classList, cdata.
	 *
	 */
	inline
	void reset(){
		clear();
		type = UNDEFINED;
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
	void setText(const std::string & s);

	template <class ...T>
	void setText(const T & ...args) {
		setText(StringBuilder<>(args...).str()); // str() to avoid infinite loop
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

	/*
	template <typename K, typename V>
	inline
	void setStyle(const std::initializer_list<std::pair<K,V> > &args){
		drain::SmartMapTools::setValues(style, args);
	}
	*/

	inline
	void setStyle(const std::initializer_list<std::pair<const char *,const drain::Variable> > &args){
		drain::SmartMapTools::setValues(style, args);
	}

	/*
	inline
	void setStyle(	const std::initializer_list<std::pair<const char *,const char *> > & args){
		drain::SmartMapTools::setValues(style, args);
	}
	*/

	/*
	template <class S>
	inline
	void setStyle(const S &value){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.error("unsupported type ", drain::TypeName<S>::str(), " for value: ", typeid(value));
	}
	*/


	// ------------------ Style Class ---------------

protected:

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

// ----------------- Static utilities for derived classes ----------------------

	template <class V>
	static inline
	void xmlAttribToStream(std::ostream &ostr, const std::string & key, const V &value){
		ostr << ' ' << key << '=' << '"' << value << '"'; // << ' ';
	}

	/// Assign another tree structure to another
	/**
	 *  \tparam XML - xml tree structure (TreeXML, TreeSVG, TreeHTML)
	 */
	template <typename XML>
	static inline
	XML & xmlAssign(XML & dst, const XML & src){

		if (&src != &dst){
			dst.clear();
			// also dst->clear();
			dst->setType(src->getType());
			dst->ctext = src->ctext;
			dst->getAttributes() = src->getAttributes();
		}

		return dst;
	}

	/// Assign another tree structure to another
	/**
	 *  \tparam XML - xml tree structure (TreeXML, TreeSVG, TreeHTML)
	 */
	template <typename X>
	static inline
	X & xmlAssign(X & dst, const typename X::xml_node_t & src){

		if (&src != &dst.data){
			dst->clear();
			dst->getAttributes().importMap(src.getAttributes());
			dst->setStyle(src.getStyle());
			dst->setText(src.ctext);
		}

		return dst;
	}

	/// Assign another tree structure to another
	/**
	 *  \tparam XML - xml tree structure (TreeXML, TreeSVG, TreeHTML)
	 *
	 *  \see clear()
	 */
	template <typename X>
	static inline
	X & xmlAssignNode(X & dst, const X & src){

		if (&src != &dst){
			//dst.clear(); // clear attributes,
			dst.reset(); // clear attributes, style, cstring and type.
			dst.setType(src.getType()); // important: creates links!
			dst.getAttributes().importMap(src.getAttributes());
			dst.setStyle(src.getStyle());
			dst.ctext = src.ctext;
		}

		return dst;
	}

	/// Assign another tree structure to another
	/**
	 *  \tparam T - XML tree
	 */
	template <typename XML, typename V>
	static inline
	XML & xmlAssign(XML & tree, const V & arg){
		tree->set(arg);
		return tree;
	}

	/// Tree
	/**
	 *
	 */
	template <typename T>
	static
	//T & xmlAssign(T & tree, std::initializer_list<std::pair<const char *,const char *> > l){
	T & xmlAssign(T & tree, std::initializer_list<std::pair<const char *,const Variable> > l){

		switch (static_cast<intval_t>(tree->getType())){
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


	///
	/**
	 *   Forward definition – type can be set only upon construction of a complete
	 *
	 */
	template <typename T>
	static inline
	T & xmlSetType(T & tree, const typename T::node_data_t::xml_tag_t & type){
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
		typename T::node_data_t::xml_tag_t type = xmlGuessType(tree.data);

		if (!key.empty()){
			return tree[key](type);
		}
		else {
			std::stringstream k("elem");
			k.width(3);
			k.fill('0');
			k << tree.getChildren().size();
			return tree[k.str()](type);
			//return xmlGuessType(tree.data, tree[k.str()]);
			/*
			T & child = tree[k.str()];
			typedef typename T::node_data_t::xml_default_elem_map_t map_t;
			const typename map_t::const_iterator it = T::node_data_t::xml_default_elems.find(tree->getNativeType());
			if (it != T::node_data_t::xml_default_elems.end()){
				child->setType(it->second);
				drain::Logger(__FILE__, __FUNCTION__).experimental<LOG_WARNING>("Default type set: ", child->getTag());
			}
			// NodeXML<drain::image::svg::tag_t>::xml_default_elem_map_t
			return child;
			*/
		}
	}

	template <typename N>
	static
	typename N::xml_tag_t xmlGuessType(const N & parentNode){
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






}  // drain::

#endif /* DRAIN_XML */

