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


#include <sstream>


#include <drain/image/Sampler.h>
#include <drain/util/Proj4.h>
#include <drain/image/AccumulatorGeo.h>
#include "data/ODIM.h"
#include "data/Data.h"
#include "Geometry.h"

// // using namespace std;

namespace rack {

//struct RadarDataSampler : drain::image::Sampler {
//}

/// Base class for utilities traversing radar data with Sampler.
/**
    \tparam OD - odim type (PolarODIM or CartesianODIM)

 */
template <class OD>
class RadarDataPicker : public drain::image::SamplePicker {
  public: // repl \n 
	/// Default constructor
	RadarDataPicker(drain::ReferenceMap & variableMap, const OD & odim) : drain::image::SamplePicker(variableMap), odim(odim) {

		variableMap.reference("i", current_i = 0);
		variableMap.reference("j", current_j = 0);
		variableMap.reference("LON", lon = 0.0);
		variableMap.reference("LAT", lat = 0.0);
		variableMap.reference("X", x = 0.0);
		variableMap.reference("Y", y = 0.0);

	}

	/// Input source type
	typedef SrcType<OD const> src_t;

	/// Input data type
	typedef PlainData<src_t> data_t;

	/// Input dataSet type
	typedef DataSetSrc<src_t> dataset_t;

	/// Map type compatible with Sampler
	typedef std::map<std::string, const data_t &> map_t;


	/// Reads a value, and scales it unless \c nodata or \c undetect.
	inline
	bool getValue(const data_t & data, double & value) const {

		value = data.data.template get<double>(this->current_i,this->current_j);
		if ((value != data.odim.nodata) && (value != data.odim.undetect)){
			value = data.odim.scaleForward(value);
			return true;
		}
		else {
			value = 0.0;
			return false;
		}
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

class PolarDataPicker : public RadarDataPicker<PolarODIM> {
  public: // repl \n 

	/// Reads a value, and scales it unless \c nodata or \c undetect.
	inline
	PolarDataPicker(drain::ReferenceMap & variableMap, const PolarODIM & odim) :
		RadarDataPicker<PolarODIM>(variableMap, odim), J2AZMDEG(360.0/static_cast<double>(odim.nrays)),
		current_sin(0), current_cos(0), current_azm(0), current_range(0)
		{

		proj.setLocation(odim.lon, odim.lat);
		proj.setProjectionDst("+proj=latlong +ellps=WGS84 +datum=WGS84");

		variableMap.reference("RANGE", current_range = 0.0);
		variableMap.reference("AZM", current_azm = 0.0);

		setSize(odim.nbins, odim.nrays);


	}


	/// Sets image position and calculates corresponding geographical coordinates.
	inline
	void setPosition(int i, int j) const {

		static const double DEG2RAD = M_PI/180.0;
		static const double RAD2DEG = 180.0/M_PI;

		this->current_i = i;
		this->current_j = j;
		current_range = odim.getBinDistance(i);
		current_azm   = static_cast<double>(j) * J2AZMDEG;  // odim.getAzm

		current_sin   = sin(current_azm*DEG2RAD);
		current_cos   = cos(current_azm*DEG2RAD);

		x = current_range*current_sin;
		y = current_range*current_cos;
		proj.projectFwd(current_range*current_sin, current_range*current_cos, lon, lat);
		lon *= RAD2DEG;
		lat *= RAD2DEG;

	}

	inline
	const RadarProj4 & getProjection() const { return proj; };

	virtual
	inline
	void writeHeader(const std::string & commentPrefix, std::ostream & ostr) const {
		ostr << commentPrefix << " proj='" <<  this->getProjection().getProjectionSrc() << "'\n";
		double lonLL, latLL, lonUR, latUR;
		this->getProjection().getBoundingBoxD(odim.getMaxRange(), lonLL, latLL, lonUR, latUR);
		ostr << commentPrefix << " bbox='" <<  lonLL << ',' <<  latLL << ',' << lonUR << ',' << latUR << "'\n";
	};

 protected:

	///  Converts the polar coordinates for a radar to geographical coordinates.
	RadarProj4 proj;

	/// Conversion from
	const double J2AZMDEG;

	/// Utility
	mutable double current_sin;
	/// Utility
	mutable double current_cos;
	/// Utility
	mutable double current_azm;
	/// Utility
	mutable double current_range;




};


class CartesianDataPicker : public RadarDataPicker<CartesianODIM> {
  public: // repl \n 
	/// Default constructor
	inline
	CartesianDataPicker(drain::ReferenceMap & variableMap, const CartesianODIM & odim) : RadarDataPicker<CartesianODIM>(variableMap,odim) {

		setSize(odim.xsize, odim.ysize);
		frame.setProjection(odim.projdef);
		frame.setGeometry(odim.xsize, odim.ysize);
		frame.setBoundingBoxD(odim.LL_lon, odim.LL_lat, odim.UR_lon, odim.UR_lat);

		variableMap.reference("j2", this->current_j2 = 0);
	}


	/// Sets image position (in 2D Cartesian space) and calculates corresponding geographical coordinates.
	inline
	void setPosition(int i, int j) const {

		this->current_i  = i;
		this->current_j  = j;
		this->current_j2 = this->height-1 - j;

		frame.pix2m(i,j, x,y);
		frame.pix2deg(i, j, lon, lat);

	}

	/// Frame for converting coordinates.
	GeoFrame frame;

	virtual
	inline
	void writeHeader(const std::string & commentPrefix, std::ostream & ostr) const {
		ostr << commentPrefix << " proj='" <<  frame.getProjection()   << "'\n";
		ostr << commentPrefix << " bbox='" <<  frame.getBoundingBoxD() << "'\n";
	};

};


}  // rack::


#endif /* RADAR_DATA_PICKER_H */

// Rack
