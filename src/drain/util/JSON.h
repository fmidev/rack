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

#include <string>

#include <drain/Sprinter.h>
#include <drain/VariableAssign.h>

#include "FileInfo.h"
#include "TextReader.h"
#include "TreeUnordered.h"

// #include "TreeOrdered.h" // consider?

namespace drain
{

/// Utility for extracting JSON-compatible strings, numbers or arrays of numbers in text data
/*'
 *  Applicable in reading configuration files and comments containing:
 *  -# plain numbers, distinguishing integers and floats
 *	-# arrays of numbers, surrounded by braces [,]
 *	-# strings, surrounded by double hyphens (")
 *
 *	Applies TextReader::scanSegment in reading character streams.
 *	Uses Type::guessArrayType() for deriving compatible storage type for arrays.
 *
 *  For writing, the new Sprinter::toStream is preferred to JSONwriter, which is deprecating.
 *
 */
class JSON  {

public:


	static FileInfo fileInfo;

	/// Reads and parses a JSON file
	/**
	 *  \tparam - tree type, especially drain::Tree<K,V>
	 */
	template <class T>
	static
	void readTree(T & tree, std::istream & istr);

	/// Write a tree into an output stream using JSON layout by default.
	/**
	 *   SprinterLayout offers flexibility in formatting. JSON layout is the default.
	 */
	template <class T>
	static
	std::ostream & treeToStream(std::ostream & ostr, const T & tree, const drain::SprinterLayout & layout = drain::Sprinter::jsonLayout, short indent=0);


	/// Read a value (JSON syntax). Read stream until a value has been extracted, with type recognition
	static
	void readValue(std::istream & istr, Variable & v, bool keepType = false);

	/// Read value (JSON syntax). Read stream until a value has been extracted, with type recognition
	/**
	 *   Allowed syntax:
	 *   - string: "<value>"
	 *   - scalar:  <value>
	 *   - array:  [<value>,<value>,<value>,<...>]
	 */
	static
	void readValue(const std::string & s, Variable & v, bool keepType = false);

	/// Given comma-separated string of values, assign them to variable of minimum compatible type
	static
	void readArray(const std::string & s, Variable & v);

protected:

	///  In reading trees.
	template <class T>
	static
	void handleValue(std::istream & istr, T & tree, const std::string & key){  // tree + key VariableMap!  (as long as Palette (or others) use JSONtree with that)
		drain::Logger log(__FILE__, __FUNCTION__);
		log.unimplemented("type:", typeid(T).name());
		log.error("stop..");
	}
	// To be later replaced with:
	// void handleValue(std::istream & istr, T & child);


};

template <class T>
std::ostream & JSON::treeToStream(std::ostream & ostr, const T & tree, const drain::SprinterLayout & layout, short indent){

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
		//ostr << layout.pairChars.suffix;
		return ostr; // = pratically exclusive
		/*
		if (CHILDREN)
			ostr << layout.mapChars.separator;
		ostr << '\n';
		*/
	}

	ostr << layout.mapChars.prefix; // << '\n';
	//ostr << layout.pairChars.prefix;
	ostr << '\n';

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
			//ostr << pad << "  " << '"' << entry.first << '"' <<  layout.pairChars.separator << ' ';  // if empty?
			ostr << pad << "  ";  // if empty?
			// if (layout.pairChars.prefix)
			ostr << layout.pairChars.prefix;
			ostr << '"' << entry.first << '"' <<  layout.pairChars.separator << ' ';  // if empty?
			treeToStream(ostr, entry.second, layout, indent+1); // recursion
			// if (layout.pairChars.suffix)
			ostr << layout.pairChars.suffix;
		}
		ostr << '\n';
	}

	// if (layout.mapChars.suffix)
	ostr << pad << layout.mapChars.suffix; // << '\n';

	return ostr;
}

template <class T>
void JSON::readTree(T & tree, std::istream & istr){

	drain::Logger log(__FILE__, __FUNCTION__);


	if (!istr){
		log.error("File read error");
		return;
	}

	char c;

	TextReader::skipWhiteSpace(istr);
	c = istr.get();
	if (c != '{'){
		log.error("Syntax error: read '", c, "' when expecting '{'");
		return;
	}

	std::string key;
	std::string value;

	// Debugging. Incomplete segments raise warnings.
	bool completed = true; //

	while (istr){

		TextReader::skipWhiteSpace(istr);

		if (!istr)
			return;

		c = istr.get();

		if (c == '"'){ // New key, starting new entry

			key = TextReader::scanSegment(istr, "\"");
			istr.get(); // swallow '"'
			TextReader::skipWhiteSpace(istr);
			c = istr.get();
			// log.warn() << " then4: " <<  (char)istr.peek() << log.endl;

			if (c == ':'){
				//handleValue(istr, tree, key);

				handleValue(istr, tree, key);
				// log.note(" - end: ", key);
				completed = true;
			}
			else {
				log.error() << "Syntax error: read \"" << key << "\" followed by '" << c << "' when expecting object {...}, string \"...\", array [...], or number" << log.endl;
				return;
			}
		}
		else if (c == '}'){
			if (!completed) // comma encountered after empty segment
				log.warn("empty section after key=", key);
			//log.warn("closing '}' for ", key);
			return;
		}
		else if (c == ','){
			//if (!completed) // comma encountered after empty segment
			//	log.warn("empty section after key=", key);
			completed = false; // trap for subsequent check
		}
		else { // TODO: warn if comma encountered after empty
			log.error("Syntax error: char '", c, "' (", (short)c, "), expected '\"', '}' or ','");
			return;
		}

	}

}


// New 2023 "implementation"
typedef drain::UnorderedMultiTree<drain::Variable,true> JSONtree2;

//template <>
//const JSONtree2 JSONtree2::dummy;

template <>
inline
void drain::JSON::handleValue(std::istream & istr, JSONtree2 & dst, const std::string & key){
//void drain::JSON::handleValue(std::istream & istr, JSONtree2 & child){

	drain::Logger log( __FILE__, __FUNCTION__);


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



template <>
inline
std::ostream & drain::Sprinter::toStream(std::ostream & ostr, const JSONtree2 & tree, const drain::SprinterLayout & layout){
	return drain::JSON::treeToStream(ostr, tree, layout);
	//return drain::Sprinter::treeToStream(ostr, tree, layout);
}


}  // drain

#endif /* META_DATA_H_*/
