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
#ifndef DRAIN_JSON_WRITER
#define DRAIN_JSON_WRITER
#include <typeinfo>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <limits> // for numeric type recognition

//#include "../util/Tree.h"

namespace drain
{

/// A partial implementation of JSON. Basic types
/**
 *
 *   Supports
 *   - nesting objects {... {... } }
 *   - integers (int), floats (double)
 *
 *   Does not support:
 *   - arrays of arrays, arrays of objects
 *   - boolean

    \example examples/JSON-example.inc

 */


class JSONwriter {

public:


	template <class T>
	static
	std::ostream & toStream(const T & x, std::ostream &ostr = std::cout, unsigned short indentation = 0){
		//const std::type_info & t = typeid(T);integer) || (std::numeric_limits<T>::is_float) || (typeid(T) == typeid(bool
		if ((std::numeric_limits<T>::is_specialized))
			return ostr << x;
		else
			return ostr << '"' << x << '"';
	}

	template <class T>
	static
	std::ostream & toStream(const T * x, std::ostream &ostr = std::cout, unsigned short indentation = 0){
		return ostr << '?' << x;
	}


	template <class T>
	static
	inline
	std::ostream & toStream(const std::vector<T> & x, std::ostream &ostr = std::cout, unsigned short indentation = 0){
		return sequenceToStream(x, ostr); // below
	}

	template <class K, class V>
	static
	inline
	std::ostream & toStream(const std::map<K,V> & m, std::ostream &ostr = std::cout, unsigned short indentation = 0){
		return mapToStream(m, ostr, indentation); // below
	}

	// CONTRADICTORY: metadata V ("attributes") should/must be a map?
	// See: drain::JSONtree::tree_t  implementation instead.
	/*
	template <class V>
	static
	std::ostream & toStream(const drain::Tree<std::string,V> & t, std::ostream &ostr = std::cout, unsigned short indentation = 0);
	*/

	/// For forcing (avoiding recursions)
	// Needed?
	template <class T>
	static inline
	std::ostream & plainToStream(const T & x, std::ostream &ostr = std::cout){
		return ostr << x;
	}

	/// Implementation for std::vector<>,  std::vector<list> and  std::vector<set>
	template <class T>
	static
	std::ostream & sequenceToStream(const T & x, std::ostream &ostr = std::cout);

	/// Like sequenceToStream() , but skipping elements of empty() == true.
	/*
	 *  For example, elements of type std::string or std::map implement empty().
	 */
	template <class T>
	static
	std::ostream & sparseSequenceToStream(const T & x, std::ostream &ostr = std::cout);

	/// Implementation for std::map<> and derived classes
	template <class T>
	static
	std::ostream & mapElementsToStream(const T & m, std::ostream &ostr = std::cout, unsigned short indentation = 0);

	/// Implementation for std::map<> and derived classes
	template <class T>
	static
	std::ostream & mapToStream(const T & m, std::ostream &ostr = std::cout, unsigned short indentation = 0);


	static
	unsigned short indentStep;


protected:

	/// Indent output with \c n spaces
	static inline
	void indent(std::ostream & ostr, unsigned short n){
		for (int i = 0; i < n; ++i)
			ostr.put(' ');
	}


};








template <>
std::ostream & JSONwriter::toStream(const std::string & x, std::ostream &ostr, unsigned short indentation);


// Controversial. But use (int)x for example to make it a number.
template <>
std::ostream & JSONwriter::toStream(const char & x, std::ostream &ostr, unsigned short indentation);

template <>
std::ostream & JSONwriter::toStream(const unsigned char & x, std::ostream &ostr, unsigned short indentation);


template <>
std::ostream & JSONwriter::toStream(const char *x, std::ostream &ostr, unsigned short indentation);





template <class T>
std::ostream & JSONwriter::sequenceToStream(const T & x, std::ostream &ostr){

	char sep = 0;

	ostr << '[';

	for (typename T::const_iterator it = x.begin(); it != x.end(); ++it){
		if (sep)
			ostr << sep << ' ';
		else
			sep = ',';
		/// Recursion
		toStream(*it, ostr); // If elements are vectors, restart...
	}

	ostr << ']';

	return ostr;
}

template <class T>
std::ostream & JSONwriter::sparseSequenceToStream(const T & x, std::ostream &ostr){

	char sep = 0;

	ostr << '[';

	for (typename T::const_iterator it = x.begin(); it != x.end(); ++it){
		//if (it->empty()){
		if (!T::template empty(*it)){
			if (sep)
				ostr << sep << ' ';
			else
				sep = ',';
			/// Recursion
			toStream(*it, ostr);
		}
	}

	ostr << ']';

	return ostr;
}



template <class T>
std::ostream & JSONwriter::mapElementsToStream(const T & m, std::ostream &ostr, unsigned short indentation){

	indentation += JSONwriter::indentStep;

	char sep = 0;

	for (typename T::const_iterator it = m.begin(); it != m.end(); ++it){

		if (sep){
			ostr << sep << '\n';
		}
		else
			sep = ',';

		JSONwriter::indent(ostr, indentation);
		ostr << '"' << it->first << '"' << ':' << ' ';
		/// Recursion
		toStream(it->second, ostr, indentation); // If elements are vectors, restart...

	}

	return ostr;
}

template <class T>
std::ostream & JSONwriter::mapToStream(const T & m, std::ostream &ostr, unsigned short indentation){
	ostr << '{' << '\n';
	JSONwriter::mapElementsToStream(m, ostr, indentation);
	JSONwriter::indent(ostr, indentation);
	ostr  << '\n' << '}';
	return ostr;
}


/*
template <class V>
std::ostream & JSONwriter::toStream(const drain::Tree<std::string,V> & t, std::ostream &ostr, unsigned short indentation){

	const bool ATTRIBS  = !t.data.empty();
	const bool CHILDREN = !t.isEmpty();

	ostr << '{';

	if (ATTRIBS){
		ostr << '\n';
		// CONTRADICTORY: metadata ("attributes") should/must be a map
		JSONwriter::indent(ostr, indentation+2);
		ostr << '"' << "data" << '"' << ": ";
		JSONwriter::toStream(t.data, ostr, indentation);
		// JSONwriter::mapElementsToStream(t.data, ostr, indentation);
	}

	if (ATTRIBS && CHILDREN){
		//JSONwriter::indent(ostr, indentation+2);
		ostr << ',';
	}

	if (CHILDREN)
		ostr << '\n';

	JSONwriter::mapElementsToStream(t, ostr, indentation);

	if (ATTRIBS || CHILDREN){
		ostr << '\n';
		JSONwriter::indent(ostr, indentation);
		//ostr << '';
	}

	ostr << '}'; // << '\n';

	return ostr;
}
*/






} // ::drain

#endif
