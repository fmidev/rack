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
 
#include "Sampler.h"

namespace drain {

namespace image {

Sampler::Sampler() : BeanLike(__FUNCTION__, "Extract samples from image") /*,
		iStep(10),l
		jStep(0),
		iRange(-1,-1),
		jRange(-1,-1),
		commentPrefix("#"),
		handleNoData("NaN"),
		//skipVoid(false),
		voidMarker("void") */{
	parameters.link("iStep",  iStep, "horz coord step");
	parameters.link("jStep",  jStep, "vert coord step");
	parameters.link("i", iRange.tuple(), "horz index or range").fillArray = true;
	parameters.link("j", jRange.tuple(), "vert index or range").fillArray = true;
	parameters.link("commentChar",   commentPrefix,  "comment prefix (char or bytevalue)");
	parameters.link("skipVoid", skipVoid,  "skip lines with invalid/missing values");
	parameters.link("handleVoid", handleVoid,  "skip or mark invalid values [skip|null|<number>]");
	//parameters.link("iStart", iRange.vect[0], "first horz index or range").fillArray = true;
	//parameters.link("jStart", jRange.vect[0], "second vert index or range").fillArray = true;
};

char Sampler::getCommentChar() const {

	if (commentPrefix.empty()){
		return 0;
	}
	else {

		char c = commentPrefix[0];

		if (commentPrefix.size() == 1){ // ASCII chars #0...#9  not accecpted
			if ((c == '0') && (c == '\0')){
				return 0;
			}
		}

		int i;
		drain::StringTools::convert(commentPrefix, i);
		if (i==0){
			// some char, not a digit
			return c;
		}
		else if ((i < 32) || (i > 128)){
			drain::Logger mout(__FUNCTION__, getName());
			mout.suspicious("commentChar='", (char)i ,"' bytevalue=" , i);
		}

		return i;
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
	mout.debug("format not given, using default (all the quantities)" );
	std::stringstream sstr;
	const std::list<std::string> & keys = variableMap.getKeyList();
	char separator = 0;
	for (std::list<std::string>::const_iterator it=keys.begin(); it!=keys.end(); ++it){
		if (!it->empty()){
			if (it->at(0) != '-'){ // Skip negatives
				if (separator)
					sstr << separator;
				else
					separator = ',';
				sstr << '$' << '{' << *it << '}';
			}
		}
		else {
			mout.warn("empty quantity" );
		}
	}
	mout.note("format: using default: " , sstr.str() );
	return sstr.str();
}



}  // namespace image

}  // namespace drain
