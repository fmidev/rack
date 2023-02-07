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
#ifndef DRAIN_JSON_READER_H_
#define DRAIN_JSON_READER_H_

#include <string>

#include "Sprinter.h"
#include "TextReader.h"
#include "Variable.h"

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
 *  For writing, the new SprinterBase::toStream is preferred to JSONwriter, which is deprecating.
 *
 */
class JSON  {

public:

	/// NEW. Reads and parses a JSON file
	/**
	 *  \tparam - tree type, especially drain::Tree<K,V>
	 */
	template <class T>
	static
	void readTree(T & tree, std::istream & istr);


	template <class T>
	static
	std::ostream & treeToStream(std::ostream & ostr, const T & tree, const drain::SprinterLayout & layout = drain::SprinterBase::jsonLayout, short indent=0);


	/// Read value. Read stream until a value has been extracted, with type recognition
	static
	void readValue(std::istream & istr, Variable & v, bool keepType = false);

	/// Read value. Read stream until a value has been extracted, with type recognition
	static inline
	void readValue(const std::string & s, Variable & v, bool keepType = false){
		std::istringstream istr(s);
		readValue(istr, v, keepType);
	};

	/// Given comma-separated string of values, assign them to variable of minimum compatible type
	static
	void readArray(const std::string & s, Variable & v);

protected:

	///  In reading trees.
	template <class T>
	static
	void handleValue(std::istream & istr, T & tree, const std::string & key){
		drain::Logger log(__FILE__, __FUNCTION__);
		log.unimplemented("type:", typeid(T).name());
	}


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
		drain::SprinterBase::toStream(ostr, tree.data, layout);
		return ostr; // exclusive
		/*
		if (CHILDREN)
			ostr << layout.mapChars.separator;
		ostr << '\n';
		*/
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

template <class T>
void JSON::readTree(T & tree, std::istream & istr){

	drain::Logger log(__FUNCTION__, __FILE__);


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




}  // drain

#endif /* META_DATA_H_*/
