/*

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#ifndef RADAR__COORDINATES_ 
#define RADAR__COORDINATES_ "radar__coordinates 0.2, May 16 2011 Markus.Peura@fmi.fi"

#include <math.h>

#include <iostream>
#include <ostream>
#include <sstream>

#include <drain/util/Proj4.h>
#include <drain/util/Rectangle.h>

#include "Constants.h"
//#include "radar.h"


namespace rack {

class RadarProj : public drain::Proj4 {

public:

	inline
	void setSiteLocationDeg(double lon, double lat){
		std::stringstream s;
		s << "+proj=aeqd" << " +lon_0=" << lon << " +lat_0=" << lat << " +ellps=WGS84";
		setProjectionSrc(s.str());
	}

	/// Sets the site in radians.
	void setSiteLocation(double lon, double lat){
		setSiteLocationDeg(lon*drain::RAD2DEG, lat*drain::RAD2DEG);
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
		determineBoundingBoxM(range, bbox.xLowerLeft, bbox.yLowerLeft, bbox.xUpperRight, bbox.yUpperRight);
	}



	/// Given radar's range, returns the metric bounding box using the current projection.
	void determineBoundingBoxD(double range, double & xLL, double & yLL, double & xUR, double & yUR) const;


	/// Given radar's range, returns the metric bounding box using the current projection.
	inline
	void determineBoundingBoxD(double range, drain::Rectangle<double> & bbox) const {
		determineBoundingBoxD(range, bbox.xLowerLeft, bbox.yLowerLeft, bbox.xUpperRight, bbox.yUpperRight);
	}


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
  class Coordinates {
  public:


	  Coordinates();

	  virtual
	 ~Coordinates(){};

    //    coordinator();

    /// Radar site latitude and longitude in radians.
    void setOrigin(const double &theta,const double &phi); 

    /// Set target projection.
    void setProjection(const std::string &s);

    // inline
    void setOriginDeg(const double &lat,const double &lon);
      //      origin(lat*M_PI/180.0 , lon*M_PI/180.0);
      //    }; 
  //const double &lat,const double &lon);

    /// \param alpha is azimuth in radians, \param range in metres.
    void setBinPosition(const double &alpha, const float &range);
   
    // \param alpha is azimuth in radians, \param range in metres.
    //void setBinPosition(double alpha, float range);
   
	/// Determines the bounding box (in degrees) of the circular radar measurement area.
    void getBoundingBox(float range,double &latMin,double &lonMin,double &latMax,double &lonMax);

    /// Info
    void info(std::ostream &ostr = std::cout);

    // site positional
    // double cos_theta;
 
    /// Radar position vector (from Earth center to surface
    // (Site normal unit vector not needed as such)
    double p01, p02, p03;

    /// Earth centered coordinates [p_1 p_2 p_3] of the current bin position.
    double p1, p2, p3;

    /// Elelements of the East pointing site unit vector [e11 e12 e13].
    double e11, e12, e13;

    ///  Elelements of the North pointing site unit vector [e21 e22 e23].
    double e21, e22, e23;

    /// Bin latitude in radians after calling bin_position().
    mutable double thetaBin;

    /// Bin longitude in radians after calling bin_position().
    mutable double phiBin;

    ///  Bin latitude in degrees after calling bin_position().
    inline
    double binLatitudeDeg(){ return thetaBin/M_PI*180.0;};
				//phi_bin/M_PI*180.0;};
				//

    ///  Bin longitude in degrees after calling bin_position().
    inline
    double binLongitudeDeg(){ return phiBin/M_PI*180.0;};
    //    theta_bin/M_PI*180.0;};;
    //
    drain::Proj4 proj;


    inline
    virtual std::string getProjectionString(){
    	std::stringstream sstr;
    	sstr << "+proj=longlat +R=" << EARTH_RADIUS << std::string(" +no_defs");
    	return sstr.str();
    };
  };

} // ::rack

#endif
