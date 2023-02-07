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

#include "FilePath.h"
#include "Log.h"
#include "Output.h"
#include "TextReader.h"
#include "Type.h"

#include "JSONtree.h"
#include "JSON.h"

namespace drain
{

FileInfo JSONtree::fileInfo("json");


template <>
const drain::JSONtree2 drain::JSONtree2::emptyNode;

/**
 *
 *  TODO: manual+includes, TypeUtils, Skip escaped \\
 *
 *
 */


/// Reads and parses a Windows INI file
void JSONtree::readINI(tree_t & t, std::istream & istr){
	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.unimplemented() << "unimplemented code" << mout.endl;
}


void JSONtree::write(const tree_t & json, const std::string & filename){

	drain::Logger mout(__FUNCTION__, __FILE__);

	//mout.error() << "failed in extracting file type of filename: " <<  filename << mout.endl;

	Output outfile(filename);

	if (!outfile){
		mout.error() << "failed in opening file: " <<  filename << mout.endl;
	}

	const FilePath path(filename);

	if (path.extension == "json"){
		mout.unimplemented("JSONtree::writeJSON OLD");
		//JSONtree::writeJSON(json, outfile);
	}
	else if (path.extension == "ini"){
		JSONtree::writeINI(json, outfile);
	}
	else {
		mout.error() << "unknown file type: " << path.extension << mout.endl;
	}

	//outfile.close();

}


/*
template <>
std::ostream & JSONwriter::toStream(const drain::JSONtree::tree_t & t, std::ostream &ostr, unsigned short indentation){

	const bool ATTRIBS  = !t.data.empty();
	const bool CHILDREN = !t.empty();

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
*/

/*
void JSONtree::writeJSON(const drain::JSONtree::tree_t & t, std::ostream & ostr, unsigned short indentation){
	JSONwriter::toStream(t, ostr, indentation);
}
*/



/// Write a Windows INI file
void JSONtree::writeINI(const tree_t & t, std::ostream & ostr, const tree_t::path_t & prefix){

/*
	//for (tree_t::node_t::const_iterator dit = t.data.begin(); dit != t.data.end(); ++dit){
	for (const auto & entry: t.data){
		ostr << entry.first << '='; // << dit->second;
		SprinterBase::toStream(ostr, entry.second, SprinterBase::jsonLayout);
		// entry.second.valueToJSON(ostr);
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
*/

}






} // drain::
