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


#ifndef RACK_GRAPHICS_RADAR
#define RACK_GRAPHICS_RADAR


//#include "resources-image.h"
//#include "resources.h"

#include <drain/image/GeoFrame.h>
#include <drain/image/TreeElemUtilsSVG.h>

#include "radar/RadarProj.h"


namespace rack {

/// Vector graphics for both composites and single radar data (polar coordinates).
/**
 *   This class utilizes SVG elements supporting free draw, especially POLYGON and PATH.
 *
 *   As geographic projections distort lines, arcs and angles, Bezier curves (PATH) are used.
 *
 *
 */
class RadarSVG {
public:

	RadarProj radarProj;

	drain::image::GeoFrame geoFrame;


	/// Number of "sectors" in a sphere.
	inline
	void setRadialResolution(int n){
		getCubicBezierConf(conf, n);
		radialResolution = n;
	}

	/**
	 *  \param n - spherical resolution (number of sectors of sphere, arcs of which are spanned by cubic control vectors).
	 */
	/*
	void getCubicCoeff(int n, double & radialCoeff, double & angularOffset) const {
		double theta = 2.0*M_PI / static_cast<double>(n);
		double k = 4.0/3.0 * ::tan(theta/4.0);
		radialCoeff = sqrt(1.0 + k*k);
		angularOffset = ::atan(k);
	}
	*/

	struct CubicBezierConf {
		// int sectorCount;
		/// Sector angle
		double delta = 0.0;
		double radialCoeff = 1.0;
		double angularOffset = 0.0;
	};

	CubicBezierConf conf;

	/***
	 *   \param n -sectors
	 */
	inline
	void getCubicBezierConf(CubicBezierConf & conf, int n) const {
		getCubicBezierConf(conf, 0.0, 2.0*M_PI / static_cast<double>(n));
	}

	void getCubicBezierConf(CubicBezierConf & conf, double angleStartR, double angleEndR) const;

	// typedef DRAIN_SVG_ELEM_CLS(PATH) svgPath;
	inline
	void convert(double radius, double azimuth, drain::Point2D<int> & imgPoint) const {
		drain::Point2D<double> geoPoint;
		polarToMeters(radius, azimuth, geoPoint);
		// radarProj.projectFwd(radius*::sin(azimuth), radius*::cos(azimuth), geoPoint.x, geoPoint.y);
		geoFrame.m2pix(geoPoint, imgPoint);
	};

	inline
	void polarToMeters(double radius, double azimuth, drain::Point2D<double> & geoPoint) const {
		radarProj.projectFwd(radius*::sin(azimuth), radius*::cos(azimuth), geoPoint.x, geoPoint.y);
		//geoFrame.m2pix(geoPoint, imgPoint);
	};

	inline
	void radarGeoToCompositeImage(drain::Point2D<double> & radarPoint, drain::Point2D<int> & imagePoint) const {
		drain::Point2D<double> compositePoint;
		radarProj.projectFwd(radarPoint, compositePoint);
		geoFrame.m2pix(compositePoint, imagePoint);
	}


	/// Move to image point at (radius, azimuth)
	/**
	 *
	 */
	inline
	void moveTo(drain::svgPATH & elem, drain::Point2D<int> & imgPoint, double radiusM, double azimuthR) const {
		convert(radiusM, azimuthR, imgPoint);
		elem.absolute<drain::svgPATH::MOVE>(imgPoint.x, imgPoint.y);
	}

	// Simple version not sharing end point.
	/**
	 *
	 */
	inline
	void moveTo(drain::svgPATH & elem, double radius, double azimuth) const {
		drain::Point2D<int> imgPoint;
		moveTo(elem, imgPoint, radius, azimuth);
	}

	inline
	void lineTo(drain::svgPATH & elem, double radius, double azimuth) const {
		drain::Point2D<int> imgPoint;
		lineTo(elem, imgPoint, radius, azimuth);
	}

	inline
	void lineTo(drain::svgPATH & elem, drain::Point2D<int> & imgPoint, double radiusM, double azimuthR) const {
		convert(radiusM, azimuthR, imgPoint);
		elem.absolute<drain::svgPATH::LINE>(imgPoint.x, imgPoint.y);
	}


	/// Single command to draw arc
	/**
	 *  \param drain::Point2D<int> & imgPoint - starting point, should be "already" compatible with (radiusM, azimutthStartR)
	 */
	void cubicBezierTo(drain::svgPATH & elem, double radiusM, double azimuthStartR, double azimuthEndR) const ;

	/**
	 *  \param drain::Point2D<int> & imgPoint - starting point, should be "already" compatible with (radiusM, azimutthStartR)
	 */
	void cubicBezierTo(drain::svgPATH & elem, drain::Point2D<int> & imgPoint, double radiusM, double azimuthStartR, double azimuthEndR) const;

	inline
	void close(drain::svgPATH & elem){
		elem.absolute<drain::svgPATH::CLOSE>();
	}

protected:

	int radialResolution;

};


} // rack::


#endif
