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

#ifndef RACK_SUN_H
#define RACK_SUN_H


#include <string>

#include <drain/util/Geo.h>


/*
 * 2001 (C) Harri.Hohti@fmi.fi
 * Original formulae from: Jean Meeus: Astronomical Formulae for Calculators (1980)
 */

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

// Rack
