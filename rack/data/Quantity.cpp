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

#include <drain/util/Debug.h>

#include "Quantity.h"

namespace rack {



EncodingODIM & Quantity::set(char typecode) {

	if (!typecode)
		typecode = 'C';  // ???

	if (!defaultType)
		defaultType = typecode;

	EncodingODIM & odim = (*this)[typecode];
	odim.type = typecode;

	return odim;
}


/*
Quantity & Quantity::set(char typecode, double gain, double offset) {

	return set(typecode, gain, offset,
			drain::Type::getMin<double>(typecode),
			drain::Type::getMax<double>(typecode));

}


Quantity & Quantity::set(char typecode, double gain, double offset, double undetect, double nodata) {

		if (!typecode)
			typecode = 'C';  // ???

		EncodingODIM & odim = (*this)[typecode];
		odim.type = std::string(1, typecode);
		odim.gain = gain;
		odim.offset = offset;
		odim.undetect = undetect;
		odim.nodata = nodata;

		if (!defaultType)
			defaultType = typecode;

		return *this;

}
*/

/*
Quantity & Quantity::setRange(char typecode, double min, double max) {

	if (!typecode)
		typecode = 'C';

	EncodingODIM & odim = (*this)[typecode];
	odim.type = std::string(1, typecode);

	//drain::Type t(typecode);
	const double minByte = drain::Type::getMin<double>(typecode);
	const double maxByte = drain::Type::getMax<double>(typecode);
	odim.gain = (max-min)/(maxByte-1 - (minByte+1));
	// min = minByte*gain + offset
	odim.offset = min -odim.gain*minByte;
	odim.undetect = minByte;
	odim.nodata   = maxByte;

	return *this; // ?

}
*/

///
/**
 *   \return true, if conf for this quantity was found, with desired type (or default type)
 */

}  // namespace rack



// Rack
