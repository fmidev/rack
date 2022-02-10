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

#ifndef DRAIN_IOS_FORMAT_H_
#define DRAIN_IOS_FORMAT_H_

#include <ios>

#include "Log.h"

namespace drain {

/// Stores precision, fillChar and fieldWidth applied by STD streams.
/**
 *
 */
class IosFormat {

public:

	inline
	IosFormat(const std::ostream & ostr = std::cout){
		copyFrom(ostr);
	}

	inline
	void copyFrom(const std::ostream & ostr){
		precision = ostr.precision();
		fillChar  = ostr.fill();
		fieldWidth = ostr.width();
	}


	inline
	void copyTo(std::ostream & ostr) const {
		ostr.precision(precision);
		ostr.fill(fillChar);
		ostr.width(fieldWidth);
	}


	inline
	char getFillChar() const {
		return fillChar;
	}

	inline
	void setFillChar(char c) {
		this->fillChar = c;
	}

	inline
	std::streamsize getFieldWidth() const {
		return fieldWidth;
	}

	inline
	void setFieldWidth(std::streamsize w) {
		this->fieldWidth = w;
	}

	std::streamsize getPrecision() const {
		return precision;
	}

	void setPrecision(std::streamsize precision) {
		this->precision = precision;
	}


protected:

	std::streamsize precision;
	std::streamsize fieldWidth;
	char fillChar;


};

inline
std::ostream & operator<<(std::ostream & ostr, const IosFormat & iosformat){
	if (iosformat.getFillChar())
		ostr << iosformat.getFillChar();
	ostr << iosformat.getFieldWidth() << "d";
	ostr << "/0." << iosformat.getPrecision() << "f";
	return ostr;
}


} // NAMESPACE

#endif /* STRINGMAPPER_H_ */

// Drain
