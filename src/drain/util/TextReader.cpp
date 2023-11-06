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

char TextReader::skipChars(std::istream & istr, const std::string chars){

	char c = '\0';
	while (istr){
		if (chars.find(istr.peek()) != std::string::npos)
			c = istr.get(); // Skip that char and continue
		else
			return '\0'; //c;
	}

	return c;
}



char TextReader::scanSegment(std::istream & istr, const std::string & endChars, std::ostream & ostr){

	drain::Logger mout(__FILE__, __FUNCTION__);

	int c=0, cPrev=0;

	// NOTE: in case of istringstream, while(!istr.eof()) or while(istr) do not work
	while (true){

		//c0 = istr.get(); //istr.peek(); // no eof triggered until this
		c = istr.peek(); //istr.peek(); // no eof triggered until this
		if (istr.eof())
			return cPrev; // previous


		// Handle escape - skip detecting endChars
		if (c == '\\'){
			istr.get(); // Swallow escape char.
			//mout.warn("escape char (", c, ")"); // todo: interpret \t, \n ?
			if (!istr){
				//mout.warn() << "str=" << ostr.str() << mout.endl;
				mout.warn("premature end-of-file after escape char '\\' (", c, ")"); // , str=" << ostr.str()
				return cPrev;
			}

			c = istr.get();

			switch (c){
			// Accept silently
			case '\\':
			case '"':
				break;
			// Accept standard special chars
			case 'n':
				c='\n';
				break;
			case 't':
				c='\t';
				break;
			case 'r':
				c='\r';
				break;
			case 'b': // bell
				c='\b';
				break;
			// Conformality to JSON not implemented. https://www.crockford.com/mckeeman.html
			case 'u':
				// mout.unimplemented();
				mout.unimplemented("unsupported escape, unicode '\\u", (char)c, "...'");
				break;
			default:
				mout.unimplemented("unsupported escape char: '\\", (char)c, "' (",(int)c,")");
			}
			ostr.put(c);
			/*
			if (!istr){
				mout.warn("premature end of file with special char '\\", (char)c, "'"); // , str=" << ostr.str()
				return cPrev;
			}
			*/

			continue;
		}

		if (endChars.find(c) != std::string::npos){ // end char found.
			return c;
		}

		ostr.put(c); // Accept and take char.
		istr.get();  // Finally, read that char...
		cPrev = c;

	}

	// Unreachable code...
	mout.warn("premature end of file, last char=", c);
	return 0;
}


// Specified implementation
template <>
char TextReader::scanSegmentToValue(std::istream & istr, const std::string & endChars, std::string & dst){
	std::stringstream sstr;
	char c = scanSegment(istr, endChars, sstr);
	dst.assign(sstr.str());
	return c;
}


} // drain::
