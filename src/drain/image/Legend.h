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
#ifndef DRAIN_LEGEND
#define DRAIN_LEGEND

//#include "Geometry.h"
//#include "File.h"
//#include "TreeSVG.h"

//#include "drain/util/JSONtree.h"
#include <ostream>
#include <vector>
#include <map>

#include "drain/util/Log.h"
#include "ImageCodebook.h"

namespace drain
{



// -----------------------

class LegendEntry { //: public std::string {

public:

	std::string label;

};



inline
std::ostream & operator<<(const LegendEntry & entry, std::ostream &ostr){
	//return ostr << (const std::string &)entry;
	return ostr << entry.label;
}


class Legend : public ImageCodeMap<std::string> {
public:

	Legend(){};// : ImageCodeMap<std::string>(256)

};

/*
template <>
inline
std::ostream & JSONwriter::toStream(const Legend & l, std::ostream &ostr, unsigned short indentation){
	return JSONwriter::mapToStream(l, ostr, indentation);
	//return JSONwriter::sparseSequenceToStream(l, ostr);
}
*/


/*
template <class T>
inline
std::ostream & operator<<(std::ostream &ostr, const ImageCodeMap<T> & m){
	m.toOStream(ostr);
	return ostr;
}
*/



} // drain::

#endif
