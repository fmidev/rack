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
#ifndef JSON_TREE_H_
#define JSON_TREE_H_

#include <iostream>
#include <list>
#include <string>

#include "../util/Tree.h"
#include "../util/VariableMap.h"


namespace drain
{

/// A partial implementation of JSON. An object consists of attributes (numeric, string, array) and nesting objects.
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

	typedef drain::Tree<std::string, drain::VariableMap> tree_t;
	typedef tree_t::path_t path_t;
	typedef tree_t::node_t node_t;

	/// Write a JSON file
	static
	void write(const tree_t & t, std::ostream & ostr = std::cout, unsigned short indentation = 0);


	/// Reads and parses a JSON file
	static
	void read(tree_t & t, std::istream & istr);

	static
	unsigned short indentStep;

protected:

	// Write utils

	/// Indent output with \c n spaces
	static inline
	void indent(std::ostream & ostr, unsigned short n){
		for (int i = 0; i < n; ++i)
			ostr.put(' ');
	}

	// Read utils

	static
	void skipChars(std::istream & istr, const std::string skip = " \t\n\r");

	static
	std::string scanSegment(std::istream & istr, const std::string & terminator);


};


} // ::drain

#endif
