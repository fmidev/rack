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
#ifndef DRAIN_UNITS
#define DRAIN_UNITS

#include "drain/Enum.h"

namespace drain {


extern const double DEG2RAD;
extern const double RAD2DEG;

extern const int   EARTH_RADIUSi;
extern const double EARTH_RADIUS;

// Basically "extensible". Other enum lists may be used. Consider reflectivity (Z) and dBZ.
enum Unit {
	UNDEFINED = 0,
	// SI units:
	METRE, // Br.Eng.
	RADIAN,   // degree
	DEGREE,   // degree
	// Other:
	PIXEL, // screen pixel
};

class Units {
public:


	///
	template <typename E=Unit>
	static
	E extract(const std::string & expr);


};


template <typename E>
E Units::extract(const std::string & expr){

	const size_t N = expr.length();
	for (const auto & entry: Enum<E>::dict){
		const size_t n =  entry.first.length();
		if (n>0){ // empty is ok (typically UNDEFINED etc), but skipped.
			if (expr.substr(N-n) == entry.first){ // find(entry.first, N-n)
				return entry.second;
			}
		}
	}

	// Contract: enum type E must have an entry associated with int value zero.
	return static_cast<E>(0);
}


DRAIN_ENUM_DICT(Unit);
DRAIN_ENUM_OSTREAM(Unit);

} // ::drain

#endif // DRAIN_UNITS

