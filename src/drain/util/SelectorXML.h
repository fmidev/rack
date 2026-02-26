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



#ifndef DRAIN_SELECTOR_XML
#define DRAIN_SELECTOR_XML

//#include <ostream>
// #include <drain/StringBuilder.h>
#include <drain/StringWrapper.h>
#include <drain/Converter.h>
#include "ClassXML.h"

namespace drain {


enum PseudoClassCSS {
	none=0,
	active,
	disabled,
	focus,
	hover,
	link,
	scope,
	target,
	valid,
	visited,
};

DRAIN_ENUM_CONV(PseudoClassCSS);



/** NEW! 2026
 *  \tparam T - element tag_t (enum or integer)
 */
template <typename E>
class SelectXML { //  : public std::string {

public:

	template <class ...TT>
	inline
	SelectXML(TT... args) {
		set(args...);
	}

	/**
	 *   Handles three types of argument
	 *   - type E : element id's, implemented as enum lists or integers.
	 *   - string or ClassXML: CSS class id's
	 *   - PseudoClassCSS:
	 */
	template <class T, class ...TT>
	void set(const T & arg, TT... args){
		set(arg);
		set(args...);
	}

	template <class T>
	void set(const T & arg){
		//Logger mout(__FILE__, __FUNCTION__);
		cls.set(arg);
		// Logger(__FILE__, __FUNCTION__).experimental<LOG_INFO>("arg '", arg, "' of type:", typeid(T).name(), ", -> CSS class: ", cls);
		//elem = drain::EnumDict<E>::dict.getValue(arg, false);
	};

	/// Set element
	/**
	 *  \tparam E
	 *
	 *  \see setElement()
	 *  \see drain::EnumDict<E>
	 */
	inline
	void set(const E & arg){
		elem = arg;
	};

	/// Set CSS class
	/**
	 *    In CSS style definitions class "EXAMPLE" appears as ".EXAMPLE".
	 *
	 */
	inline
	void set(const std::string & arg){
		cls.assign(arg);
		//elem = drain::EnumDict<E>::dict.getValue(arg, false);
	};

	/// Set CSS class
	inline
	void set(const char *arg){
		cls.assign(arg);
		// elem = drain::EnumDict<E>::dict.getValue(arg, false);
	};


	/// Set CSS class
	inline
	void set(const ClassXML & arg){
		cls.assign(arg);
	};

	/// Set one of the element pseudo classes: focus, hover
	/**
	 *
	 */
	inline
	void set(const PseudoClassCSS & arg){
		pseudoClass.set(arg);
	};

	/// Set one of the element pseudo classes: focus, hover
	/**
	 *  Note: argument may contain an invalid value (a value not corresponding to any in PseudoClassCSS).
	 *
	 *  \see drain::PseudoClassCSS
	 */
	inline
	void set(const StringWrapper<drain::PseudoClassCSS> & p){
		pseudoClass.assign(p);
	};

	/// Set element explicitly
	/**
	 *  For setting an element type that is not (yet) supported in the enumeration dictionary.
	 *
	 *  \see template E
	 *  \see drain::EnumDict<E>
	 */
	template <class T>
	inline
	void setElement(const T & arg){
		elem = drain::EnumDict<E>::dict.getValue(arg, false);
	};


	template <class T>
	inline
	void setClass(const T & arg){
		// FIX StringConv
		cls = arg;
	};

	/// Set one of the element pseudo classes: focus, hover
	/**
	 *  This function is useful in setting a pseudo class that is not (yet) supported in the enumeration dictionary.
	 *  \see drain::PseudoClassCSS
	 *  \see drain::EnumDict<drain::PseudoClassCSS>::dict
	 */
	template <class T>
	inline
	void setPseudoClass(const T & psCls){
		pseudoClass.set(psCls);
	};





	inline
	void clear(){
		elem = 0;
		cls.clear();
		pseudoClass.clear();
	};

	void toStream(std::ostream & ostr) const {

		// TODO id?

		// Undefined ~ 0 (contract)
		if (static_cast<int>(elem) != 0){
			ostr << drain::EnumDict<E>::dict.getKey(elem);
		}

		if (!cls.empty()){
			ostr << '.' << cls;
		}

		if (!pseudoClass.empty()){
			ostr << ':' << pseudoClass;
		}

	}

	const std::string & str() const {
		//sstr.str("");
		std::stringstream sstr;
		toStream(sstr);
		currentStr = sstr.str();
		return currentStr;
		//return sstr.str();
	}

	inline
	operator const std::string &() const {
		return str();
	}


protected:

	E elem = static_cast<E>(0);
	ClassXML cls = "";
	StringWrapper<PseudoClassCSS> pseudoClass;

	inline
	void set(){};

	mutable
	std::string currentStr;
	//std::stringstream sstr;

};

}



DRAIN_ENUM_DICT(drain::PseudoClassCSS);


DRAIN_ENUM_OSTREAM(drain::PseudoClassCSS);


namespace drain {



template <typename X>
std::ostream & operator<<(std::ostream & ostr, const drain::SelectXML<X> &x){
	//return ostr << x.str();
	x.toStream(ostr);
	return ostr;
}


/// OLD, Currently used only as CSS element selector.
/*
class SelectorXML : public std::string {

public:

	static
	const char CLASS = '.';

	static
	const char ID = '#';


	inline
	SelectorXML(const std::string &s) : std::string(s){
	}

	inline
	SelectorXML(const char *s) : std::string(s){
	}

	template <class ...T>
	inline
	SelectorXML(T... args) : std::string(StringBuilder<>(args...)){
	}

};
*/

// OLD CSS class selector.
/**  Deprecated
 *
class SelectorXMLcls : public SelectorXML {
public:

	template <class C>
	inline
	SelectorXMLcls(const C &cls) : SelectorXML(CLASS, cls){
	}

	template <class E, class C>
	inline
	SelectorXMLcls(const E &elem, const C &cls) : SelectorXML(elem, CLASS, cls){
	}

};
 */

/*
class SelectorXMLid : public SelectorXML {
public:

	template <class T>
	inline
	SelectorXMLid(const T & arg) : SelectorXML(ID, arg){
	}

};
*/


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

