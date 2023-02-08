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

#include "FileInfo.h"
//#include "JSONwriter.h"
#include "JSON.h"
#include "TextReader.h"
#include "TreeUnordered.h"
#include "VariableMap.h"

namespace drain
{


// New 2023 "implementation"
typedef drain::UnorderedMultiTree<drain::Variable,true> JSONtree2;

//template <>
//const JSONtree2 JSONtree2::dummy;

template <>
inline
std::ostream & drain::SprinterBase::toStream(std::ostream & ostr, const JSONtree2 & tree, const drain::SprinterLayout & layout){
	return drain::JSON::treeToStream(ostr, tree, layout);
	//return drain::SprinterBase::treeToStream(ostr, tree, layout);
}

template <>
inline
void drain::JSON::handleValue(std::istream & istr, JSONtree2 & dst, const std::string & key){
//void drain::JSON::handleValue(std::istream & istr, JSONtree2 & child){

	drain::Logger log( __FUNCTION__, __FILE__);


	JSONtree2 & child = dst.addChild(key);

	TextReader::skipWhiteSpace(istr);

	char c = istr.peek();

	if (c == '{'){
		// log.warn("Reading object '", key, "'");
		//JSON::readTree(dst[key], istr); /// RECURSION
		JSON::readTree(child, istr); /// RECURSION
	}
	else {
		// log.warn("Reading value '", key, "'");
		//JSON::readValue(istr, dst[key].data);
		JSON::readValue(istr, child.data);
	}

	return;
}



/// A static class, with tree_t providing a partial implementation of JSON. An object consists of attributes (numeric, string, array) and nesting objects (excluding arrays).
/**
 *
 *   Usage:
 *   drain::JSONtree::tree_t tree;
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
	typedef drain::UnorderedMultiTree<drain::VariableMap> tree_t;

	/// Must be implement list concept, eg. derived from  std::list<T>
	typedef tree_t::path_t path_t;

	/// Path key type (child map key type)
	typedef tree_t::key_t key_t;

	/// Must contain attributes
	typedef tree_t::node_data_t node_data_t;

	static FileInfo fileInfo;


	static
	void write(const tree_t & json, const std::string & filename);

	/// Write a JSON file
	//  Note: must be implemented only after JSONwriter::toStream<>
	//static //inline // deprecated?
	// void writeJSON(const tree_t & t, std::ostream & ostr = std::cout, unsigned short indentation = 0);
		// TODO use: drain::SprinterBase::treeToStream(ostr, tree, layout);


	/// Write a Windows INI file
	// TODO: move to TreeUtils, Sprinter-like?
	static
	void writeINI(const tree_t & t, std::ostream & ostr = std::cout, const tree_t::path_t & prefix = tree_t::path_t());



	/// Reads and parses a JSON file
	static
	void read(tree_t & t, std::istream & istr){
		JSON::readTree(t, istr);
	}

	template <class T>
	static
	void handleValue(std::istream & istr, T & dst, const std::string & key = "");


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

// deprecating
/*
template <>
std::ostream & JSONwriter::toStream(const drain::JSONtree::tree_t & t, std::ostream &ostr, unsigned short indentation);
*/

// NEW (recommended)
/*
template <>
inline
std::ostream & drain::SprinterBase::toStream(std::ostream & ostr, const drain::JSONtree::tree_t & tree,
		const drain::SprinterLayout & layout){
	return drain::SprinterBase::treeToStream(ostr, tree, layout);
}
*/


template <>
inline
void JSON::handleValue(std::istream & istr, JSONtree::tree_t & dst, const std::string & key){

	drain::Logger log(__FILE__, __FUNCTION__);

	TextReader::skipWhiteSpace(istr) ;
	char c = istr.peek();

	if (c == '{'){
		//log.warn("Reading object '", key, "'");
		/// RECURSION
		JSON::readTree(dst[key], istr);
		TextReader::skipWhiteSpace(istr);
		c = istr.peek();
		//log.warn("Read object, next: '", c, "'");
	}
	else {
		log.debug3("Reading value '", key, "'");
		JSON::readValue(istr, dst.data[key]);
		//ValueReader::scanValue(istr, vmap[key]);
	}

	return;
}



} // ::drain

#endif
