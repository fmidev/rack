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
 
/*
 * TreeSVG.cpp
 *
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#include "Sampler.h"

namespace drain {

namespace image {

Sampler::Sampler() : BeanLike(__FUNCTION__, "Extract samples from image"),
		iStep(10),
		jStep(0),
		iRange(-1,-1),
		jRange(-1,-1),
		commentPrefix("#"),
		skipVoid(false),
		voidMarker("void data") {
	parameters.link("iStep",  iStep, "horz coord step");
	parameters.link("jStep",  jStep, "vert coord step");
	parameters.link("i", iRange.vect, "horz index or range").fillArray = true;
	parameters.link("j", jRange.vect, "vert index or range").fillArray = true;
	parameters.link("commentChar",   commentPrefix,  "comment prefix (char or bytevalue)");
	parameters.link("skipVoid", skipVoid,  "skip lines with invalid/missing values");
};

char Sampler::getCommentChar() const {
	if (commentPrefix.empty()){
		return  0;
	}
	else {
		if (commentPrefix.size() == 1){ // ASCII chars #0...#9  not accecpted
			return commentPrefix[0];
		}
		else {
			drain::Logger mout(__FUNCTION__, getName());
			int i = 0;
			drain::StringTools::convert(commentPrefix, i);
			if ((i < 32) || (i > 128))
				mout.warn() << "commentChar bytevalue: " << i << " > commentChar=" << (char)i << mout.endl;
			return i;
		}
	}
}


std::string Sampler::getFormat(const std::string & formatStr) const {

	if (!formatStr.empty()){
		return formatStr;
	}

	drain::Logger mout(__FUNCTION__, getName());

	//const bool FORMAT = !formatStr.empty();
	//std::string format = drain::StringTools::replace(formatStr, "\\n", "\n"); // "\\n", "\n");
	//format = drain::StringTools::replace(format, "\\t", "\t");

	/// If format not explicitly set, use all the variables => create default format.
	//if (!FORMAT){
	mout.debug() << "constructing default format (all the quantities)" << mout.endl;
	std::stringstream sstr;
	const std::list<std::string> & keys = variableMap.getKeyList();
	char separator = 0;
	for (std::list<std::string>::const_iterator it=keys.begin(); it!=keys.end(); ++it){
		if (!it->empty()){
			if (it->at(0) != '-'){
				if (separator)
					sstr << separator;
				else
					separator = ',';
				sstr << '$' << '{' << *it << '}';
			}
		}
		else {
			mout.warn() << "empty quantity" << mout.endl;
		}
	}
	return sstr.str();
}


/*
std::ostream & NodeSVG::toOStr(std::ostream &ostr, const TreeSVG & tree){
	ostr << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>";
	ostr << '\n';
	//NodeXML::toOStr()
	//NodeXML::toOStr(ostr, tree, "svg");
	NodeXML::toOStr(ostr, tree);
	return ostr;
}
*/


}  // namespace image

}  // namespace drain


// Rack
