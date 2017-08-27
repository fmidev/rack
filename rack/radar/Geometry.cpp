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
#include "Geometry.h"

#include <math.h>

namespace rack
{

double DEG2RAD(M_PI/180.0);
double RAD2DEG(180.0/M_PI);

int Geometry::EARTH_RADIUSi   = 6371000;
double Geometry::EARTH_RADIUS = 6371000.0;
double Geometry::EARTH_RADIUS_43 = 4.0/3.0 * EARTH_RADIUS;
	
Geometry::Geometry ()
{
	beamWidth = 1.0 * (M_PI/180.0);
}

Geometry::~Geometry()
{
}



    // angle is distance in radians from the beam center   OBSOLETE?
double Geometry::normalizedBeamPower(double angle){
    	float w2 = (float)(beamWidth * beamWidth);
    	return w2 / (w2 + angle*angle);
}

    //  Find upper and lower scan  OBSOLETE?
void Geometry::findClosestElevations(const float &elevationAngle,
    	int &elevationIndexLower, float &elevationAngleLower, 
    	int &elevationIndexUpper, float &elevationAngleUpper
    	){
        // Find upper and lower scan
    
    	elevationAngleLower = -M_PI/2.0;
    	elevationAngleUpper = +M_PI/2.0;
    	
    	elevationIndexLower = -1;
    	elevationIndexUpper = -1;
    
    	double e;
    	for (std::vector<float>::size_type i=0; i < elevationAngles.size(); i++){

    		e = elevationAngles[i];
    		
    		if ((e > elevationAngleLower) && (e <= elevationAngle)) {
    			elevationIndexLower = i;
    			elevationAngleLower = e;
    		}

    		if ((e < elevationAngleUpper) && (e >= elevationAngle)) {
    			elevationIndexUpper = i;
    			elevationAngleUpper = e;
    		}
    	};
 
    }



//    deer::image::image<T> *my_source_image;


    /*! \brief The altitude of a point at beam distance (b)
     *  and elevation (eta).
     *
     *  By cosine rule:
     *   c² = a² + b² - 2ab·cos(gamma);
     */
    double Geometry::heightFromEtaBeam(float eta,float b){
    	double a = EARTH_RADIUS_43;
    	return sqrt( a*a + b*b - 2.0*a*b*cos(M_PI/2.0 + eta) ) - a;
    }

    /** The altitude of a point above ground.
     * 
     *  @param eta  Elevation in radians
     *  @param beta Ground angle 
     * 
     *  By sine rule:
     *  \f[
     *  \sin(\gamma)/c = \sin(\alpha)/a
     *  \Leftrightarrow  c = a · \frac{\sin(\gamma)}{\sin(\alpha)}
     *  \Leftrightarrow  h = c-a = a·( \frac{\sin(\gamma)}{\sin(\alpha)} - 1)
     *  \f]
     *
     *  Always
     *  \f[ \sin(\gamma) = \sin(\eta+\pi/2) = \cos(\eta)
     *  \f]
     *
     *  Because \f$ \alpha = \pi - \gamma - \beta \f$ in a triangle,
     *  \f[
     *    \sin(\alpha) = \sin(\pi-\gamma-\beta) = \sin(\beta+\gamma)
     *             = \sin(\beta + \eta+\pi/2) = \cos(\beta + \eta)
     *  \f]
     *
     *  Finally,
     *  \f[
     *   h =  a·( \frac{\cos(\eta)}{\cos(\beta + \eta)} - 1)
     *  \f]
     */
    double Geometry::heightFromEtaBeta(double eta, double beta){
    	// a = EARTH_RADIUS_43
    	return EARTH_RADIUS_43 * (cos(eta)/cos(beta + eta) - 1.0);
    }
  
    /** The altitude of a point at ground distance g and elevation eta.
     * 
     *  @param eta Elevation in radians
     *  @param g   Ground distance in metres
     * 
     *  @see #heightFromEtaBeta(double, double) which is preferred as being faster.
     */
    double Geometry::heightFromEtaGround(double eta, double g){
    	double beta = g/EARTH_RADIUS_43;
    	// a = EARTH_RADIUS_43
    	return EARTH_RADIUS_43 * (cos(eta)/cos(eta + beta) - 1.0);
    }

    /*! The on-beam distance at ground angle \c beta and altitude \c h.
     *
     *  By cosine rule:
     *  \f[ 
     *       b^2 = a^2 + c^2 - 2ac\cos\beta
     *           = a^2 + (a^2 + 2ah+  h^2) - 2a(a + h)\cos\beta
     *           = 2a(a+h) + h^2 - 2a(a + h)\cos\beta
     *           = 2a(a+h)(1-\cos\beta) + h^2
     *  \f]
     */
    //  inline 
    double Geometry::beamFromBetaH(double beta,double h){
    	double a = EARTH_RADIUS_43;
    	return sqrt((2.0*a)*(a+h)*(1.0-cos(beta)) + h*h);
    }


    // OK THIS FAR?

    /*! \brief The on-beam distance at (elevation) and (altitude).
     *
     *  By sine rule:
     *  sin(gamma)/c = sin(beta)/b  => b = sin(beta)*c/sin(gamma).
     */
    //  static
    //  inline 
    double Geometry::beamFromEtaH(double eta,double h){
    	double c = EARTH_RADIUS_43 + h;
    	double a = EARTH_RADIUS_43;
    	double gamma = eta + (M_PI/2.0);
    	double beta = M_PI - gamma - asin(a*sin(gamma)/c);
    	return sin(beta) * c / sin(gamma); // / my_binDepth;
    }



    /*! \brief The on-beam distance at (elevation) and ground angle
     *  (beta).
     *
     *  By sine rule:
     *  sin(beta)/b = sin(alpha)/a
     *  => b = sin(beta) * a/sin(alpha).
     */
    //  inline
    double Geometry::beamFromEtaBeta(double eta,double beta){
    	double a = EARTH_RADIUS_43;
    	/// Angle(RADAR,BIN)
    	//    double beta = g / EARTH_RADIUS_43; 
    	/// Angle(BIN->RADAR,BIN->GROUND_POINT)
    	double alpha = M_PI - (eta + (M_PI/2.0)) - beta;
    	return sin(beta) * a / sin(alpha);
    }


    /*! \brief The on-beam distance at (elevation) and ground distance
     *  (groundDist).
     *
     *  Let b = beam distance and a = EARTH_RADIUS_. 
     *  By sine rule:
     *  sin(beta)/b = sin(alpha)/a
     *  => b = sin(beta) * a/sin(alpha).
     */
    double Geometry::beamFromEtaGround(float eta,float g){

    	/// Angle(RADAR,BIN)
    	double beta = g / EARTH_RADIUS_43; 

    	/// Angle(BIN->RADAR,BIN->GROUND_POINT)
    	double alpha = static_cast<double>(M_PI - (eta + (M_PI/2.0)) - beta);

    	return sin(beta) * EARTH_RADIUS_43 / sin(alpha);

    }
  
  
    /*! \brief
     * Given elevation in radians and on-beam distance, returns the distance
     * from radar to the ground point under the bin.
     */

    double Geometry::groundFromEtaBeam(double eta, double beam){
    	//    float x,y;
    	double x = beam * (float)cos(eta);
    	double y = beam * (float)sin(eta);
    	return EARTH_RADIUS_43 * atan(x / (y + EARTH_RADIUS_43));
    }


    /// Given elevation angle \c eta and altitude \c h, returns the ground angle.
    /*!
     * By sine rule:
     *
     * \f[
     * \frac{\sin\alpha}{a} = \frac{\sin\gamma}{c} = \frac{\sin\gamma}{a+h}
     * \Leftrightarrow
     * \sin\alpha =\frac{a}{a+h}\sin\gamma
     * \Rightarrow
     * \gamma = \pi - \gamma - \alpha = \pi - (\eta+\pi/2) - \arcsin \left( \frac{a}{a+h}\sin(\eta+\pi/2) \right)
     * \f]
     */
    double Geometry::gammaFromEtaH(double eta,double h){
    	return M_PI_2 - eta - asin(EARTH_RADIUS_43/(EARTH_RADIUS_43+h)*sin(eta+M_PI_2));
    	//return 180.0/M_PI * asin(EARTH_RADIUS_43/(EARTH_RADIUS_43+h)*sin(eta+M_PI_2));
    }

    /// Given ground angle \c beta and altitude \c h, returns the elevation angle.
    /*! 
     * By sine rule:
     *
     * \f[
     * \sin(\beta)/b = \sin(\gamma)/c 
     * \Leftrightarrow  
     * \sin(\beta) * c/b = \sin(\gamma) 
     * /// = \sin(\pi-\gamma) = 
     * /// \sin(\pi-(\eta + \pi/2)) = \sin(\pi/2-\eta) = \cos(\eta)
     * \sin(\pi/2+\eta) = \sin(\pi-(\pi/2+\eta)) = \sin(\pi/2-\eta) = \cos(\eta)
     * \Rightarrow  
     * \eta = \arccos( \sin(\beta) * c/b )  // WRONG! ALWAYS POSITIVE!
     * \f]
     */
    /*! By cosine rule:
     *  \f[
     *  c^2 = a^2 + b^2 - 2ab\cos\gamma 
     *  \Leftrightarrow
     *  \cos\gamma = (a^2 + b^2 - c^2) / 2ab = 
     *  =  (a^2 + (a^2 + c^2 - 2ac\cos\beta) - c^2) / 2ab
     *  =  (2a^2 - 2ac\cos\beta) / 2ab = (a-c\cos\beta) / b
     *  \f]
     */
    double Geometry::etaFromBetaH(double beta,double h){
    	//
    	double a  = EARTH_RADIUS_43;
    	//
    	double a2 = a*a; //
    	// FINAL:
    	double c    = h + EARTH_RADIUS_43;
    	double c2   = c*c; //
    	//    double beta = g/EARTH_RADIUS_43;
    	//    double b2   = a2 + c2 - 2.0*a*c*cos( beta );
    	double b   = sqrt(a2 + c2 - 2.0*a*c*cos( beta ));

    	//    return asin( (a2 + b*b - c2) / (2 * a * b) ); // feb 2005 

    	return acos( (a - (a+h)*cos(beta)) / b) - (M_PI/2.0); // feb 2005 
    }

    double Geometry::bFromGH(double g,double h){
    	return beamFromBetaH(g/EARTH_RADIUS_43,h);
    }

    /// Given ground distance \c g and altitude \c h, returns elevation angle.
    //  Feb 2005
    double Geometry::etaFromGH(double g,double h){
    	return etaFromBetaH(g/EARTH_RADIUS_43,h);
    }




}

// Rack
