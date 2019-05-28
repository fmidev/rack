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

#include <sstream>
#include <stdexcept>

#include "Log.h"
//#include "RegExp.h"
#include "Type.h"

#include "JSONtree.h"


namespace drain
{


/**
 *
 *  TODO: manual+includes, TypeUtils, Skip escaped \\
 *
 *
 */
unsigned short JSON::indentStep(2);

void JSON::read(tree_t & t, std::istream & istr){

	drain::Logger log("JSON", __FUNCTION__);

	if (!istr){
		log.error() << "File read error" << log.endl;
		return;
	}

	char c;

	JSON::skipChars(istr, " \t\n\r");
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

		JSON::skipChars(istr, " \t\n\r");

		c = istr.get();

		if (c == '"'){ // New entry

			key = JSON::scanSegment(istr, "\"");
			istr.get(); // swallow terminator
			// log.debug(1) << " key=" << key << log.endl;

			node_t & vmap = t.data;

			skipChars(istr, " \t\n\r");
			c = istr.get();
			if (c == ':'){
				skipChars(istr, " \t\n\r");
				c = istr.peek();
				switch (c) {
				case '{': // NESTING OBJECT (SUBTREE)
					//istr.get();
					log.debug(2) << "Reading object '" << key << "'" << log.endl;
					JSON::read(t[key], istr);
					break;
				case '"': // STRING
					istr.get();
					value = JSON::scanSegment(istr, "\"");
					istr.get();
					// std::cout << "String: " << value << '\n';
					log.debug(2) << "String attribute '" << key << "'='" << value << "'" << log.endl;
					vmap[key] = value;
					break;
				case '[': // ARRAY TODO: chars/integer/float handling
					istr.get();
					value = JSON::scanSegment(istr, "]");
					istr.get();
					if (value.find_first_of("{}") != std::string::npos){
						log.warn() << "Arrays of objects not supported (key='" << key << "')" << log.endl;
					}
					if (value.find_first_of("[]") != std::string::npos){
						log.warn() << "Arrays of arrays not supported (key='" << key << "')" << log.endl;
					}

					if (true) {
						std::list<std::string> l;
						drain::StringTools::split(value, l, ',', " '\"\t\n\r");
						const std::type_info & atype = Type::guessArrayType(l);
						log.debug(2) << "Array attribute '" << key << "'=[" << value << "] (type="<< drain::Type::getTypeChar(atype) << ")" << log.endl;
						vmap[key].clear();
						vmap[key].setType(atype);
						for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); ++it) {
							vmap[key] << *it;
						}
						//vmap[key] = value;
					}
					break;
				default: //
					value = JSON::scanSegment(istr, ",} \t\n\r");
					const std::type_info & type = Type::guessType(value);
					vmap[key].setType(type);
					log.debug(2) << "Numeric attribute '" << key << "'= " << value << ", type=" << drain::Type::getTypeChar(type) << log.endl;
					vmap[key] = value;
					break;
				}
				completed = true;
			}
			else {
				log.error() << "Syntax error: read '" << c << "' when expecting object {...}, string \"...\", array [...], or number" << log.endl;
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

void JSON::write(const tree_t & json, std::ostream & ostr, unsigned short indentation){

	const node_t & vmap = json.data;

	char sep = 0;

	ostr << "{\n";

	indentation += JSON::indentStep;

	if (!vmap.empty()){
		vmap.toJSON(ostr, indentation); // relies on similar formatting
		if (!json.isEmpty())
			ostr << ','; // Add comma, if non-empty subtree coming next
		ostr << '\n';
	}


	for (typename tree_t::const_iterator it = json.begin(); it != json.end(); ++it){

		if (sep){
			ostr << sep << '\n';
		}
		else {
			sep = ',';
		}

		indent(ostr, indentation);
		ostr << '"' << it->first << '"' << ": ";

		// Recursion
		JSON::write(it->second, ostr, indentation); // + JSON::indentStep);

	}

	/// If also object was dumped above, add newline
	if (!json.isEmpty())
		ostr << '\n';

	// Attributes and object are completed, hence decrement indentation for terminal char '}'
	if (indentation >= JSON::indentStep)
		 indentation -= JSON::indentStep;
	else {
		drain::Logger mout("JSON", __FUNCTION__);
		mout.warn() << "skipped negative indentation" << mout.endl;
	}

	indent(ostr, indentation);
	ostr << '}'; // << indentation;

	// If end of recursion, file completed, add newline.
	if (indentation == 0)
		ostr << '\n';



}




// Read utils

void JSON::skipChars(std::istream & istr, const std::string chars){

	while (istr){
		if (chars.find(istr.peek()) == std::string::npos)
			return;
		else
			istr.get();
	}

}


std::string JSON::scanSegment(std::istream & istr, const std::string & terminator){

	drain::Logger mout("JSON", __FUNCTION__);

	char c;
	std::stringstream sstr;  // TODO escape handler

	while (istr){

		c = istr.peek();
		if (c == '\\'){
			mout.warn() << "special char: " << c << mout.endl; // todo: interpret \t, \n ?
			// sstr << c; // swallowed, hence should be returned in output?
			istr.get();
			c = istr.get();
			if (!istr){
				mout.warn() << "premature file end, str=" << sstr.str() << mout.endl;
				return sstr.str();
			}
			sstr << c;
			continue;
		}

		if (terminator.find(c) == std::string::npos)
			sstr << c;  //c;
		else // did not swallow terminator! (maybe comma)
			return sstr.str();
		istr.get();
	}

	mout.warn() << "premature file end, str=" << sstr.str() << mout.endl;

	return sstr.str();
}




} // drain::
