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
#ifndef DRAIN_VALUE_READER_H_
#define DRAIN_VALUE_READER_H_

#include <string>

#include "VariableMap.h"

namespace drain
{



/// Utility for extracting JSON-compatible strings, numbers or arrays of numbers in text data
/*'
 *  Applicable in reading configuration files and comments containing:
 *  -# plain numbers, distinguishing integers and floats
 *	-# arrays of numbers, surrounded by braces [,]
 *	-# strings, surrounded by double hyphens (")
 *
 *	Applies TextReader::scanSegment in reading character streams.
 *	Uses Type::guessArrayType() for deriving compatible storage type for arrays.
 *
 *
 */
class ValueReader  {

public:

	/// Read stream until a value has been extracted, with type recognition
	static
	void scanValue(std::istream & istr, Variable & v);

	/// Read stream until a value has been extracted, with type recognition
	static inline
	void scanValue(const std::string & s, Variable & v){
		std::istringstream istr(s);
		scanValue(istr, v);
	};

	/// Given comma-separated string of values, assign them to variable of minimum compatible type
	static
	void scanArrayValues(const std::string & s, Variable & v);


};


}  // drain

#endif /* META_DATA_H_*/
