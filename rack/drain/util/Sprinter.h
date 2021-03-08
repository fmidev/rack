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
#ifndef Drain_Sprinter_H_
#define Drain_Sprinter_H_

#include <string.h> // strlen?
#include <iostream>
#include <stdexcept>

#include <string>
#include <utility> // pair
#include <vector>
#include <set>
#include <list>
#include <map>

#include "UniTuple.h"


namespace drain {


/// Small container for printing style for putting of structured objects (array, maps, pairs).
/**
 *  Designed for recursive traversal or structures (like JSON trees).
 *
 *  Notice that set() derived from UniTuple<char,3> has argument order: prefix, separator, suffix
 */
struct TypeLayout : public UniTuple<char,3>{

	typedef char cstr_t;
	cstr_t & prefix;
	cstr_t & separator;
	cstr_t & suffix;

	/// Constructor for simple layout using a separator but no parentheses/braces.
	TypeLayout(cstr_t separator=','):
		prefix(this->next()), separator(this->next()), suffix(this->next()){
		set(0, separator, 0);
	}

	/// Constructor
	/***
	 *    Notice that arguments (a,b) invoke set(a,0,b), not set(a,b).
	 */
	TypeLayout(cstr_t prefix, cstr_t suffix):
		prefix(this->next()), separator(this->next()), suffix(this->next()){
		set(prefix, 0, suffix);
	}

	/// Constructor.
	TypeLayout(cstr_t prefix, cstr_t separator, cstr_t suffix):
		prefix(this->next()), separator(this->next()), suffix(this->next()){
		set(prefix, separator, suffix);
	}

	/// Constructor accepting three-letter chars: {prefix,separator,suffix}
	TypeLayout(const char layout[4]): //  = "{,}"
		prefix(this->next()), separator(this->next()), suffix(this->next()){
		setLayout(layout);
	}

	TypeLayout(const TypeLayout & layout):
		//UniTuple<char,3>(),
		prefix(this->next()), separator(this->next()), suffix(this->next()){
		this->assign(layout);
	}

	/// Set layout with a single string, for example: "{,}" .
	/**
	 *  The argument string can consist of 0-3 characters:
	 *  # no layout: a sequence is simply concatenated
	 *  # separator only, for example: "," -- parentheses will be omitted
	 *  # prefix and suffix, for example: "{}" -- separator will be omitted
	 *  # prefix, separator, suffix, for example "(:)"
	 *
	 */
	TypeLayout & setLayout(const char *layout);

};

/*   USE: TypeLayout(',')
struct SimpleTypeLayout : public  TypeLayout{
	SimpleTypeLayout (char separator): TypeLayout(0,0,separator){
	}
};
*/

struct SprinterLayout{

	//char itemSeparator = ',';  // consider as equal sign:  KEY:VALUE
	TypeLayout arrayChars = TypeLayout("[,]");
	TypeLayout mapChars = TypeLayout("{,}");
	TypeLayout pairChars = TypeLayout("(,)");
	TypeLayout stringChars = TypeLayout('"',0, '"');
	//std::string boolTrue  = "true";
	//std::string boolFalse = "false";

	SprinterLayout(const char *arrayChars="[,]", const char *mapChars="{,}", const char *pairChars="(,)", const char *stringChars=nullptr){
		this->arrayChars.setLayout(arrayChars);
		this->mapChars.setLayout(mapChars);
		this->pairChars.setLayout(pairChars);

		if (stringChars)
			this->stringChars.setLayout(stringChars);
	}

	//
	///SprinterLayout(const TypeLayout & style= (), ){


	SprinterLayout(char itemSeparator){ // ','
		arrayChars.separator = itemSeparator;
		mapChars.separator = itemSeparator;
		pairChars.separator = itemSeparator;
	}


};


class SprinterBase {

public:

	/// Displayes objects with {...}, arrays with [...], pairs with (,) and strings without hyphens.
	static const SprinterLayout defaultLayout;

	/// Resembles JSON structure: {"a":1,"b":22,"c":3}
	static const SprinterLayout jsonLayout;

	/// Simply concatenate values without punctuation.
	static const SprinterLayout emptyLayout;

	/// Put each array and object element on a separate line
	static const SprinterLayout lineLayout;

	// Complicated, but unversal...
	template <class K, class V>
	static
	std::ostream & pairToStream(std::ostream & ostr, const std::pair<K,V> & x, const SprinterLayout & layout){

		if (layout.pairChars.prefix)
			ostr << layout.pairChars.prefix;

		toStream(ostr, x.first, layout);

		if (layout.pairChars.separator)
			ostr << layout.pairChars.separator; // First char

		toStream(ostr, x.second, layout);

		if (layout.pairChars.suffix)
			ostr << layout.pairChars.suffix;

		return ostr;
	}



	/// Convenience: if sequence type (array, list, set, map) not given, assume array.
	template <class T>
	static
	std::ostream & sequenceToStream(std::ostream & ostr, const T & x, const SprinterLayout & layout){
		return sequenceToStream(ostr, x, layout.arrayChars, layout);
	}


	/// Print sequence x with myChars, continue recursively with layout.
	template <class T>
	static
	std::ostream & sequenceToStream(std::ostream & ostr, const T & x, const TypeLayout & myChars, const SprinterLayout & layout){

		if (myChars.prefix)
			ostr << myChars.prefix;

		char sep = 0;
		for (typename T::const_iterator it=x.begin(); it != x.end(); ++it){
			//ostr << '[' << *it << ']';
			if (sep)
				ostr << sep;
			else
				sep = myChars.separator;

			toStream(ostr, *it, layout);
		}

		if (myChars.suffix)
			ostr << myChars.suffix;

		return ostr;

	}

	/// Given a sequence or subsequence of keys, output values of a map in that order.
	/**
	 *
	 *  \tparam M - map type
	 *  \tparam K - an iterable sequence of keys
	 *
	 *  Note: sequenceToStream is faster - recommended if all the entries are printed in native map order.
	 */
	template <class M, class K>
	static
	std::ostream & mapToStream(std::ostream & ostr, const M & m, const SprinterLayout & layout, const K & keys){

		if (layout.mapChars.prefix)
			ostr << layout.mapChars.prefix;

		char sep = 0; // for (const typename K::value_type & key =
		for (typename K::const_iterator it=keys.begin(); it != keys.end(); ++it){
			if (sep)
				ostr << sep;
			else
				sep = layout.mapChars.separator;

			typename M::const_iterator mit = m.find(*it);
			if (mit != m.end()){
				toStream(ostr, *mit, layout);
			}
			else {
				std::cerr << __FILE__ << __FUNCTION__ << ':' << "missing key: " << *it << std::endl;
			}

		}

		if (layout.mapChars.suffix)
			ostr << layout.mapChars.suffix;

		return ostr;

	}


	/// Routine for non-sequence types that may like prefix and suffix, anyway.
	template <class T>
	static
	std::ostream & basicToStream(std::ostream & ostr, const T & x, const TypeLayout & myChars){
		if (myChars.prefix)
			ostr << myChars.prefix;
		ostr << x;
		if (myChars.suffix)
			ostr << myChars.suffix;
		return ostr;
	}

	// std::ostream & mapToStream(std::ostream & ostr, const T & x){

	template <class K, class V>
	static
	std::ostream & toStream(std::ostream & ostr, const std::pair<K,V> & x, const SprinterLayout & layout = defaultLayout){
		return pairToStream(ostr, x, layout);
	}

	template <class D>
	static
	std::ostream & toStream(std::ostream & ostr, const std::vector<D> & x, const SprinterLayout & layout = defaultLayout){
		return sequenceToStream(ostr, x, layout.arrayChars, layout);
	}

	template <class D>
	static
	std::ostream & toStream(std::ostream & ostr, const std::list<D> & x, const SprinterLayout & layout = defaultLayout){
		return sequenceToStream(ostr, x, layout.arrayChars, layout);
	}

	template <class D>
	static
	std::ostream & toStream(std::ostream & ostr, const std::set<D> & x, const SprinterLayout & layout = defaultLayout){
		return sequenceToStream(ostr, x, layout.arrayChars, layout);
	}

	template <class K, class V>
	static
	std::ostream & toStream(std::ostream & ostr, const std::map<K,V> & x, const SprinterLayout & layout = defaultLayout){
		//return mapToStream(ostr, x);
		return sequenceToStream(ostr, x, layout.mapChars, layout);
	}

	/// Conventional type: std::string. Notice prefix revealed.
	static
	std::ostream & toStream(std::ostream & ostr, const std::string & x, const SprinterLayout & layout = defaultLayout) {
		return basicToStream(ostr, x, layout.stringChars);
		//return ostr << layout.stringChars.prefix << x << layout.stringChars.suffix;
	}

	/// Conventional type: C string.
	static
	std::ostream & toStream(std::ostream & ostr, const char *x, const SprinterLayout & layout = defaultLayout) {
		return basicToStream(ostr, x, layout.stringChars);
		//return ostr << layout.stringChars.prefix << x << layout.stringChars.suffix;
	}

	/// Single char gets styled same way as strings
	static
	std::ostream & toStream(std::ostream & ostr, char c, const SprinterLayout & layout = defaultLayout) { // short int?
		return basicToStream(ostr, c, layout.stringChars);
	}

	/// Pointer: redirect to actual target object
	template <class D>
	static
	std::ostream & toStream(std::ostream & ostr, D *x, const SprinterLayout & layout = defaultLayout) {
		return ostr << *x;
	}

	/// Pointer: redirect to actual target object
	template <class D>
	static
	std::ostream & toStream(std::ostream & ostr, const D *x, const SprinterLayout & layout = defaultLayout) {
		return ostr << *x;
	}

	/// Default, unformatted output for basetypes and types not matching the other templates.
	template <class D>
	static
	std::ostream & toStream(std::ostream & ostr, const D &x, const SprinterLayout & layout = defaultLayout) {
		// Layout not used
		return ostr << x;
	}


};

// MOVE to json files const SprinterLayout SprinterBase::layoutJSON;


/**
 *   Examples
 */
template <class T>
class Sprinter : public SprinterBase {

public:

	const T & src;
	SprinterLayout layout;

	Sprinter(const T & x, const SprinterLayout & layout) : src(x), layout(layout) {
	}
	// Dangerous? Sprinter(const Sprinter<T> & x) : src(x.src){}

	std::ostream & toStream(std::ostream & ostr) const {
		return SprinterBase::toStream(ostr, src, layout);
	}

	std::string str() const {
		std::stringstream sstr;
		SprinterBase::toStream(sstr, src, layout);
		return sstr.str();
	}



};


template <class T>
inline
std::ostream & operator<<(std::ostream & ostr, const Sprinter<T> & sp){
	return sp.toStream(ostr);
}


// Short cut (constructor wrappers)

template <class T>
inline
Sprinter<T> sprinter(const T & x, const char *arrayCaps, const char *mapCaps="{,}", const char *pairCaps="(:)", const char *stringCaps="\""){
	//SprinterLayout layout(arrayCaps,mapCaps,pairCaps,stringCaps);
	return Sprinter<T>(x, SprinterLayout(arrayCaps,mapCaps,pairCaps,stringCaps)); // copy const
}

template <class T>
inline
Sprinter<T> sprinter(const T & x, const SprinterLayout & layout = SprinterLayout()){
	return Sprinter<T>(x, layout); // copy const
}




}// Drain


#endif

