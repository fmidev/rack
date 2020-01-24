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

//#include <sstream>
#include <ostream>
#include <stdexcept>

#include "Log.h"

#include "TextReader.h"


namespace drain
{


/**
 *
 *
 *
 */


// Read utils

void TextReader::skipChars(std::istream & istr, const std::string chars){

	while (istr){
		if (chars.find(istr.peek()) == std::string::npos)
			return;
		else
			istr.get();
	}

}



bool TextReader::scanSegment(std::istream & istr, const std::string & endChars, std::ostream & ostr){

	drain::Logger mout(__FUNCTION__, __FILE__);

	//size_t count = 0;
	int c;

	// NOTE: in case of istringstream, while(!istr.eof()) or while(istr) do not work
	while (true){

		c = istr.get(); //istr.peek(); // no eof triggered until this
		if (istr.eof())
			return false;

		if (c == '\\'){
			mout.warn() << "special char: " << c << mout.endl; // todo: interpret \t, \n ?
			// ostr << c; // swallowed, hence should be returned in output?
			//istr.get();
			c = istr.get();
			if (!istr){
				//mout.warn() << "str=" << ostr.str() << mout.endl;
				mout.warn() << "premature end of file" << mout.endl; // , str=" << ostr.str()
				return false; // ostr.str();
			}
			ostr.put(c);
			//++count;
			continue;
		}

		if (endChars.find(c) == std::string::npos){
			ostr.put(c);
			//++count;
		}
		else // end char found! Note: does not swallow endChars (maybe comma)
			return true;

		//istr.get();
	}

	//mout.warn() << "premature file end, str=" << ostr.str() << mout.endl;
	mout.warn() << "premature end of file, last char=" << c << mout.endl;

	return false; // ostr.str();
}


// Specified implementation
template <>
bool TextReader::scanSegmentToValue(std::istream & istr, const std::string & endChars, std::string & dst){
	std::stringstream sstr;
	bool result = scanSegment(istr, endChars, sstr);
	dst.assign(sstr.str());
	return result;
}


} // drain::
