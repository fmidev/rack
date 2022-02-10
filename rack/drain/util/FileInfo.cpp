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

// #include "drain/util/Time.h"
#include <sstream>
#include "FileInfo.h"
#include "StringBuilder.h"

namespace drain
{


void FileInfo::setExtensionRegExp(const std::string & extRegExp, int flags){
	// fileNameRegexp.setExpression(StringBuilder().create(".*\\.", r, "$").str(), flags);
	//extensionRegexp.setExpression(StringBuilder().create("((.*/)?([^/]+)).*\\.", r, "$").str(), flags);
	/*
	std::stringstream sstr;
	sstr << '^' << extRegExp << '$';
	extensionRegexp.setExpression(sstr.str(), flags);
	*/
	extensionRegexp.setExpression(StreamBuilder().create("^",extRegExp,"$").str(), flags);
}

Registry<FileInfo> & getFileInfoRegistry(){
	static Registry<FileInfo> registry;
	return registry;
}




} // drain::

