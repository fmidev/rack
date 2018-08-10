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
#ifndef AccumulationConverter_H_
#define AccumulationConverter_H_


//#include <drain/util/Rectangle.h>
//#include "AccumulatorGeo.h"

#include <string>

// using namespace std;

namespace drain {

namespace image {

/// Converts raw data to values appropriate for accumulation.
/**
 *
 */
struct AccumulationConverter {

public:

	virtual
	~AccumulationConverter(){};

	/// Converts storage data containing marker codes etc to natural scale. Returns true if data is valid for use, otherwise false.
	inline
	virtual
	bool decode(double & value) const { return true; };

	/// Converts storage data containing marker codes etc to natural scale. Returns true if data is valid for use, otherwise false.
	inline
	virtual
	bool decode(double & value, double & weight) const { return true; };

	/// Converts natural-scale data to storage data containing marker codes etc.
	inline
	virtual
	void encode(double & value, double & weight) const {};

	/// Converts natural-scale data to storage data containing marker codes etc.
	inline
	virtual
	void encodeWeight(double & weight) const {};

	/// Converts natural-scale data to storage data containing marker codes etc.
	inline
	virtual
	void encodeCount(double & weight) const {};

	inline
	virtual
	void encodeStdDev(double & stdDev) const {};

	/// When using unsigned types, encoding typically requires adding a positive bias and scaling the data.
	inline
	virtual
	void encodeDiff(double & diff) const {};

	/// Returns the value when weight==0.0. Physically, this relates to results obtained under measurement accuracy.
	inline
	double getNoDataMarker() const {
		double value = 0.0;
		double  weight = 0.0;
		encode(value, weight);
		return value;
	}

	/// Default output storage data type
	std::string type;

	virtual
	std::ostream & toOstr(std::ostream & ostr) const {
		//ostr << 5; ????
		return ostr;
	}




};


inline
std::ostream & operator<<(std::ostream & ostr, const AccumulationConverter & converter){
	return converter.toOstr(ostr);
}

} // image::


} // drain::

#endif /*DataAccumulator_H_*/

// Drain
