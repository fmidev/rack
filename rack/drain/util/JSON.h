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
#ifndef DRAIN_JSON_H_
#define DRAIN_JSON_H_

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <list>
#include <map>

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


class JSON {

public:


	template <class T>
	static
	std::ostream & toStream(const T & x, std::ostream &ostr = std::cout){
		return ostr << x;
	}

	template <class T>
	static
	std::ostream & toStream(const T * x, std::ostream &ostr = std::cout){
		return ostr << '?' << x;
	}


	template <class T>
	static
	std::ostream & toStream(const std::vector<T> & x, std::ostream &ostr = std::cout){
		return sequenceToStream(x, ostr); // below
	}

	template <class K, class V>
	static
	std::ostream & toStream(const std::map<K,V> & x, std::ostream &ostr = std::cout);


	/// Implementation for std::vector<>,  std::vector<list> and  std::vector<set>
	template <class T>
	static
	std::ostream & sequenceToStream(const T & x, std::ostream &ostr = std::cout);

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

template <class T>
std::ostream & JSON::sequenceToStream(const T & x, std::ostream &ostr){
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


template <class K, class V>
std::ostream & JSON::toStream(const std::map<K,V> & x, std::ostream &ostr){
	char sep = 0;
	ostr << '{';
	for (typename std::map<K,V>::const_iterator it = x.begin(); it != x.end(); ++it){
		if (sep)
			ostr << sep << '\n';
		else
			sep = ',';
		ostr << '"' << it->first << '"' << ':' << ' ';
		/// Recursion
		toStream(it->second, ostr); // If elements are vectors, restart...
	}
	ostr << '}';
	return ostr;
}



template <>
std::ostream & JSON::toStream(const std::string & x, std::ostream &ostr);


// Controversial. But use (int)x for example to make it a number.
template <>
std::ostream & JSON::toStream(const char & x, std::ostream &ostr);

template <>
std::ostream & JSON::toStream(const unsigned char & x, std::ostream &ostr);


template <>
std::ostream & JSON::toStream(const char *x, std::ostream &ostr);





} // ::drain

#endif
