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

#include <drain/image/File.h>
//#include <drain/image/MathOpPack.h>
#include <drain/util/Log.h>

#include "hi5/Hi5Write.h"

#include "AndreOp.h"


using namespace drain::image;
using namespace hi5;



namespace rack {

int AndreOp::getClassCode(const std::string & key){

	drain::Logger mout("AndreOp", __FUNCTION__);

	classtree_t &t = getClassTree()["entries"];

	classtree_t::path_t path(key, t.getSeparator());
	mout.debug() << "path(" << path.separator << ") "<< drain::StringTools::join(path, path.separator) << mout.endl;

	return getClassCode(t, path.begin(), path.end());

}

int AndreOp::getClassCode(classtree_t & tr, classtree_t::path_t::const_iterator it, classtree_t::path_t::const_iterator eit){


	drain::Logger mout("AndreOp", __FUNCTION__);

	if (it == eit){ // "empty path"
		if (!tr.data.hasKey("value")){
			mout.note() << "missing 'value' attribute (index) of existing class " << tr.data << mout.endl; // ddificult to locate, try tr.dump()
		}
		return tr.data["value"];
	}

	const classtree_t::path_t::value_type & key = *it;
	//mout.note() << "entered " << key << mout.endl;

	if (!tr.hasChild(key)){
		static unsigned short counter(32);
		mout.note() << "creating class code: *." << *it << ' ' << counter << mout.endl;
		//tr.getPaths()
		tr[key].data["value"] = counter;
		++counter;
	}
	else {
		mout.info() << "existing class code: *." << *it << '(' << tr[key].data.getValues() << ')' << mout.endl;
	}

	//mout.note() << "descending to " << *it << mout.endl;
	return getClassCode(tr[key], ++it, eit);

}


}  // rack::


// Rack
