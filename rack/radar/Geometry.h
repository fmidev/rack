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
#ifndef GEOMETRY_RADAR_H_
#define GEOMETRY_RADAR_H_


#include <sstream>
#include <vector>
#include <drain/util/Proj4.h>
#include <drain/image/GeoFrame.h>

// // using namespace std;

namespace rack
{


// See also RadarProj @ Coordinates?

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

// Rack
