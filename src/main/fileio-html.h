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

#ifndef RACK_FILE_IO_HTML
#define RACK_FILE_IO_HTML

#include <string>

#include <drain/RegExp.h>

#include "drain/prog/CommandBank.h"
#include "drain/prog/CommandInstaller.h"
#include "drain/util/FileInfo.h"
#include "drain/util/TreeHTML.h"


#include "hi5/Hi5.h"

#include "resources.h"

namespace rack {


class H5HTMLextractor {

public:

	void setBaseDir(const drain::FilePath::path_t & dir){
		basedir = dir;
	};

	drain::FilePath::path_t basedir;

	// Returns the root of the HTML document, that is the <HTML> element. Prepares style etc. if not set already
	drain::TreeHTML & getHtml();

	int visitPrefix(const Hi5Tree & tree, const Hi5Tree::path_t & odimPath);

	inline
	int visitPostfix(const Hi5Tree & tree, const Hi5Tree::path_t & odimPath){
		return 0;
	};

	static
	drain::TreeHTML & addTogglerScript(drain::TreeHTML & head, const std::string key = "toggler-script");

	static
	drain::TreeHTML & addTogglerStyle(drain::TreeHTML & head, const std::string key = "toggler-style");

protected:

	drain::TreeHTML html;


};


} // rack::


#endif

