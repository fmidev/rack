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
#include <fstream>
#include <stdexcept>

#include "Log.h"
#include "Type.h"
#include "FilePath.h"
#include "TextReader.h"
//#include "ValueReader.h"
#include "JSONtree.h"
#include "JSON.h"

namespace drain
{




/**
 *
 *  TODO: manual+includes, TypeUtils, Skip escaped \\
 *
 *
 */
//unsigned short JSONtree::indentStep(2);

//onst RegExp JSONtree::filenameExtension("\\.([[:alnum:]]+)$");

void JSONtree::read(tree_t & t, std::istream & istr){

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

			node_t & vmap = t.data;

			TextReader::skipWhiteSpace(istr); // Chars(istr, " \t\n\r");
			// log.warn() << " then3: " <<  (char)istr.peek() << log.endl;
			c = istr.get();
			// log.warn() << " then4: " <<  (char)istr.peek() << log.endl;

			if (c == ':'){
				TextReader::skipWhiteSpace(istr) ; // Chars(istr, " \t\n\r");
				c = istr.peek();

				if (c == '{'){
					log.debug(2) << "Reading object '" << key << "'" << log.endl;
					/// RECURSION
					JSONtree::read(t[key], istr);
				}
				else {
					log.debug(2) << "Reading value '" << key << "'" << log.endl;
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


/// Reads and parses a Windows INI file
void JSONtree::readINI(tree_t & t, std::istream & istr){
	drain::Logger mout("JSON", __FUNCTION__);
	mout.error() << "unimplemented code" << mout.endl;
}




void JSONtree::write(const tree_t & json, const std::string & filename){

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL "JSON", __FUNCTION__);

	const FilePath path(filename);
	//mout.error() << "failed in extracting file type of filename: " <<  filename << mout.endl;

	std::ofstream outfile;
	outfile.open(filename.c_str(), std::ios::out);

	if (!outfile){
		mout.error() << "failed in opening file: " <<  filename << mout.endl;
	}
	// std::vector<std::string> result;
	//if (!filenameExtension.execute(filename, result)){
	//const std::string & ext = result[1];


	if (path.extension == "json"){
		JSONtree::writeJSON(json, outfile);
	}
	else if (path.extension == "ini"){
		JSONtree::writeINI(json, outfile);
	}
	else {
		mout.error() << "unknown file type: " << path.extension << mout.endl;
	}


	outfile.close();

}



template <>
std::ostream & JSONwriter::toStream(const drain::JSONtree::tree_t & t, std::ostream &ostr, unsigned short indentation){

	const bool ATTRIBS  = !t.data.empty();
	const bool CHILDREN = !t.isEmpty();

	ostr << '{';

	if (ATTRIBS)
		ostr << '\n';

	JSONwriter::mapElementsToStream(t.data, ostr, indentation);

	if (ATTRIBS && CHILDREN){
		ostr << ',';
	}

	if (CHILDREN)
		ostr << '\n';

	JSONwriter::mapElementsToStream(t, ostr, indentation);

	if (ATTRIBS || CHILDREN){
		ostr << '\n';
		JSONwriter::indent(ostr, indentation);
	}

	ostr << '}'; // << '\n';

	return ostr;
}


void JSONtree::writeJSON(const drain::JSONtree::tree_t & t, std::ostream & ostr, unsigned short indentation){
	JSONwriter::toStream(t, ostr, indentation);
}

/*
void JSONtree::writeJSON(const tree_t & json, std::ostream & ostr, unsigned short indentation){



	char sep = 0;

	ostr << "{\n";

	indentation += JSONtree::indentStep;

	if (!json.data.empty()){
		json.data.toJSON(ostr, indentation); // relies on similar formatting
		if (!json.isEmpty())
			ostr << ','; // Add comma, if non-empty subtree coming next
		ostr << '\n';
	}


	// Traverse descendants recursively
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
		JSONtree::writeJSON(it->second, ostr, indentation); // + JSONtree::indentStep);

	}

	/// If also object was dumped above, add newline
	if (!json.isEmpty())
		ostr << '\n';

	// Attributes and object are completed, hence decrement indentation for terminal char '}'
	if (indentation >= JSONtree::indentStep)
		indentation -= JSONtree::indentStep;
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
*/


/// Write a Windows INI file
void JSONtree::writeINI(const tree_t & t, std::ostream & ostr, const tree_t::path_t & prefix){


	for (tree_t::node_t::const_iterator dit = t.data.begin(); dit != t.data.end(); ++dit){
		ostr << dit->first << '='; // << dit->second;
		dit->second.valueToJSON(ostr);
		ostr << '\n';
	}
	ostr << '\n';


	// Traverse children (only)
	for (tree_t::const_iterator it = t.begin(); it != t.end(); ++it){

		tree_t::path_t path(prefix);
		path << it->first;

		ostr << '[' << path << ']' << '\n';

		writeINI(it->second, ostr, path);

		ostr << '\n';

	}

}






} // drain::
