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


#include "drain/image/Sampler.h"
#include "drain/util/Proj4.h"
#include "drain/image/AccumulatorGeo.h"
#include "data/ODIM.h"
#include "data/Data.h"
#include "Geometry.h"

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
	RadarDataPicker(drain::ReferenceMap & variableMap, const OD & odim) : drain::image::SamplePicker(variableMap), odim(odim) {

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


public:

	/// Reads a value, and scales it unless \c nodata or \c undetect.
	inline
	PolarDataPicker(drain::ReferenceMap & variableMap, const PolarODIM & odim) :
		RadarDataPicker<PolarODIM>(variableMap, odim), J2AZMDEG(360.0/static_cast<double>(odim.geometry.height)),
		current_sin(0), current_cos(0), current_azm(0), current_range(0)
		{

		// drain::Logger mout(__FUNCTION__, __FUNCTION__);

		proj.setLocation(odim.lon, odim.lat);
		proj.setProjectionDst("+proj=latlong +ellps=WGS84 +datum=WGS84");

		variableMap.link("RANGE", current_range = 0.0);
		variableMap.link("AZM", current_azm = 0.0);

		setSize(odim.geometry.width, odim.geometry.height);

		infoMap["lon"] = odim.lon;
		infoMap["lat"] = odim.lat;
		Rectangle<double> bbox;
		// std::vector<double> bbox(4);
		//this->getProjection().getBoundingBoxD(odim.getMaxRange(), bbox[0], bbox[1], bbox[2], bbox[3]);
		this->getProjection().getBoundingBoxD(odim.getMaxRange(), bbox.lowerLeft.x, bbox.lowerLeft.y, bbox.upperRight.x, bbox.upperRight.y);
		infoMap["bbox"] = bbox.toVector();

	}


	/// Sets image position and calculates corresponding geographical coordinates.
	inline
	void setPosition(int i, int j) const {

		this->current_i = i;
		this->current_j = j;
		current_range = odim.getBinDistance(i);
		current_azm   = static_cast<double>(j) * J2AZMDEG;  // odim.getAzm

		current_sin   = sin(current_azm * drain::DEG2RAD);
		current_cos   = cos(current_azm * drain::DEG2RAD);

		x = current_range*current_sin;
		y = current_range*current_cos;
		proj.projectFwd(current_range*current_sin, current_range*current_cos, lon, lat);
		lon *= drain::RAD2DEG;
		lat *= drain::RAD2DEG;

	}

	inline
	const RadarProj4 & getProjection() const { return proj; };

	/*
	virtual inline
	void writeHeader(char commentPrefix, std::ostream & ostr) const {	};
	*/

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


		setSize(odim.geometry.width, odim.geometry.height);
		frame.setGeometry(odim.geometry.width, odim.geometry.height);
		if (!frame.geometryIsSet()){
			mout.warn() << "Array geometry undefined?" << mout.endl;
		}

		frame.setBoundingBoxD(odim.LL_lon, odim.LL_lat, odim.UR_lon, odim.UR_lat);
		if (!frame.bboxIsSet()){
			mout.warn() << "Bounding box undefined?" << mout.endl;
		}

		frame.setProjection(odim.projdef);
		if (!frame.projectionIsSet()){
			mout.warn() << "Projection undefined?" << mout.endl;
		}
		/* keep for a while
		if (!odim.projdef.empty()){
			frame.setProjection(odim.projdef);
		}
		else {
			mout.note() << odim << mout.endl;
			mout.warn() << "no projdef in metadata, cannot derive geographical coords (LON,LAT)" << mout.endl;
		}
		*/

		if (!frame.isDefined()){
			//mout.warn() << "Geo frame properties undefined, incomplete metadata?" << mout.endl;
			mout.note() << odim << mout.endl;
			mout.note() << frame << mout.endl;
			mout.note() << frame.getBoundingBoxR() << mout.endl;
			mout.warn() << "frame could not be defined, incomplete metadata? (above)" << mout.endl;
		}

		variableMap.link("j2", this->current_j2 = 0);

		variableMap.separator = ','; // bug? Was not initialized
		mout.debug() << "variableMap: " << variableMap << mout.endl;
		mout.debug() << "frame: " << frame << mout.endl;

		infoMap["bbox"] = frame.getBoundingBoxD().toVector();
		infoMap["proj"] = frame.getProjection();
		//infoMap["bbox"] = frame.getBoundingBoxD();

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

};


}  // rack::


#endif /* RADAR_DATA_PICKER_H */

// Rack
