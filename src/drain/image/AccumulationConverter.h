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


//#include "drain/util/Rectangle.h"
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

	/// Converts storage data to natural scale.
	/**
	 *   \param value - storage data, possibly containing marker codes
	 *   \return - true if data is valid for use, otherwise false.
	 */
	virtual inline
	bool decode(double & value) const { return true; };

	/// Converts storage data, possibly containing marker codes, to natural scale. Returns true if data is valid for use, otherwise false.
	/// Converts storage data to natural scale.
	/**
	 *   \param value - storage data, possibly containing marker codes
	 *   \return - true if data is valid for use, otherwise false.
	 */
	virtual inline
	bool decode(double & value, double & weight) const { return true; };

	/// Converts natural-scale data to storage data, applying marker codes if needed.
	virtual	inline
	void encode(double & value, double & weight) const {};

	/// Converts natural-scale data to storage data, applying marker codes if needed.
	virtual	inline
	void encodeWeight(double & weight) const {};

	/// Converts natural-scale data to storage data, applying marker codes if needed.
	virtual	inline
	void encodeCount(double & weight) const {};

	virtual	inline
	void encodeStdDev(double & stdDev) const {};

	/// When using unsigned types, encoding typically requires adding a positive bias and scaling the data.
	virtual	inline
	void encodeDiff(double & diff) const {};

	/// Returns the value when weight==0.0. Physically, this relates to results obtained under measurement accuracy.
	virtual inline
	double getNoDataMarker() const {
		double value  = 0.0;
		double weight = 0.0;
		encode(value, weight);  // infinite loop, if encode uses marker?
		return value;
	}

	/**
	 *  Unchecked code!
	 */
	virtual inline
	double getNoWeightDataMarker() const {
		double value  = 0.0;
		double weight = 0.0;
		encode(value, weight);
		return weight;
	}

	/// Returns a marker value which indicates that although data has been measured, it is not within required accuracy or is otherways unreliable.
	virtual inline
	double getNoReadingMarker() const {
		return 0.0;
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
