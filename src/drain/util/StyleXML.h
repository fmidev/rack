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
 *  StyleXML.h
 *
 *      Author: mpeura
 */


#ifndef DRAIN_STYLE_XML
#define DRAIN_STYLE_XML

#include <ostream>

#include <drain/Sprinter.h>
#include <drain/FlexibleVariable.h>

#include "ReferenceMap.h"

namespace drain {


/**
 *   Flexibility is used (at least) in:
 *   - linking box.height to font-size (in TEXT or TSPAN elems)
 */
class StyleXML : public ReferenceMap2<FlexibleVariable> {

public:

	inline
	StyleXML(){};

	static const SprinterLayout styleLineLayout;
	static const SprinterLayout styleRecordLayout;
	static const SprinterLayout styleRecordLayoutActual;

	/// Practical utility, helps in adding C++ code commenting...
	template <class T>
	static inline
	void commentToStream(std::ostream & ostr, const T & v){
		ostr << "/* " << v << " */ ";
	};

protected:

};


inline
std::ostream & operator<<(std::ostream &ostr, const StyleXML & style){
	drain::Sprinter::toStream(ostr, style.getMap(), drain::Sprinter::xmlAttributeLayout);
	return ostr;
}


}  // drain::

#endif /* DRAIN_XML */

