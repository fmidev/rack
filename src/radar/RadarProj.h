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
#ifndef RADAR_PROJ_
#define RADAR_PROJ_ "RadarProj 2023 Markus Peura fmi.fi"

#include <math.h>

#include <iostream>
#include <ostream>
#include <sstream>

#include "drain/util/Proj6.h"
#include "drain/util/Rectangle.h"

#include "Constants.h"
//#include "radar.h"

// See also RadarProj4 @ Geometry?

namespace rack {

/** Deprecated. Use RadarProj(6) below.
 *
 */
class RadarProj4 : public drain::Proj6 {

public:

	/// Sets location of the radar and the azimuthal equidistant (AEQD) projection accordingly.
	/*
	 *  \param lon - longitude of the location, in degrees
	 *  \param lat - longitude of the location, in degrees
	 */
	RadarProj4(double lon=0.0, double lat=0.0){
		setLocation(lon, lat);
	}

	/// Sets location of the radar and the azimuthal equidistant (AEQD) projection accordingly.
	/*
	 *  \param lon - longitude of the location, in degrees
	 *  \param lat - longitude of the location, in degrees
	 */
	inline
	void setLocation(double lon, double lat){
		std::stringstream sstr;
		sstr << "+proj=aeqd" << " +lon_0=" << lon << " +lat_0=" << lat << " +ellps=WGS84 +type=crs";
		setProjectionSrc(sstr.str());
	};

	/// Bounding box in radians
	inline
	void getBoundingBox(double range, double & lonLL, double & latLL, double & lonUR, double & latUR) const {
		projectFwd(5.0/4.0*M_PI, ::sqrt(2.0)*range, lonLL, latLL);
		projectFwd(1.0/4.0*M_PI, ::sqrt(2.0)*range, lonUR, latUR);
	}

	/// Bounding box in degrees
	inline
	void getBoundingBoxD(double range, double & lonLL, double & latLL, double & lonUR, double & latUR) const {
		getBoundingBox(range, lonLL, latLL, lonUR, latUR);
		lonLL *= drain::RAD2DEG;
		latLL *= drain::RAD2DEG;
		lonUR *= drain::RAD2DEG;
		latUR *= drain::RAD2DEG;
	}



};

class RadarProj : public drain::Proj6 {

public:

	/// Sets location of the radar and the azimuthal equidistant (AEQD) projection accordingly.
	/*
	 *  \param lon - longitude of the location, in degrees
	 *  \param lat - longitude of the location, in degrees
	 */
	RadarProj(double lonDeg=0.0, double latDeg=0.0){
		setSiteLocationDeg(lonDeg, latDeg);
	}

	/// Sets location of the radar and the azimuthal equidistant (AEQD) projection accordingly.
	/*
	 *  \param lon - longitude of the location, in degrees
	 *  \param lat - longitude of the location, in degrees
	inline
	void setLocation(double lon, double lat){
		std::stringstream sstr;
		sstr << "+proj=aeqd" << " +lon_0=" << lon << " +lat_0=" << lat << " +ellps=WGS84 +type=crs"; //
		setProjectionSrc(sstr.str());
	};
	*/

	/// Sets location of the radar and the azimuthal equidistant (AEQD) projection accordingly.
	/*
	 *  \param lon - longitude of the location, in degrees
	 *  \param lat - longitude of the location, in degrees
	 */
	inline
	void setSiteLocationDeg(double lon, double lat){
		std::stringstream s;
		s << "+proj=aeqd" << " +lon_0=" << lon << " +lat_0=" << lat << " +ellps=WGS84 +type=crs"; //  +type=crs
		setProjectionSrc(s.str());
	}

	/// Sets location of the radar and the azimuthal equidistant (AEQD) projection accordingly.
	/*
	 *  \param lon - longitude of the location in radians
	 *  \param lat - longitude of the location in degrees
	 */
	inline
	void setSiteLocationRad(double lon, double lat){
		setSiteLocationDeg(lon*drain::RAD2DEG, lat*drain::RAD2DEG);
	}

	/// Bounding box in radians
	inline
	void getBoundingBox(double range, double & lonLL, double & latLL, double & lonUR, double & latUR) const {
		projectFwd(5.0/4.0*M_PI, ::sqrt(2.0)*range, lonLL, latLL);
		projectFwd(1.0/4.0*M_PI, ::sqrt(2.0)*range, lonUR, latUR);
	}

	/// Bounding box in degrees
	inline
	void getBoundingBoxD(double range, double & lonLL, double & latLL, double & lonUR, double & latUR) const {
		getBoundingBox(range, lonLL, latLL, lonUR, latUR);
		lonLL *= drain::RAD2DEG;
		latLL *= drain::RAD2DEG;
		lonUR *= drain::RAD2DEG;
		latUR *= drain::RAD2DEG;
	}


	inline
	void setLatLonProjection(){
		setProjectionDst("+proj=latlong +ellps=WGS84 +datum=WGS84 +no_defs");
	}

	/// Given radar's range, returns the metric bounding box using the current projection.
	void determineBoundingBoxM(double range, double & xLL, double & yLL, double & xUR, double & yUR) const;

	/// Given radar's range, returns the metric bounding box using the current projection.
	inline
	void determineBoundingBoxM(double range, drain::Rectangle<double> & bbox) const {
		determineBoundingBoxM(range, bbox.lowerLeft.x, bbox.lowerLeft.y, bbox.upperRight.x, bbox.upperRight.y);
	}

	/// Given radar's range, returns the metric bounding box using the current projection.
	//void determineBoundingBoxD(double range, double & xLL, double & yLL, double & xUR, double & yUR) const;


	/// Given radar's range, returns the metric bounding box using the current projection.
	/*
	inline
	void determineBoundingBoxD(double range, drain::Rectangle<double> & bbox) const {
		determineBoundingBoxD(range, bbox.lowerLeft.x, bbox.lowerLeft.y, bbox.upperRight.x, bbox.upperRight.y);
	}
	*/


};


/// Simple spherical coordinate computation. Does not handle projections, but earth coords.
//  DEPRECATED
/*!
     \image latex radar-coordinates-fig.pdf
     \image html  radar-coordinates-fig.png

    \section Variablenames Variable names

      - \f$\phi\f$, phi:  longitude of radar site (in radians)
      - \f$\theta\f$, theta: latitude of radar site (in radians)
      - \f$\alpha\f$, alpha: azimuth angle of the radar beam, \f$+\pi/2\f$=North
      - \f$r\f$:  distance to the surface point
      - \f$\boldmath{e}_{i}=(e_{i1}e_{i1}e_{i1})\f$: unit vectors at the site


     Note. This is a model for ideal sphere, defined as
     \code
     "+proj=longlat +R=6371000 +no_defs";
     \endcode
     where R is the radius of the Earth. Use getDatumString() to get the actual std::string.

     Note that this is not equivalent with
     \code
     EPSG:4326 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
     \endcode
 */

} // ::rack

#endif

// Rack
