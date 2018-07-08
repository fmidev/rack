/*

    Copyright 2001 - 2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

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
