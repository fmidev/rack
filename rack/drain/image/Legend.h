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
#ifndef DRAIN_CODEMAP
#define DRAIN_CODEMAP

//#include "Geometry.h"
//#include "File.h"
//#include "TreeSVG.h"

//#include "util/JSONtree.h"
#include <ostream>
#include <vector>

namespace drain
{


template <class T>
class ImageCodeMap : public std::vector<T> {

public:

	typedef std::vector<T> cont_t;
	typedef T value_t;

	/// Default constructor
	ImageCodeMap(typename cont_t::size_type n=0, const T & value=T()): cont_t(n, value), separator(0) {

	};

	/// Copy constructor
	ImageCodeMap(const cont_t & map): cont_t(map), separator(0) {

	};

	///
	/*
	 *    \param equal - typically =, :, or -
	 *    \param start - typically hyphen or leading parenthesis (, {, [
	 *    \param end   - typically hyphen or trailing parenthesis ), }, [
	 *    \param separator - typically comma or semicolon
	 */
	void toOStream(std::ostream & ostr, char equal='=', char startChar=0, char endChar=0, char separatorChar=0) const {

		char sep = 0;

		for (typename cont_t::size_type i = 0; i != this->size(); ++i){

			if (sep)
				ostr << sep;
			else {
				sep = separatorChar ? separatorChar : this->separator;
				sep = sep ? sep : ',';
			}

			if (equal)
				ostr << i << equal;

			if (startChar)
				ostr << startChar;

			ostr << (*this)[i];

			if (endChar)
				ostr << endChar;

		}

	}

	char separator;
};

template <class T>
inline
std::ostream & operator<<(std::ostream &ostr, const ImageCodeMap<T> & m){
	m.toOStream(ostr);
	return ostr;
}




} // drain::

#endif
