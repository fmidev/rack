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
#ifndef DRAIN_SPRINTER
#define DRAIN_SPRINTER

#include <string.h> // strlen
#include <iostream>
#include <stdexcept>

#include <string>
#include <utility> // pair
#include <vector>
#include <set>
#include <list>
#include <map>

#include <drain/UniTuple.h>



namespace drain {


/// Small container for printing style for putting of structured objects (array, maps, pairs).
/**
 *  Designed for recursive traversal or structures (like JSON trees).
 *
 *  Notice that set() derived from UniTuple<char,3> has argument order: prefix, separator, suffix
 */
class TypeLayoutBase : public UniTuple<char,3>{

public:

	typedef char cstr_t;
	cstr_t & prefix;
	cstr_t & separator;
	cstr_t & suffix;

	inline
	TypeLayoutBase(): // chars('\0',3), prefix(chars[0]), separator(chars[1]), suffix(chars[2]){
		prefix(this->next()), separator(this->next()), suffix(this->next()){
	}

	inline
	bool empty() const {  // conside rename isDefined(), esp. if functions used later ?
		return ((prefix==0) && (separator==0) && (suffix==0));
	}


};

struct TypeLayout : public TypeLayoutBase{


	/// Constructor for simple layout using a separator but no parentheses/braces.
	TypeLayout(cstr_t separator=','){
		set(0, separator, 0);
	}

	/// Constructor
	/***
	 *    Notice that arguments (a,b) invoke set(a,0,b), not set(a,b).
	 */
	TypeLayout(cstr_t prefix, cstr_t suffix){
		set(prefix, 0, suffix);
	}

	/// Constructor.
	TypeLayout(cstr_t prefix, cstr_t separator, cstr_t suffix){
		set(prefix, separator, suffix);
	}

	/// Constructor accepting three-letter chars: {prefix,separator,suffix}
	TypeLayout(const char layout[4]){ //  = "{,}"
		setLayout(layout);
	}

	TypeLayout(const TypeLayout & layout){
		this->assignSequence(layout);
		//this->assign(layout); // NOTE: copying element by element, not involving strings possibly containing null char (premature end-or-read).
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


/**

   // array,map,pair,string,key, mapPair

    Consider printing a map structure
    - as a map:  {key:value, key2:value2, ... }
    - as a list or pairs: [(key,value), (key2,value2), ...]

*/

struct SprinterLayout{

	// char itemSeparator = ',';  // consider as equal sign:  KEY:VALUE
	TypeLayout arrayChars   = TypeLayout("[,]");
	TypeLayout mapChars     = TypeLayout("{,}");
	TypeLayout pairChars    = TypeLayout("(,)"); // layout for (key,value), see keyChars
	TypeLayout stringChars  = TypeLayout('"',0,'"'); // TODO: Separate value and sequence layouts?
	TypeLayout keyChars     = TypeLayout(0,0,0); // given a pair = (key,value), defined how the key is .
	TypeLayout mapPairChars = TypeLayout(0,0,0); // When a map {entry, entry,... } is rendered, defines how

	// TypeLayout stringChars = TypeLayout("\"\"");
	// std::string boolTrue  = "true";
	// std::string boolFalse = "false";
	// std::string nullValue = "null";

	SprinterLayout(const char *arrayChars="[,]", const char *mapChars="{,}", const char *pairChars="(,)",
			const char *stringChars=nullptr, const char *keyChars=nullptr, const char *mapPairChars=nullptr){
		this->arrayChars.setLayout(arrayChars);
		this->mapChars.setLayout(mapChars);
		this->pairChars.setLayout(pairChars);
		if (stringChars)
			this->stringChars.setLayout(stringChars);
		if (keyChars)
			this->keyChars.setLayout(keyChars);
		if (mapPairChars)
			this->mapPairChars.setLayout(mapPairChars);
	}

	SprinterLayout(const SprinterLayout &layout):
		arrayChars(layout.arrayChars), mapChars(layout.mapChars), pairChars(layout.pairChars),
		stringChars(layout.stringChars), keyChars(layout.keyChars), mapPairChars(layout.mapPairChars){
	}


	SprinterLayout(char itemSeparator){ // ','
		arrayChars.separator = itemSeparator;
		mapChars.separator = itemSeparator;
		pairChars.separator = itemSeparator;
		mapPairChars.separator = itemSeparator;
	}



};

// Mainly for debugging
inline
std::ostream & operator<<(std::ostream & ostr, const SprinterLayout & layout){
	ostr << "arrayChars:   " << layout.arrayChars << '\n';
	ostr << "mapChars:     " << layout.mapChars << '\n';
	ostr << "pairChars:    " << layout.pairChars << '\n';
	ostr << "stringChars:  " << layout.stringChars << '\n';
	ostr << "keyChars:     " << layout.keyChars << '\n';
	ostr << "mapPairChars: " << layout.mapPairChars << '\n';
	return ostr;
}


class Sprinter {

public:

	/// Marker for unset layout
	static const SprinterLayout UNSET_LAYOUT;

	/// Displays objects with {...}, arrays with [...], pairs with (,) and strings without hyphens.
	static const SprinterLayout defaultLayout;

	/// Simply concatenate values without punctuation.
	static const SprinterLayout emptyLayout;

	/// Display plain values, concatenating them with comma (except for strings).
	static const SprinterLayout plainLayout;

	/// Put each array and object element on a separate line
	static const SprinterLayout lineLayout;

	/// Resembles JSON structure: {"a":1,"b":22,"c":3}
	/**
	 *   # arrays  as [value, ...]
	 *   # objects as {...}
	 *   # pairs   as {key:value}
	 *   # strings with double quotes: "..."
	 */
	static const SprinterLayout jsonLayout;

	/// JavaScript layout. Like JSON layout, but keys without hyphens.
	static const SprinterLayout jsLayout;

	/// Like attributes in XML (HTML, SVG, ...) tags
	static const SprinterLayout xmlAttributeLayout;

	/// C++ code initializer list style: all objects with {...}, with comma ',' separator.
	/**
	 *   # arrays  as {value, ...}
	 *   # objects as {}
	 *   # pairs   as {key,value}
	 *   # strings with double quotes: "..."
	 */
	static const SprinterLayout cppLayout;

	/// C++ code initializer list style: all objects with {...}, with comma ',' separator.
	/**
	 *   # arrays  with [value, ...]
	 *   # objects with {}
	 *   # pairs   with (key,value), like tuples (of two elements)
	 *   # strings with single quotes: '...'
	 */
	static const SprinterLayout pythonLayout;

	/// Simulates how arguments are given to command line options.
	static const SprinterLayout cmdLineLayout; // = {":", ",", "=", ""};

	/// Output Dictionary, which is a list
	// static const SprinterLayout dictLayout; // = {",", "?", "=", ""};

	static inline
	void prefixToStream(std::ostream & ostr, const TypeLayout & layout){
		if (layout.prefix)
			ostr << layout.prefix;
	};

	static inline
	void separatorToStream(std::ostream & ostr, const TypeLayout & layout){
		if (layout.separator)
			ostr << layout.separator;
	};

	static inline
	void suffixToStream(std::ostream & ostr, const TypeLayout & layout){
		if (layout.suffix)
			ostr << layout.suffix;
	};


	// Complicated, but universal...
	template <class K, class V>
	static
	std::ostream & pairToStream(std::ostream & ostr, const std::pair<K,V> & x, const SprinterLayout & layout){

		prefixToStream(ostr, layout.pairChars);

		// EXPERIMENTAL
		prefixToStream(ostr, layout.keyChars);
		//toStream(ostr, x.first, layout);
		ostr << x.first;
		suffixToStream(ostr, layout.keyChars);

		separatorToStream(ostr, layout.pairChars);

		toStream(ostr, x.second, layout);

		suffixToStream(ostr, layout.pairChars);

		return ostr;
	}

	/// If (key,value) pairs needs specific layout in maps, use this. (Compare with list of tuples).
	template <class K, class V>
	static
	std::ostream & mapPairToStream(std::ostream & ostr, const std::pair<K,V> & x, const SprinterLayout & layout){

		prefixToStream(ostr, layout.mapPairChars);

		// EXPERIMENTAL
		prefixToStream(ostr, layout.keyChars);
		//toStream(ostr, x.first, layout);
		ostr << x.first;
		suffixToStream(ostr, layout.keyChars);

		separatorToStream(ostr, layout.mapPairChars);

		toStream(ostr, x.second, layout); // recursion

		suffixToStream(ostr, layout.mapPairChars);

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

		prefixToStream(ostr, myChars);

		bool first = true;
		for (const auto & elem: x){

			if (first)
				first = false;
			else
				separatorToStream(ostr, myChars);

			toStream(ostr, elem, layout);
		}

		suffixToStream(ostr, myChars);

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

		prefixToStream(ostr, layout.mapChars); // redesign: layout.startMap(ostr)
		//if (layout.mapChars.prefix)
		//	ostr << layout.mapChars.prefix;

		const bool MAP_PAIRS = !layout.mapPairChars.empty();

		// char sep = 0; // for (const typename K::value_type & key =
		for (typename K::const_iterator it=keys.begin(); it != keys.end(); ++it){

			if (it != keys.begin())
				separatorToStream(ostr, layout.mapChars);

			typename M::const_iterator mit = m.find(*it);
			if (mit != m.end()){
				if (MAP_PAIRS){
					mapPairToStream(ostr, *mit, layout);
				}
				else {
					toStream(ostr, *mit, layout);
				}
			}
			else {
				std::cerr << __FILE__ << __FUNCTION__ << ':' << "missing key: " << *it << std::endl;
			}

		}

		suffixToStream(ostr, layout.mapChars); // redesign: layout.endMap(ostr) or layout.mapLayout.putStart(ostr); .putEnd(ostr);

		return ostr;

	}


	/// Write drain::Tree's or any trees that have tree::data[] member.
	//template <class T, bool EXCL>
	//static
	//std::ostream & treeToStream(std::ostream & ostr, const Tree<T,EXCL> & tree, const drain::SprinterLayout & layout, short indent){
	template <class T>
	static
	std::ostream & treeToStream(std::ostream & ostr, const T & tree, const drain::SprinterLayout & layout, short indent=0);


	/// Routine for non-sequence types that may like prefix and suffix, anyway.
	template <class T>
	static
	std::ostream & basicToStream(std::ostream & ostr, const T & x, const TypeLayout & myChars){
		prefixToStream(ostr, myChars);
		ostr << x;
		suffixToStream(ostr, myChars);
		return ostr;
	}

	// std::ostream & mapToStream(std::ostream & ostr, const T & x){

	/// New (experimental)
	template <class T>
	static
	std::ostream & toStream(std::ostream & ostr, const std::initializer_list<T> & x, const SprinterLayout & layout = defaultLayout){
		//ostr << x.size() << '@';
		return sequenceToStream(ostr, x, layout.arrayChars, layout);
	}

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
	std::ostream & toStream(std::ostream & ostr, const std::map<K,V> & x, const SprinterLayout & layout = UNSET_LAYOUT){

		if (&layout == &UNSET_LAYOUT){
			return sequenceToStream(ostr, x, jsonLayout.mapChars, jsonLayout);
		}
		else {
			return sequenceToStream(ostr, x, layout.mapChars, layout);
		}
		//return mapToStream(ostr, x);
	}

	template <class D, size_t N>
	static inline
	std::ostream & toStream(std::ostream & ostr, const drain::UniTuple<D,N> & x, const SprinterLayout & layout = defaultLayout){
		return sequenceToStream(ostr, x, layout.arrayChars, layout);
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

	/// Single char gets styled same way as strings
	static
	std::ostream & toStream(std::ostream & ostr, bool b, const SprinterLayout & layout = defaultLayout) { // short int?
		//return basicToStream(ostr, b?"true":"false", layout.stringChars);
		return ostr << (b ? "true":"false");
	}


	/// Pointer: redirect to actual target object
	template <class D>
	static
	std::ostream & toStream(std::ostream & ostr, D *x, const SprinterLayout & layout = defaultLayout) {
		if (x == nullptr)
			return ostr << "null";
		else if (x == 0)
			return ostr << "null";
		else
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



template <class T>
std::ostream & Sprinter::treeToStream(std::ostream & ostr, const T & tree, const drain::SprinterLayout & layout, short indent){
// template <class T, bool EXCL>//
// std::ostream & Sprinter::treeToStream(std::ostream & ostr, const Tree<T,EXCL> & tree, const drain::SprinterLayout & layout, short indent){

	const bool DATA     = !tree.data.empty();
	const bool CHILDREN = !tree.empty();

	if (! (DATA||CHILDREN)){
		// Also empty element should return something, here {}, but could be "" or null ?
		ostr << layout.mapChars.prefix << layout.mapChars.suffix; // '\n';
		return ostr;
	}

	const std::string pad(2*indent, ' ');


	if (DATA){
		drain::Sprinter::toStream(ostr, tree.data, layout);
		//return ostr;
		/*
		char sep = 0;
		for (const auto & entry: tree.data){
			if (sep){
				ostr << sep;
				ostr << '\n';
			}
			else {
				sep = layout.mapChars.separator;
			}
			ostr << pad << "  " << '"' << entry.first << '"' << layout.pairChars.separator << ' ';
			drain::Sprinter::toStream(ostr, entry.second, layout);
		}
		*/
		if (CHILDREN)
			ostr << layout.mapChars.separator;
		ostr << '\n';
	}

	ostr << layout.mapChars.prefix << '\n';

	if (CHILDREN){
		char sep = 0;
		for (const auto & entry: tree){
			if (sep){
				ostr << sep;
				ostr << '\n';
			}
			else {
				sep = layout.mapChars.separator;
			}
			ostr << pad << "  " << '"' << entry.first << '"' <<  layout.pairChars.separator << ' ';  // if empty?
			treeToStream(ostr, entry.second, layout, indent+1); // recursion
		}
		ostr << '\n';
	}

	ostr << pad << layout.mapChars.suffix; //  << '\n';

	return ostr;
}


// MOVE to json files const SprinterLayout Sprinter::layoutJSON;

///
/**
 *   Examples.
 *
 *   \tparam T - basetype variable or object to be printed
 *   \tparam L - default layout; allows creating typedefs with default layouts
 *
 *   Templated class, because references an external object of any type.
 */
template <class T, class L=SprinterLayout>
class Sprintlet : public Sprinter {

public:

	const T & src;
	SprinterLayout layout;

	Sprintlet(const T & x, const SprinterLayout & layout = L()) : src(x), layout(layout) {
	}
	// Dangerous? Sprinter(const Sprinter<T> & x) : src(x.src){}

	std::ostream & toStream(std::ostream & ostr) const {
		return Sprinter::toStream(ostr, src, layout);
	}

	std::string str() const {
		std::stringstream sstr;
		Sprinter::toStream(sstr, src, layout);
		return sstr.str();
	}

};


template <class T, class L>
inline
std::ostream & operator<<(std::ostream & ostr, const Sprintlet<T,L> & sp){
	return sp.toStream(ostr);
}


// Short cut (constructor wrappers)

template <class T>
inline
Sprintlet<T> sprinter(const T & x, const char *arrayCaps, const char *mapCaps="{,}", const char *pairCaps="(:)", const char *stringCaps="\""){
	return Sprintlet<T>(x, SprinterLayout(arrayCaps,mapCaps,pairCaps,stringCaps)); // copy const
}

template <class T>
inline
Sprintlet<T> sprinter(const T & x, const SprinterLayout & layout = SprinterLayout()){
	return Sprintlet<T>(x, layout); // copy const
}


} // drain


#endif

