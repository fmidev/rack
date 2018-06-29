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
#ifndef DataCoder_H_
#define DataCoder_H_


//
//#include <drain/util/Rectangle.h>
#include <drain/image/AccumulationConverter.h>
#include <drain/util/Type.h>

#include "hi5/Hi5.h"
#include "data/ODIM.h"
#include "Data.h"
#include "DataSelector.h"
#include "QuantityMap.h"
//#include "Coordinates.h"


namespace rack {

using namespace drain::image;




/// Converts ODIM encoded data (with markers) to natural values and backwards.
/**
 *   Converts ODIM encoded data with \c nodata and \c undetect markers to natural values and backwards.
 */
class DataCoder : public AccumulationConverter {

public:

	DataCoder() : defaultQuality(0.5), // undetectQualityCoeff(0.75),
	dataODIM(defaultDataODIM), qualityODIM(defaultQualityODIM) {
		init();
		//setLimits();
	};

	/// Without quality (with impicit quality)
	DataCoder(const ODIM & dataODIM) : defaultQuality(0.5), // undetectQualityCoeff(0.75),
			dataODIM(dataODIM), qualityODIM(defaultQualityODIM) {
		init();
		const QuantityMap & qm = getQuantityMap();
		qm.setQuantityDefaults(defaultQualityODIM, "QIND", "C");
		// setLimits();
		//defaultQualityODIM.setQuantityDefaults("QIND","C");
	};

	DataCoder(const ODIM & dataODIM, const ODIM & qualityODIM) : defaultQuality(0.5), //undetectQualityCoeff(0.75),
			dataODIM(dataODIM), qualityODIM(qualityODIM) {
		init();
		//setLimits();
	};

	DataCoder(const DataCoder & converter) : defaultQuality(0.5), // undetectQualityCoeff(0.75),
			dataODIM(defaultDataODIM), qualityODIM(defaultQualityODIM) {
		defaultDataODIM    = converter.dataODIM;
		defaultQualityODIM = converter.qualityODIM;
		init();
		//setLimits();
	};

	virtual
	~DataCoder(){};

	virtual
	bool decode(double & value) const;

	/// Converts storage data containing marker codes etc to natural scale.
	virtual
	bool decode(double & value, double & weight) const;

	/// Converts natural-scale data to storage data containing marker codes etc.
	/**
	 *   Does not use weight...
	 */
	virtual
	//void encode(double & value, double & weight) const;
	void encode(double & value, double & weight) const;

	virtual
	void encodeWeight(double & weight) const;

	/// Creates a naive quality field: data=1.0, undetect/nodata=0.0
	/**
	 *   Class-specific extension, ie not inherited from base class.
	void encodeDataToWeight(double value, double & weight) const;
	 */


	void init();

	/*
	inline
	double setLimits(){
		dataODIM
	}
	double minValue;
	double maxValue;
	*/

	/// If source data has no quality field, this value is applied for (detected) data.
	double defaultQuality;

	/// Physical value applied in locations marked with \c undetect . This should be compatible with the quantity, say DBZH or RHOHV.
	/**
	 *  In decoding, if undetectWeight>0, data marked \c undetect will replaced with this value.
	 *
	 *  Remember to call init() if dataODIM has been changed.
	 */
	double undetectValue; // "undetectValue"

	// in encoding, values lower than this value will be marked \c undetect .
	double minCodeValue;


	/// Quality, relative to data quality, applied in locations marked with \c undetect .
	/// NEW 2017: static (for Composite, Cappi)
	static double undetectQualityCoeff;

	const ODIM & dataODIM;
	const ODIM & qualityODIM;

	ODIM defaultDataODIM;
	ODIM defaultQualityODIM;

	inline
	virtual
	std::ostream & toOstr(std::ostream & ostr) const {
		ostr << "DataConverter defaultQuality=" << defaultQuality << ", minDetectableValue=" << undetectValue << ", undetectQualityCoeff=" << undetectQualityCoeff << ", \n";
		ostr << "\t data: " << EncodingODIM(dataODIM) << '\n';
		ostr << "\t q:    " << EncodingODIM(qualityODIM) << '\n';
		return ostr;
	}

protected:

	/// A physical value greater than undetectValue. In \i encoding, lower values will be marked \c undetect . See init().
	/**
	 *  Remember to call init() if dataODIM has been changed.
	 */
	double detectionThreshold;

};


} // rack::

#endif

// Rack
