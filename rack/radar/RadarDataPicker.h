/*

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack library for C++.

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

public:

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
	typedef DataSet<src_t> dataset_t;

	/// Map type compatible with Sampler
	typedef std::map<std::string, const data_t &> map_t;


	// Note: setPosition(int i, int j) defined in derived classes.

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

	/// Prints images geometry, buffer size and type information.
	void toOStr(std::ostream &ostr = std::cout) const {
		ostr << "RadarDataPicker " << this->width << "x" << this->height << "; " << this->ref << "; " <<  odim << '\n';
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
	p.toOStr(ostr);
	return ostr;
}

class PolarDataPicker : public RadarDataPicker<PolarODIM> {
  public: // repl \n 

	/// Reads a value, and scales it unless \c nodata or \c undetect.
	inline
	PolarDataPicker(drain::ReferenceMap & variableMap, const PolarODIM & odim) :
		RadarDataPicker<PolarODIM>(variableMap, odim), J2AZMDEG(360.0/static_cast<double>(odim.nrays)),
		current_sin(0), current_cos(0), current_azm(0), current_range(0)
		{

		// drain::Logger mout(__FUNCTION__, __FUNCTION__);

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

public:

	/// Default constructor
	inline
	CartesianDataPicker(drain::ReferenceMap & variableMap, const CartesianODIM & odim) : RadarDataPicker<CartesianODIM>(variableMap,odim) {

		drain::Logger mout(__FUNCTION__, __FUNCTION__);
		setSize(odim.xsize, odim.ysize);
		if (!odim.projdef.empty()){
			frame.setProjection(odim.projdef);
		}
		else {
			mout.note() << odim << mout.endl;
			mout.warn() << "no projdef in metadata, cannot derive geographical coords (LON,LAT)" << mout.endl;
		}
		frame.setGeometry(odim.xsize, odim.ysize);
		frame.setBoundingBoxD(odim.LL_lon, odim.LL_lat, odim.UR_lon, odim.UR_lat);

		if (!frame.isDefined()){
			mout.note() << odim << mout.endl;
			mout.note() << frame << mout.endl;
			mout.note() << frame.getBoundingBoxR() << mout.endl;
			mout.warn() << "frame could not be defined, incomplete metadata? (above)" << mout.endl;
		}

		variableMap.reference("j2", this->current_j2 = 0);

		variableMap.separator = ','; // bug? Was not initialized
		mout.debug() << "variableMap: " << variableMap << mout.endl;
		mout.debug() << "frame: " << frame << mout.endl;

	}


	/// Sets image position (in 2D Cartesian space) and calculates corresponding geographical coordinates.
	inline
	void setPosition(int i, int j) const {

		this->current_i  = i;
		this->current_j  = j;
		this->current_j2 = this->height-1 - j;

		//if (!frame.projR2M.isSet()){ // odim.projdef.empty()
		if (frame.projR2M.isSet()){ // odim.projdef.empty()
			frame.pix2m(i,j, x,y);
			frame.pix2deg(i, j, lon, lat);
		}
	}

	/// Frame for converting coordinates.
	GeoFrame frame;

	virtual
	inline
	void writeHeader(const std::string & commentPrefix, std::ostream & ostr) const {
		ostr << commentPrefix << " proj='" <<  frame.getProjection()   << "'\n";
		const drain::Rectangle<double> & bbox = frame.getBoundingBoxD();
		ostr << commentPrefix << " BBOX='" << bbox << "'\n";
		ostr << commentPrefix << " XRANGE='" <<  bbox.xLowerLeft << ':' <<  bbox.xUpperRight << "'\n";
		ostr << commentPrefix << " YRANGE='" <<  bbox.yLowerLeft << ':' <<  bbox.yUpperRight << "'\n";
	};

};


}  // rack::


#endif /* RADAR_DATA_PICKER_H */
