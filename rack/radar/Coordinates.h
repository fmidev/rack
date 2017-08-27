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
		// static const double DEG2RAD = M_PI/180.0;
		// setSiteLocation(lon*DEG2RAD, lat*DEG2RAD);
		std::stringstream s;
		s << "+proj=aeqd" << " +lon_0=" << lon << " +lat_0=" << lat << " +ellps=WGS84";
		setProjectionSrc(s.str());
	}

	/// Sets the site in radians.
	void setSiteLocation(double lon, double lat){
		static const double RAD2DEG = 180.0/M_PI;
		setSiteLocationDeg(lon*RAD2DEG, lat*RAD2DEG);
	}

	inline
	void setLatLonProjection(){
		setProjectionDst("+proj=latlong +ellps=WGS84 +datum=WGS84");
	}

	/// Given radar's range, returns the metric bounding box using the current projection.
	void determineBoundingBoxM(double range, double & xLL, double & yLL, double & xUR, double & yUR) const;

	/// Given radar's range, returns the metric bounding box using the current projection.
	inline
	void determineBoundingBoxM(double range, drain::Rectangle<double> & bbox) const {
		determineBoundingBoxM(range, bbox.xLowerLeft, bbox.yLowerLeft, bbox.xUpperRight, bbox.yUpperRight);
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

// Rack
