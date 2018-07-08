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
#ifndef GEOMETRY_RADAR_H_
#define GEOMETRY_RADAR_H_


#include <sstream>
#include <vector>
#include <drain/util/Proj4.h>
#include <drain/image/GeoFrame.h>

// // using namespace std;

namespace rack
{



/// Equals M_PI/180.0
//extern double DEG2RAD;

/// Equals 180.0/M_PI
//extern double RAD2DEG;

class RadarProj4 : public drain::Proj4 {

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
		sstr << "+proj=aeqd" << " +lon_0=" << lon << " +lat_0=" << lat << " +ellps=WGS84";
		setProjectionSrc(sstr.str());
	};

	/// Bounding box in radians
	inline
	void getBoundingBox(double range, double & lonLL, double & latLL, double & lonUR, double & latUR) const {
		projectFwd(5.0/4.0*M_PI, sqrt(2.0)*range, lonLL, latLL);
		projectFwd(1.0/4.0*M_PI, sqrt(2.0)*range, lonUR, latUR);
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

	
    /*! 
     *
     * \image latex radar-geometry-fig.pdf
     * \image html  radar-geometry-fig.png
     *
     *  Distances in meters, angles in radians.
     *
     *   - \f$a\f$: Earth radius, the distance from the Earth's center to the radar 
     *   - \f$b\f$: beam distance; the distance from the radar to the bin
     *   - \f$c\f$: distance from the Earth's center to the bin 
     *
     *   - \f$h = c - a\f$: altitude from ground to beam point
     *   - \f$g\f$: ground distance; surface distance 
     *        from the radar to the ground point
     *
     *  - eta \f$ \eta \f$: beam elevation
     *
     *  - alpha \f$ \alpha = \angle(b,c)\f$: "sky angle", 
     *  - beta  \f$ \beta  = \angle(a,c)\f$: "ground angle"
     *  - gamma \f$ \gamma = \angle(a,b) = \eta + \pi/2 \f$: "radar angle" 
     *
     *  - r = radian
     *  - d = degree
     *
     * Cosine rule:
     *  \f[
     *      c^2 = a^2 + b^2 - 2ab·\cos(\gamma);
     *  \f]
     *
     * Sine rule:
     *  \f[
     *      \frac{\sin\alpha}{a} = \frac{\sin\beta}{b} = \frac{\sin\gamma}{c}
     *  \f]
     *
     *
     */
class Geometry
{
public:
	Geometry();
	virtual ~Geometry();
	
	/// Altitude, given elevation [rad] and bin distance [m].
	static
	double heightFromEtaBeam(double eta, double b);

	/// Altitude, given elevation [rad] and ground angle [rad].
    static
    double heightFromEtaBeta(double eta, double beta);

	/// Altitude, given elevation [rad] and ground distance [m].
    static
    double heightFromEtaGround(double eta, double g);

    static inline
    double betaFromGround(double g){ return g / EARTH_RADIUS_43; }; // SHOULD BE EARTH_RADIUS

    static
    double beamFromBetaH(double beta,double h);

    static
    double beamFromEtaH(double eta,double h);

    static
    double beamFromEtaBeta(double eta,double beta);

    static
    double beamFromEtaGround(float eta,float g);

    //inline unsigned int binFromEtaBeta(double eta,double beta){ 
    //	return static_cast<unsigned int>(binDepth/2 + binDepth*beamFromEtaBeta(beta,eta));};
    
    static double groundFromEtaBeam(double eta, double beam);
    static double gammaFromEtaH(double eta, double height);

    static inline
    double groundFromEtaH(double eta,double h){ return EARTH_RADIUS_43*gammaFromEtaH(eta,h); }; //EARTH_RADIUS

    static
    double etaFromBetaH(double beta,double h);
    
    double bFromGH(double g,double h);

    static
    double etaFromGH(double g,double h);
	
	
	double normalizedBeamPower(double angle);
    
    void findClosestElevations(const float &elevationAngle,
    	int &elevationIndexLower,float &elevationAngleLower,int &elevationIndexUpper, float &elevationAngleUpper);
    
	float beamWidth;
	
	// new
	//static int    EARTH_RADIUSi;
	//static double EARTH_RADIUS;
    static double EARTH_RADIUS_43;


	/// Note: radians!
	std::vector<float> elevationAngles;
	std::vector<unsigned int> bins;
};

}  // rack::


#endif /*GEOMETRY_H_*/
