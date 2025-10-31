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

#include "radar/Composite.h"
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


	inline
	RadarSVG(int radialBezierResolution = 8){
		setRadialResolution(radialBezierResolution);
	};

	inline
	RadarSVG(const RadarSVG & radarSvg){
		setRadialResolution(radialBezierResolution);
	};



	enum StyleClasses {
		VECTOR_OVERLAY, // this is more for group ID/name
		GRID,           // CSS
		HIGHLIGHT,      // CSS
	};

	/// Sets some CSS properties applicable in radar graphics.
	/**
	 *  Creates a style element only if i does not exist already.
	 *
	 */
	static
	drain::image::TreeSVG & getStyle(drain::image::TreeSVG & svgDoc);

	/// Get (create) group dedicated for layers drawn over radar data
	/**
	 *
	 */
	static
	drain::image::TreeSVG & getGeoGroup(drain::image::TreeSVG & svgDoc);


	// Projection of the latest radar input.
	RadarProj radarProj;

	// Maximum range of the latest radar input.
	int maxRange = 0; // metres

	/// Geographic extent and projection (Cartesian)
	drain::image::GeoFrame geoFrame;

	/// Read meta data related to polar coordinates, that is, geographic configuration of a single radar.
	/**
	 *  This command can be used together with configureCartesian().
	 */
	void updateRadarConf(const drain::VariableMap & where);

	/// Read meta data related to Cartesian data, that is, geographic configuration of a radar composite.
	/**
	 *
	 *  This command can be used together with configurePolar().
	 */
	// template <class T>
	// void updateCartesianConf(const drain::SmartMap<T> & where);
	void updateCartesianConf(const drain::VariableMap & where);

	void updateCartesianConf(const Composite & comp);

	/// Number of "sectors" in a sphere.
	inline
	void setRadialResolution(int n){
		getCubicBezierConf(conf, n);
		radialBezierResolution = n;
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

	int radialBezierResolution;

};

/*
template <class T>
void RadarSVG::updateCartesianConf(const drain::SmartMap<T> & where) {

	drain::Logger mout(__FILE__, __FUNCTION__);
	// Todo: also support fully cartesian input (without single-site metadata)
	// radarProj.setSiteLocationDeg(where["lon"], where["lat"]);

	const int epsg = where.get("epsg", 0); // non-standard
	if (epsg){
		mout.attention("EPSG found: ", epsg);
		geoFrame.setProjectionEPSG(epsg);
		// radarProj.setProjectionDst(epsg);
	}
	else {
		const std::string projdef = where.get("projdef", ""); // otherwise gets "null"
		geoFrame.setProjection(projdef);
		// radarProj.setProjectionDst(projdef);
	}
	geoFrame.setBoundingBoxD(where["LL_lon"], where["LL_lat"], where["UR_lon"], where["UR_lat"]);
	geoFrame.setGeometry(where["xsize"], where["ysize"]);

}
*/

} // rack::


DRAIN_ENUM_DICT(rack::RadarSVG::StyleClasses);

DRAIN_ENUM_OSTREAM(rack::RadarSVG::StyleClasses);

namespace drain {

template <> // for T (Tree class)
template <> // for K (path elem arg)
inline
const image::TreeSVG & image::TreeSVG::operator[](const rack::RadarSVG::StyleClasses & cls) const {
	return (*this)[EnumDict<rack::RadarSVG::StyleClasses>::dict.getKey(cls, false)];
}

template <> // for T (Tree class)
template <> // for K (path elem arg)
inline
image::TreeSVG & image::TreeSVG::operator[](const rack::RadarSVG::StyleClasses & cls) {
	return (*this)[EnumDict<rack::RadarSVG::StyleClasses>::dict.getKey(cls, false)];
}

template <> // for T (Tree class)
template <> // for K (path elem arg)
inline
bool image::TreeSVG::hasChild(const rack::RadarSVG::StyleClasses & cls) const {
        return hasChild(EnumDict<rack::RadarSVG::StyleClasses>::dict.getKey(cls, true)); // no error
}

}

#endif
