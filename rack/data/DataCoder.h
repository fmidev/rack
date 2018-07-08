/*

    Copyright 2001 - 2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

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
