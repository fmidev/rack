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

class Graphic  {

public:

	enum GRAPHIC {
		VECTOR_OVERLAY, // this is more for group ID/name
		HIGHLIGHT,      // CSS: activated on tool tip
		GRID,           // overlapping with element class?
		DOT,
		LABEL, // External
		RAY,
		SECTOR,
		ANNULUS,
		CIRCLE,
	};

	const drain::ClassXML cls;


	inline
	Graphic(GRAPHIC g = RAY) : cls(drain::EnumDict<GRAPHIC>::dict.getKey(g)) {
	}

	inline
	Graphic(const Graphic & g) : cls(g.cls) {
	}

	inline
	Graphic(const drain::ClassXML & cls) : cls(cls) {
	}

	static
	drain::image::TreeSVG & getGraphicStyle(drain::image::TreeSVG & svgDoc);


};


}

DRAIN_ENUM_DICT(rack::Graphic::GRAPHIC);
DRAIN_ENUM_OSTREAM(rack::Graphic::GRAPHIC);

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
	RadarSVG(int radialBezierResolution = 0){
		if (radialBezierResolution > 0){
			setRadialResolution(radialBezierResolution);
		}
		else {
			setRadialResolution(drain::image::FileSVG::radialBezierResolution);
		}
	};

	inline
	RadarSVG(const RadarSVG & radarSvg){
		setRadialResolution(radarSvg.radialBezierResolution);
	};


	/*
	enum StyleClasses {
		VECTOR_OVERLAY, // this is more for group ID/name
		HIGHLIGHT,      // CSS: activated on tool tip
		// ^ rename VECTORS  VECTOR_GRAPHICS
		GRID,           // CSS
	};
	*/

	/// Sets some CSS properties applicable in radar graphics (grids, sectors).
	/**
	 *  Creates a style element only if it does not exist already.
	 *
	 *  More general style is obtained with RackSVG::getStyle().
	 */
	// static
	// drain::image::TreeSVG & getOverlayStyle(drain::image::TreeSVG & svgDoc);

	/// Get (create) group dedicated for layers drawn over radar data
	/**
	 *
	 */
	static
	drain::image::TreeSVG & getOverlayGroup(drain::image::TreeSVG & svgDoc);


	// Projection of the latest radar input.
	RadarProj radarProj;

	inline
	void deriveMaxRange(const Hi5Tree & srcPolar){
		maxRange = DataTools::getMaxRange(srcPolar);
	}

	/// If r is inside +/-100% = [-1.0,1.0], return that portion of maximum range, else the argument as such.
	inline
	double getRange(double r=1.0){
		if (r < -1.0){
			return r;
		}
		else if (r > +1.0){
			return r;
		}
		else {
			return r*maxRange;
		}
	};



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
		if (n>1){
			getCubicBezierConf(conf, n);
			radialBezierResolution = n;
		}
		else {
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.error("To small radialBezierResolution: ", n);
		}
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
	void close(drain::svgPATH & elem) const {
		// No difference for absolute/relative
		elem.absolute<drain::svgPATH::CLOSE>();
	}

	/// Convenience: draw sector, starting from radius.min, ending at radius.max, in azimuth range azimuthR.min ... azimuthR.max.
	/**
	 *
	 *
	 */
	void drawSector(drain::svgPATH & elem, const drain::Range<double> & radius, const drain::Range<double> & azimuthR = {0.0, 0.0}) const;

	/// Convenience: draw circle (disk or annulus)
	/**
	 *   If (radius.min == radius.max), dist
	 *
	 */
	inline
	void drawCircle(drain::svgPATH & elem, const drain::Range<double> & radius) const {
		drawSector(elem, radius, {0.0, 0.0});
	}

protected:

	int radialBezierResolution;

	// Maximum range of the latest radar input.
	double maxRange = 0.0; // metres

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

/*


DRAIN_ENUM_DICT(rack::RadarSVG::StyleClasses);

DRAIN_ENUM_OSTREAM(rack::RadarSVG::StyleClasses);

*/

namespace drain {


template <> // for T (Tree class)
template <> // for K (path elem arg)
inline
const image::TreeSVG & image::TreeSVG::operator[](const rack::Graphic::GRAPHIC & cls) const {
	return (*this)[EnumDict<rack::Graphic::GRAPHIC>::dict.getKey(cls, false)];
}

template <> // for T (Tree class)
template <> // for K (path elem arg)
inline
image::TreeSVG & image::TreeSVG::operator[](const rack::Graphic::GRAPHIC & cls) {
	return (*this)[EnumDict<rack::Graphic::GRAPHIC>::dict.getKey(cls, false)];
}

template <> // for T (Tree class)
template <> // for K (path elem arg)
inline
bool image::TreeSVG::hasChild(const rack::Graphic::GRAPHIC & cls) const {
        return hasChild(EnumDict<rack::Graphic::GRAPHIC>::dict.getKey(cls, true)); // no error
}

}

#endif
