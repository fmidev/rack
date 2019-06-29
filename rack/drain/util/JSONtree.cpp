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
#include "Type.h"
#include "TextReader.h"
#include "ValueReader.h"
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
			istr.get(); // swallow terminator
			// log.debug(1) << " key=" << key << log.endl;

			node_t & vmap = t.data;

			TextReader::skipChars(istr, " \t\n\r");
			c = istr.get();
			if (c == ':'){
				TextReader::skipChars(istr, " \t\n\r");
				c = istr.peek();

				if (c == '{'){
					log.debug(2) << "Reading object '" << key << "'" << log.endl;
					JSON::read(t[key], istr);
				}
				else {
					log.debug(2) << "Reading value '" << key << "'" << log.endl;
					ValueReader::scanValue(istr, vmap[key]);
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


	for (tree_t::const_iterator it = json.begin(); it != json.end(); ++it){

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






} // drain::
