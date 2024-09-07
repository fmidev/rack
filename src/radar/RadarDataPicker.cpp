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

#include <drain/TypeUtils.h>
#include "RadarDataPicker.h"
#include "Geometry.h"


namespace rack {



PolarDataPicker::PolarDataPicker(drain::ReferenceMap2<> & variableMap, const PolarODIM & odim) :
						RadarDataPicker<PolarODIM>(variableMap, odim), J2AZMDEG(360.0/static_cast<double>(odim.area.height)), eta(odim.getElangleR()),
						current_sin(0), current_cos(0), current_azm(0), current_range(0), current_height(0) {

	// drain::Logger mout(__FUNCTION__, __FUNCTION__);
	//this->parameters.link("groundCoord", groundCoord);

	proj.setSiteLocationDeg(odim.lon, odim.lat);
	//proj.setProjectionDst("+proj=latlong +ellps=WGS84 +datum=WGS84"); // +type=crs ?
	proj.setProjectionDst("EPSG:4326");

	variableMap.link("RANGE", current_range);
	variableMap.link("AZM", current_azm);
	variableMap.link("HGHT", current_height);

	setSize(odim.area.width, odim.area.height);

	infoMap["lon"] = odim.lon;
	infoMap["lat"] = odim.lat;
	// if odim.angles ...
	// if needed: infoMap["elangle"] = odim.elangle;
	drain::Rectangle<double> bbox;

	this->getProjection().getBoundingBoxDeg(odim.getMaxRange(), bbox.lowerLeft.x, bbox.lowerLeft.y, bbox.upperRight.x, bbox.upperRight.y);

	infoMap["bbox"] = bbox.toVector();
}


/// Sets image position and calculates corresponding geographical coordinates.

void PolarDataPicker::setPosition(int i, int j) const {

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

	current_height = Geometry::heightFromEtaBeam(eta, current_range);
}



CartesianDataPicker::CartesianDataPicker(drain::ReferenceMap2<> & variableMap, const CartesianODIM & odim) : RadarDataPicker<CartesianODIM>(variableMap,odim) {

	drain::Logger mout(__FUNCTION__, __FUNCTION__);


	setSize(odim.area.width, odim.area.height);
	frame.setGeometry(odim.area.width, odim.area.height);
	if (!frame.geometryIsSet()){
		mout.warn("Array geometry undefined?" );
	}

	//frame.setBoundingBoxD(odim.LL_lon, odim.LL_lat, odim.UR_lon, odim.UR_lat);
	frame.setBoundingBoxD(odim.bboxD);
	if (!frame.bboxIsSet()){
		mout.warn("Bounding box undefined?" );
	}

	/* TODO: if (odim.epsg)
			frame.setProjection(odim.epsg)
		else
	 */
	frame.setProjection(odim.projdef);
	if (!frame.projectionIsSet()){
		mout.warn("Projection undefined?");
	}
	/* keep for a while
		if (!odim.projdef.empty()){
			frame.setProjection(odim.projdef);
		}
		else {
			mout.note(odim );
			mout.warn("no projdef in metadata, cannot derive geographical coords (LON,LAT)" );
		}
	 */

	if (!frame.isDefined()){
		//mout.warn("Geo frame properties undefined, incomplete metadata?" );
		mout.note(odim);
		mout.note(frame);
		mout.note(frame.getBoundingBoxRad());
		mout.warn("frame could not be defined, incomplete metadata? (above)");
	}

	variableMap.link("j2", this->current_j2 = 0);

	variableMap.separator = ','; // bug? Was not initialized
	mout.debug("variableMap: ", variableMap);
	mout.debug("frame: ", frame);

	infoMap["bbox"] = frame.getBoundingBoxDeg().toVector();
	infoMap["proj"] = frame.getProjection();
	infoMap["epsg"] = frame.projGeo2Native.getDst().getEPSG(); // NOTE probably unset...

}


/// Sets image position (in 2D Cartesian space) and calculates corresponding geographical coordinates.
void CartesianDataPicker::setPosition(int i, int j) const {

	this->current_i  = i;
	this->current_j  = j;
	this->current_j2 = this->height-1 - j;

	//if (!frame.projGeo2Native.isSet()){ // odim.projdef.empty()
	if (frame.projGeo2Native.isSet()){ // odim.projdef.empty()
		frame.pix2m(i,j, x,y);
		frame.pix2deg(i, j, lon, lat);
	}
}

}  // rack::


namespace drain {

/** This policy suggests typedef + spacialization for the actual classes, as well.
 *
 */

DRAIN_TYPENAME_DEF(rack::RadarDataPicker<rack::PolarODIM>);
DRAIN_TYPENAME_DEF(rack::RadarDataPicker<rack::CartesianODIM>);

/*
template <>
const std::string TypeName<rack::RadarDataPicker<rack::PolarODIM> >:: name("PolarDataPicker");

template <>
const std::string TypeName<rack::RadarDataPicker<rack::CartesianODIM> >:: name("CartesianDataPicker");
*/

/*
template <>
const std::string TypeName<rack::PolarDataPicker>:: name("PolarDataPicker");

template <>
const std::string TypeName<rack::CartesianDataPicker>:: name("CartesianDataPicker");
 */
}
