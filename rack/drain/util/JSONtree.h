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

#include "Tree.h"
#include "VariableMap.h"

#include "JSONwriter.h"
#include "JSON.h"

#include "TextReader.h"

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
class JSONtree {

public:

	/// The "native" drain::JSON tree, yet others can be used in templates.
	//typedef drain::Tree<std::string, drain::VariableMap> tree_t;
	typedef drain::Tree<drain::VariableMap> tree_t;

	/// Must be implement list concept, eg. derived from  std::list<T>
	typedef tree_t::path_t path_t;

	/// Path key type (child map key type)
	typedef tree_t::key_t key_t;

	/// Must contain attributes
	typedef tree_t::node_t node_t;


	static
	void write(const tree_t & json, const std::string & filename);

	/// Write a JSON file
	//  Note: must be implemented only after JSONwriter::toStream<>
	static // deprecated?
	void writeJSON(const tree_t & t, std::ostream & ostr = std::cout, unsigned short indentation = 0);


	/// Write a Windows INI file
	static
	void writeINI(const tree_t & t, std::ostream & ostr = std::cout, const tree_t::path_t & prefix = tree_t::path_t());


	/// Reads and parses a JSON file
	/**
	 *  \tparam - tree type, especially drain::Tree<K,V>
	 */
	template <class T>
	static
	void readTree(T & tree, std::istream & istr);



	/// Reads and parses a JSON file
	static
	void read(tree_t & t, std::istream & istr){
		readTree(t, istr);
	}


	/// Reads and parses a Windows INI file
	static
	void readINI(tree_t & t, std::istream & istr);

	//static unsigned short indentStep;

protected:

	/// Indent output with \c n spaces
	/*
	static inline
	void indent(std::ostream & ostr, unsigned short n){
		for (int i = 0; i < n; ++i)
			ostr.put(' ');
	}
	*/

};

template <>
std::ostream & JSONwriter::toStream(const drain::JSONtree::tree_t & t, std::ostream &ostr, unsigned short indentation);


template <class T>
void JSONtree::readTree(T & t, std::istream & istr){

	drain::Logger log("JSON", __FUNCTION__);

	if (!istr){
		log.error() << "File read error" << log.endl;
		return;
	}

	char c;

	TextReader::skipChars(istr, " \t\n\r");
	c = istr.get();
	if (c != '{'){
		//std::cerr << "Fail: " << c << '\n';
		log.error() << "Syntax error: read '" << c << "' when expecting '{'" << log.endl;
		return;
	}

	std::string key;
	std::string value;
	bool completed = false;

	while (istr){

		TextReader::skipChars(istr, " \t\n\r");

		c = istr.get();

		if (c == '"'){ // New entry

			key = TextReader::scanSegment(istr, "\"");
			// log.warn() << " then1: " <<  (char)istr.peek() << log.endl;
			//istr.get(); // swallow terminator
			// log.warn() << " then2: " <<  (char)istr.peek() << log.endl;

			typename T::node_t & vmap = t.data;

			TextReader::skipWhiteSpace(istr); // Chars(istr, " \t\n\r");
			// log.warn() << " then3: " <<  (char)istr.peek() << log.endl;
			c = istr.get();
			// log.warn() << " then4: " <<  (char)istr.peek() << log.endl;

			if (c == ':'){
				TextReader::skipWhiteSpace(istr) ; // Chars(istr, " \t\n\r");
				c = istr.peek();

				if (c == '{'){
					log.debug3() << "Reading object '" << key << "'" << log.endl;
					/// RECURSION
					JSONtree::readTree(t[key], istr);
				}
				else {
					log.debug3() << "Reading value '" << key << "'" << log.endl;
					JSONreader::readValue(istr, vmap[key]);
					//ValueReader::scanValue(istr, vmap[key]);
				}
				completed = true;
			}
			else {
				log.error() << "Syntax error: read \"" << key << "\" followed by '" << c << "' when expecting object {...}, string \"...\", array [...], or number" << log.endl;
				return;
			}
		}
		else if (c == '}')
			return;
		else if (c == ','){
			if (!completed) // comma encountered after empty segment
				log.warn() << "empty section after key=" << key << log.endl;
			completed = false; // trap for subsequent check
		}
		else { // TODO: warn if comma encountered after empty
			log.error() << "Syntax error: char '" << c << "', expected '\"', '}' or ','" << log.endl;
		}

	}

}


} // ::drain

#endif
