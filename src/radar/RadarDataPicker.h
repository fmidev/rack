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
#ifndef RADAR_DATA_PICKER_H
#define RADAR_DATA_PICKER_H


#include <drain/TypeUtils.h>
#include <sstream>


#include <drain/image/Sampler.h>
#include <drain/image/AccumulatorGeo.h>
#include "data/Data.h"

// ##include "Geometry.h"
#include "RadarProj.h"

// // using namespace std;

namespace rack {

/// Base class for utilities traversing radar data with Sampler.
/**
    \tparam OD - odim type (PolarODIM or CartesianODIM)


	TODO: min/max coord control, like CoordHandler for checking min and max.  => drain::Sampler
 */
template <class OD>
class RadarDataPicker : public drain::image::SamplePicker {

public:

	/// Default constructor
	RadarDataPicker(drain::ReferenceMap2<> & variableMap, const OD & odim) : drain::image::SamplePicker(variableMap), odim(odim) {

		variableMap.link("i", current_i = 0);
		variableMap.link("j", current_j = 0);
		variableMap.link("LON", lon = 0.0);
		variableMap.link("LAT", lat = 0.0);
		variableMap.link("X", x = 0.0);
		variableMap.link("Y", y = 0.0);

	}

	/// Input source type
	typedef SrcType<OD const> src_t;

	/// Input data type
	typedef PlainData<src_t> data_t;

	/// Input dataSet type
	typedef DataSet<src_t> dataset_t;

	/// Map type compatible with Sampler
	typedef std::map<std::string, const data_t &> map_t;

	// setPosition(i,j) in derived classes.

	/// Reads a value, and scales it unless \c nodata or \c undetect.
	inline
	bool getValue(const data_t & data, double & value) const {

		value = data.data.template get<double>(this->current_i,this->current_j);
		if ((value != data.odim.nodata) && (value != data.odim.undetect)){
			value = data.odim.scaleForward(value);
			return true;
		}
		else {
			// Keep raw, maybe odd marker values of nodata/undetect!
			// value = 0.0;
			return false;
		}
	}

	/// Prints images geometry, buffer size and type information.
	void toStream(std::ostream &ostr = std::cout) const {
		// "RadarDataPicker "
		ostr << drain::TypeName<rack::RadarDataPicker<OD> >::name;
		ostr << this->width << "x" << this->height << "; " << this->ref << "; " <<  odim << '\n';
	}



protected:

	/// Metric coordinate corresponding to longitudinal coordinate
	mutable double x;
	/// Metric coordinate corresponding to latitudinal coordinate
	mutable double y;

	/// Longitudinal coordinate (degrees)
	mutable double lon;
	/// Latitudinal coordinate (degrees)
	mutable double lat;

	/// Metadata for navigating and scaling grid data
	const OD & odim;


};

template <class OD>
inline
std::ostream & operator<<(std::ostream &ostr, const RadarDataPicker<OD> & p){
	p.toStream(ostr);
	return ostr;
}

}


namespace rack {

class PolarDataPicker : public RadarDataPicker<PolarODIM> {


public:

	/// Reads a value, and scales it unless \c nodata or \c undetect.

	PolarDataPicker(drain::ReferenceMap2<> & variableMap, const PolarODIM & odim);

	/// Sets image position and calculates corresponding geographical coordinates.
	virtual
	void setPosition(int i, int j) const;

	inline
	const RadarProj & getProjection() const {
		return proj;
	};

	/*
	virtual inline
	void writeHeader(char commentPrefix, std::ostream & ostr) const {	};
	*/

	/// If true, i is looped as a ground coordinate.
	bool groundCoord = false;

 protected:

	///  Converts the polar coordinates for a radar to geographical coordinates.
	//RadarProj4 proj;
	RadarProj proj;

	/// Conversion from
	const double J2AZMDEG;

	// Elevation angle in radians
	const double eta;

	/// Utility
	mutable double current_sin;
	/// Utility
	mutable double current_cos;
	/// Utility
	mutable double current_azm;

	/// Distance from the radar
	mutable double current_range;

	/// Altitude from the mean sea level
	mutable double current_height;

};


class CartesianDataPicker : public RadarDataPicker<CartesianODIM> {

public:

	/// Default constructor
	CartesianDataPicker(drain::ReferenceMap2<> & variableMap, const CartesianODIM & odim);

	/// Sets image position (in 2D Cartesian space) and calculates corresponding geographical coordinates.
	void setPosition(int i, int j) const;

	/// Frame for converting coordinates.
	GeoFrame frame;

};


}  // rack::

namespace drain {

DRAIN_TYPENAME(rack::RadarDataPicker<rack::PolarODIM>);
DRAIN_TYPENAME(rack::RadarDataPicker<rack::CartesianODIM>);

} // Rack


#endif /* RADAR_DATA_PICKER_H */
