/*

    Copyright 2001 - 2013  Harri Hohti, Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

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

   Created on: Mar 7, 2011
   Author: Harri Hohti (C++ wrapper by Markus Peura)

 */

#ifndef RACK_SUN_H
#define RACK_SUN_H


#include <string>

#include <drain/util/Geo.h>
// // using namespace std;

namespace rack {

struct Equatorial {
	double RA;
	double dec;
};


struct Geographical {
	double lat;
	double lon;
};

struct Horizontal {
	double azm;
	double elev;
};


//class Sun : public Geographical, public Equatorial, public Horizontal {
class Sun : public Horizontal {

public:

	Sun(const std::string & datestr = "197001010000", double lon=0.0, double lat=0.0){
		setStarTime(datestr);
		setLocation(lon, lat);
	};

	/// Returns the sun azimuth and elevation angle at a given time in a geographical position.
	/**
	 * 	\param datestr - date std::string in format "YYYYmmddHHMM"
	 *  \param lonDeg - longitude in radians
	 *  \param latDeg - latitude in radians
	 *  \param azmRad  - retrieved azimuth angle in radians
	 *  \param elevRad - retrieved elevation angle in radians
	 */
	static
	void getSunPos(const std::string & datestr, double lonDeg, double latDeg, double & azmRad, double & elevRad);



	inline
	void setStarTime(const std::string & datestr){

		double jDate = JDate(datestr.c_str());

		/* Tähtiaika radiaaneina auringon tuntikulman laskentaa varten */
		starTime = Sidereal_GW(jDate) * 15.0 * drain::DEG2RAD;
		Solar_coords(jDate, equ);

	}


	/* Lasketaan auringon horisontaalikoordinaatit paikan ja ajan funktiona */
	inline
	void setLocation(double lon, double lat){

		/* Lasketaan auringon horisontaalikoordinaatit paikan ja ajan funktiona */
		geo.lat = lat;
		geo.lon = lon;
		Solar_pos(starTime, geo, equ, *this);
	}

	Geographical geo;


private:

	/* Tähtiaika radiaaneina auringon tuntikulman laskentaa varten */
	double starTime;

	Equatorial equ;

	//static double DEG_TO_RAD,RAD_TO_DEG;

	/*
	typedef struct { double RA;
	double Dec;
	} Equatorial;

	/// Radians (was degrees)
	typedef struct {
		double A;
		double h;
	} Horizontal;

	/// Radians?
	typedef struct {
		double lat;
		double lon;
	} Geographical;
	*/

	// Equatorial equ;

	// Geographical geo;

	/* Julian date */
	static double JDate(const char *datestr);

	/* 0-meridian astronomic time */
	static double Sidereal_GW(double JD);

	/* equatoriali coords */
	//static Equatorial Solar_coords(double JD);
	static
	void Solar_coords(double JD, Equatorial & equ);

	/* Horizontal coords without refraction */
	/* Auringon horisontaalikoordinaatit (ei refraktiota) */
	static
	//Horizontal Solar_pos(double Sd0,
	void Solar_pos(double Sd0, const Geographical & geo, const Equatorial & equ, Horizontal & hor);


};

} // rack::

#endif /* RACK_H_ */
