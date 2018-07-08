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
#include <math.h>
#include <limits>
#include <drain/util/Log.h>
#include "Geometry.h"
#include "Coordinates.h"

// g++ -I. -c radar__coorz.cpp 

// huom. virhe tekstin kohdassa r/2PI R => r/R
// 
// // using namespace std;

namespace rack {


void RadarProj::determineBoundingBoxM(double range, double & xLL, double & yLL, double & xUR, double & yUR) const {

	drain::Logger mout("RadarProj", __FUNCTION__);

	mout.debug(1) << "start\n" << *this << mout.endl;

	mout.debug(1) << "range=" << range << mout.endl;

	xLL = +std::numeric_limits<double>::max();
	yLL = +std::numeric_limits<double>::max();
	xUR = -std::numeric_limits<double>::max();
	yUR = -std::numeric_limits<double>::max();

	double azimuth;
	double x,y;

	const unsigned int azimuthStep = 12;
	for (int a = 0; a < 360; a += azimuthStep) {

		azimuth = static_cast<double>(a) * drain::DEG2RAD;
		projectFwd(range*sin(azimuth), range*cos(azimuth), x, y);

		mout.debug(5) << x << ',' << y << mout.endl;

		xLL = std::min(x,xLL);
		yLL = std::min(y,yLL);
		xUR = std::max(x,xUR);
		yUR = std::max(y,yUR);
	}

	mout.debug(1) << xLL << ',' << yLL << ':' << xUR << ',' << yUR << mout.endl;


}

void RadarProj::determineBoundingBoxD(double range, double & xLL, double & yLL, double & xUR, double & yUR) const {

	drain::Logger mout("RadarProj", __FUNCTION__);

	mout.debug(1) << "start " << *this << mout.endl;

	mout.debug(1) << "range=" << range << mout.endl;

	xLL = +std::numeric_limits<double>::max();
	yLL = +std::numeric_limits<double>::max();
	xUR = -std::numeric_limits<double>::max();
	yUR = -std::numeric_limits<double>::max();


	double azimuth;
	double x,y;

	const unsigned int azimuthStep = 12;
	for (int a = 0; a < 360; a += azimuthStep) {

		azimuth = static_cast<double>(a) * drain::DEG2RAD;
		x = range*sin(azimuth);
		y = range*cos(azimuth);
		//projectFwd(range*sin(azimuth), range*cos(azimuth), x, y);
		mout.debug(5) << x << ',' << y << mout.endl;

		xLL = std::min(x,xLL);
		yLL = std::min(y,yLL);
		xUR = std::max(x,xUR);
		yUR = std::max(y,yUR);
	}

	mout.debug(1) << xLL << ',' << yLL << ':' << xUR << ',' << yUR << mout.endl;

}



Coordinates::Coordinates(){
	std::stringstream sstr;
	sstr << "+proj=longlat +R=" << EARTH_RADIUS << " +no_defs";
	proj.setProjectionSrc(sstr.str());
	proj.setProjectionDst("+proj=longlat +datum=WGS84 +no_defs");
}

void Coordinates::setProjection(const std::string &s){
	proj.setProjectionDst(s);
}

/*! \par theta latitude in radians, \par phi longitude in radians
    To speed up the computation of bin locations, some internal variables are
    set at this stage.
 */
void Coordinates::setOrigin(const double &theta,const double &phi){
	/*
    theta  = lat/180.0*M_PI; // latitude in radians
    phi    = lon/180.0*M_PI; // longitude in radians
	 */

	const double sin_theta = sin(theta);
	const double cos_theta = cos(theta);
	//   cos_theta = cos(theta);
	const double sin_phi   = sin(phi);
	const double cos_phi   = cos(phi);



	// Radar position vector
	// (Site normal unit vector not needed as such)
	p01 = EARTH_RADIUS * cos_phi * cos_theta;
	p02 = EARTH_RADIUS * sin_phi * cos_theta;
	p03 = EARTH_RADIUS *           sin_theta;

	// e1 = site unit vector (East)
	e11 = -sin_phi;
	e12 =  cos_phi;
	e13 =        0;

	// e2 = site unit vector (North)
	e21 = -cos_phi * sin_theta;
	e22 = -sin_phi * sin_theta;
	e23 =            cos_theta;

	//    bin_position(0.0,0.0);

}

void Coordinates::setOriginDeg(const double &lat,const double &lon){
	setOrigin(lat/180.0*M_PI , lon/180.0*M_PI);
}


/*! Explanation?
 * Sets surface coordinates (phiBin, thetaBin) according to polar coordinates.
 */
void Coordinates::setBinPosition(const double &azimuth,const float &range){
	//		    double &lat,double &lon){
	static double x1, x2, x3;
	//    x1 = sin(range/EARTH_RADIUS_43); EROR
	x1 = cos( range / (EARTH_RADIUS * 2.0) );
	x2 = range * sin(azimuth);
	x3 = range * cos(azimuth);

	//
	p1 = (p01*x1 + e11*x2 + e21*x3);
	p2 = (p02*x1 + e12*x2 + e22*x3);
	p3 = (p03*x1 + e13*x2 + e23*x3);

	///  lon = atan2((e11*x1 + e21*x2 + p01*x3),
	///	(e12*x1 + e22*x2 + p02*x3));

	//  lon = asin( (e12*x1 + e22*x2 + p02*x3) );
	// longitude
	// TODO if (mode == IDEAL_GEOID)
	/*
	phiBin   = atan2( p2,p1 );
	thetaBin = asin( p3 / EARTH_RADIUS );
	*/

	phiBin   = atan2( p2,p1 );
	thetaBin = asin( p3 / EARTH_RADIUS );
	proj.projectFwd(phiBin,thetaBin);
	//phiBin   = phiBin;
	//thetaBin = thetaBin;
}

/*!
 * Sets surface coordinates (phiBin, thetaBin) according to polar coordinates.
 */
/*
  void Coordinates::getBinPosition(double &phi,float &theta,
		  const double &azimuth,const float &range){
  }
 */

/*!
 * Polar coordinates.
 */
/*
    void Coordinator::setBinPosition(double azimuth,float range){
  		setBinPosition((const double &)azimuth,(const double &)range);
    };
 */

/**
 *
 */
void  Coordinates::getBoundingBox(float range, double &latMin,double &lonMin, double &latMax,double &lonMax)
{

	double azm, lat, lon;
	setBinPosition(0,0);
	//if (drain::Debug > 1)
	//	std::cout <<  " getBoundingBox, site=" << binLatitudeDeg() << ',' << binLongitudeDeg() << std::endl;

	// TODO: this will not work over the day zone...
	latMin = +360;
	lonMin = +360;
	latMax = -360;
	lonMax = -360;

	for (int i=0; i<360; i+=20){
		azm = static_cast<float>(i)*M_PI/180.0;
		setBinPosition(azm,range);
		lat = binLatitudeDeg();
		lon = binLongitudeDeg();

		//std::cout <<  " sub= [" << azm << "°," << maxRange << "] => " << lat << ',' << lon << std::endl;
		// if (drain::Debug > 2)
			//std::cout <<  " sub= [" << i << "°," << range << "] => " << lat << ',' << lon << std::endl;
		latMin = std::min(lat,latMin);
		latMax = std::max(lat,latMax);
		lonMin = std::min(lon,lonMin);
		lonMax = std::max(lon,lonMax);
	}
}



void  Coordinates::info(std::ostream &ostr){
	//ostr << "site = [ " << p01 << ' ' << p02 << ' ' << p03 << " ]\n";
	ostr << "p0 = [ " << p01 << ' ' << p02 << ' ' << p03 << " ]\n";
	ostr << "e1 = [ " << e11 << ' ' << e12 << ' ' << e13 << " ]\n";
	ostr << "e2 = [ " << e21 << ' ' << e22 << ' ' << e23 << " ]\n";
	ostr << "p  = [ " << p1  << ' ' << p2  << ' ' << p3  << " ]\n";
	ostr << "phi,theta = [ " << phiBin << ',' << thetaBin << " ]\n";
	ostr << "lat,lon   = [ " << binLatitudeDeg() << ',' << binLongitudeDeg() << " ]\n";
	ostr << EARTH_RADIUS << "\n";
}

} // ::rack

