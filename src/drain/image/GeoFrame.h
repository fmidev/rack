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
#ifndef GEOFRAME_H_
#define GEOFRAME_H_

#include "Geometry.h"
#include "drain/util/BoundingBox.h"
#include "drain/util/Geo.h"
#include "drain/util/Proj6.h"  // for geographical projection of radar data bins


namespace drain
{



namespace image
{

class GeoInfo {

public:
	// TODO: CartesianODIM compatibility
};

/// Array with georeferencing support.
/*!

 */
class GeoFrame  { //  See also

public:

	//typedef unsigned int icoord_t;

	/// Default constructor.
	GeoFrame(unsigned int width = 0, unsigned int height = 0);

	virtual inline
	~GeoFrame(){
	};

	inline
	void reset(){
		setGeometry(0, 0);
		setBoundingBoxD(0,0,0,0);
		//bBoxD.set(+180.0, +90.0, -180.0, -90.0);
	}

	/// Returns true, if the geographical extent has been set.
	/**
	 *
	 */
	inline
	bool projectionIsSet() const {
		return projGeo2Native.isSet();
	};

	/// Return true, if array area is greater than zero.
	inline
	bool geometryIsSet() const {
		return ((frameWidth > 0) && (frameHeight > 0));
	};

	/// Returns true, if the bounding box (geographical extent) has been set.
	/**
	 *  Problem: sometimes bbox will be initisalized to negative "direction" to make it adapt to
	 *  a (set of) new bounding box(es).
	 */
	inline
	bool bboxIsSet() const {
		return (bBoxD.getArea() > 0.0);
	};

	/// Returns true, if the projection, array area and geographical extent bounding box has been set.
	/**
	 * 	This function is used in creating single-radar products where the bounding box will be
	 *  matched to that of the radar scope, by default.
	 */
	inline
	bool isDefined() const {
		return geometryIsSet() && projectionIsSet() && bboxIsSet();
	};


	// Notice that someday this does NOT allocate memory. @see allocate();
	virtual
	void setGeometry(unsigned int width, unsigned int height);

	/// Return the nominal width, not the width of the memory array which does not have to be allocated.
	inline
	int getFrameWidth() const {return frameWidth; };

	/// Return the nominal height, not the height of the memory array which does not have to be allocated.
	inline
	int getFrameHeight() const {return  frameHeight; };


	/// Sets bounding box in degrees OR in metres in the target coordinate system.
	/**
	 *   Checks if a coordinate looks like degrees, that is, longitude is within [-90,+90] and latitude within [-180,+180].
	 *   Assumes that small absolute values suggest degrees, larger are metres.
	 *   Projection has to be set prior to setting metric bbox.
	 *
	 */
	void setBoundingBox(double lonLL, double latLL, double lonUR, double latUR);

	/// Sets bounding box in degrees in the target coordinate system.
	inline
	void setBoundingBox(const drain::Rectangle<double> & bbox){
		setBoundingBox(bbox.lowerLeft.x, bbox.lowerLeft.y, bbox.upperRight.x, bbox.upperRight.y);
	}


	/// Sets bounding box in degrees in the target coordinate system.
	//inline
	void setBoundingBoxD(double lonLL, double latLL, double lonUR, double latUR);
	// { setBoundingBoxR(DEG2RAD*lonLL, DEG2RAD*latLL, DEG2RAD*lonUR, DEG2RAD*latUR);}

	/// Sets bounding box in degrees in the target coordinate system.
	inline
	void setBoundingBoxD(const drain::Rectangle<double> & bboxD){
		setBoundingBoxD(bboxD.lowerLeft.x, bboxD.lowerLeft.y, bboxD.upperRight.x, bboxD.upperRight.y);
	}


	/// Sets bounding box in radians in the target coordinate system.
	inline
	void setBoundingBoxR(double lonLL, double latLL, double lonUR, double latUR){
		setBoundingBoxD(RAD2DEG*lonLL, RAD2DEG*latLL, RAD2DEG*lonUR, RAD2DEG*latUR);
	}

	/// Sets bounding box in radians in the target coordinate system.
	inline
	void setBoundingBoxR(const drain::Rectangle<double> & bboxR){
		setBoundingBoxR(bboxR.lowerLeft.x, bboxR.lowerLeft.y, bboxR.upperRight.x, bboxR.upperRight.y);
	}


	/// Sets bounding box in meters in the target coordinate system.
	void setBoundingBoxM(double xLL, double yLL, double xUR, double yUR);

	/// Sets bounding box in meters in the target coordinate system.
	inline
	/*
	void setBoundingBoxM(const drain::Rectangle<double> & bboxM) {
		setBoundingBoxM(bboxM.lowerLeft.x, bboxM.lowerLeft.y, bboxM.upperRight.x, bboxM.upperRight.y);
	}*/
	void setBoundingBoxM(const drain::UniTuple<double,4> & bboxM) {
		setBoundingBoxM(bboxM[0], bboxM[1], bboxM[2], bboxM[3]);
	}

protected:

	/// Proj 6 NEW: Given BBox in geo coords [deg], adjust geo coords [rad]
	void updateBoundingBoxR(); //double lonLL, double latLL, double lonUR, double latUR);

	/// Given BBox in geo coords [rad], adjust geo coords [deg]
	void updateBoundingBoxD(); //double lonLL, double latLL, double lonUR, double latUR);

	/// Given BBox in geo coords [rad], adjust metric bounding box. Do not update xScale or yScale.
	void updateBoundingBoxM(); // double lonLL, double latLL, double lonUR, double latUR);


	/// Geometric scaling.
	void updateScaling();

public:



	/// Returns the geographical scope in Degrees.
	inline
	const drain::Rectangle<double> & getBoundingBoxDeg() const { return bBoxD; };

	/// Returns the geographical scope in Meters.
	inline
	const drain::Rectangle<double> & getBoundingBoxNat() const { return bBoxNative; };

	/// Returns the geographical scope in Radians.
	inline
	const drain::Rectangle<double> & getBoundingBoxRad() const { return bBoxR; };

	void getCenterPixel(drain::Rectangle<double> & pixelD) const;

	/// Return horizontal resolution of a pixel in meters (if metric) or degrees (if unprojected, "latlon").
	inline
	double getXScale() const {
		return xScale;
	}

	/// Return vertical resolution of a pixel in meters (if metric) or degrees (if unprojected, "latlon").
	inline
	double getYScale() const {
		return yScale;
	}



	///  Crops the initial bounding box with a given bounding box.
	/*
	 *  \par bboxM - bounding box (xLowerLeft, yLowerLeft, xUpperRight, yUpperRight).
	 */
	inline
	void cropWithM(drain::Rectangle<double> & bboxM) {
		cropWithM(bboxM.lowerLeft.x, bboxM.lowerLeft.y, bboxM.upperRight.x, bboxM.upperRight.y);
	}

	/// Crops the initial bounding box with a given bounding box.
	void cropWithM(double xLL, double yLL, double xUR, double yUR);


	/// Projects geographic coordinates to image coordinates.
	virtual inline
	void deg2pix(double lon, double lat, int & i, int & j) const {
		projGeo2Native.projectFwd(lon, lat, lon, lat); // PROJ6-CRS uses degrees
		// ...Now (lon,lat) are metric
		// projGeo2Native.projectFwd(lon*DEG2RAD, lat*DEG2RAD, lon, lat); // PROJ6-CRS uses degrees
		m2pix(lon, lat, i,j);
	}

	/// Project geographic coordinates (degrees) to image coordinates (pixels).
	inline
	void deg2pix(const drain::Point2D<double> & loc, drain::Point2D<int> & pix) const {
		deg2pix(loc.x, loc.y, pix.x, pix.y);
	}

	/// Convert degrees to native (often metric) coordinates
	/**
	 */
	virtual inline
	void deg2m(double lon, double lat, double &x, double &y) const {
		projGeo2Native.projectFwd(lon, lat, x, y); // PROJ6-CRS uses degrees
	}

	/// Convert degrees to native (often metric) coordinates
	/**
	 */
	inline
	void deg2m(const drain::Point2D<double> & pDeg, drain::Point2D<double> & pMet) const {
		deg2m(pDeg.x, pDeg.y, pMet.x, pMet.y);
	}

	/// Calculates the geographic coordinates of the center of a pixel at (i,j).
	virtual inline
	void pix2deg(int i, int j, double & lon, double & lat) const {
		pix2m(i,j, lon,lat); //pix2m(i,j, x,y);
		projGeo2Native.projectInv(lon,lat, lon,lat);
		// lon *= RAD2DEG; // PROJ6-CRS uses degrees
		// lat *= RAD2DEG; // PROJ6-CRS uses degrees
	}

	/// Calculates the geographic coordinates (lon,lat) [rad] of the center of a pixel at (i,j).
	virtual inline
	void pix2rad(int i, int j, double & lon, double & lat) const {
		pix2m(i,j, lon,lat);
		projGeo2Native.projectInv(lon,lat, lon,lat);
		// PROJ6-CRS uses degrees:
		lon *= DEG2RAD;
		lat *= DEG2RAD;
	}


	/// Calculates the geographic coordinates of the center of a pixel at (i,j).
	inline
	void pix2deg(const drain::Point2D<int> & pix, drain::Point2D<double> & loc) const {
		pix2deg(pix.x,pix.y, loc.x,loc.y);
	}

	/// Calculates the geographic coordinates of the lower left corner of a pixel at (i,j).
	virtual inline
	void pix2LLdeg(int i,int j, double & lon, double & lat) const {
		double x, y; // metric
		pix2LLm(i,j, x,y);
		projGeo2Native.projectInv(x,y, lon,lat);
		// lon *= RAD2DEG; // PROJ6-CRS uses degrees
		// lat *= RAD2DEG; // PROJ6-CRS uses degrees
	}

	/// Convert metric coordinates to degrees
	/**
	 */
	virtual inline
	void m2deg(double x, double y, double & lon, double & lat) const {
		projGeo2Native.projectInv(x,y, lon,lat);
		// lon *= RAD2DEG; // PROJ6-CRS uses degrees
		// lat *= RAD2DEG; // PROJ6-CRS uses degrees
	}

	/// Convert metric coordinates to degrees
	/**
	 */
	virtual inline
	void m2deg(const drain::Point2D<double> & pMetric, drain::Point2D<double> & pDeg) const {
		projGeo2Native.projectInv(pMetric.x, pMetric.y, pDeg.x, pDeg.y); // lon, lat);
		//pDeg.x *= RAD2DEG; // PROJ6-CRS uses degrees
		//pDeg.y *= RAD2DEG; // PROJ6-CRS uses degrees
	}

	/* UNTESTED
	inline virtual
	void LLdeg2m(const double & lon, const double & lat, double & x, double & y) const {
			projGeo2Native.projectFwd(lon,lat, x,y);
	}
	*/


	/// Scales geographic map coordinates to image coordinates.
	/**
	 *  \par x - horizontal map coordinate in meters
	 *  \par y - vertical map coordinate in meters
	 *  \par i - horizontal image coordinate
	 *  \par j - vertical image coordinate
	 * Note that i increases to right, j downwards.
	 */
	inline virtual
	void m2pix(double x, double y, int & i, int & j) const {
		// i = static_cast<int>(0.5+ (x - bBoxNative.lowerLeft.x) / xScale); //  xOffset
		// j = frameHeight-1 - static_cast<int>(0.5+ (y - bBoxNative.lowerLeft.y) / yScale);
		// j = frameHeight-1 - static_cast<int>(0.5+ (y - bBoxNative.lowerLeft.y) / yScale);
		i = ::lround((x - bBoxNative.lowerLeft.x) / xScale); //  xOffset
		j = frameHeight-1 - ::lround((y - bBoxNative.lowerLeft.y) / yScale);
	}

	inline virtual
	void m2pix(const drain::Point2D<double> & pMetric, drain::Point2D<int> & pImage) const {
		// pImage.x = static_cast<int>(0.5+ (pMetric.x - bBoxNative.lowerLeft.x) / xScale); //  xOffset
		// pImage.y = frameHeight-1 - static_cast<int>(0.5+ (pMetric.y - bBoxNative.lowerLeft.y) / yScale);
		// pImage.y = frameHeight-1 - static_cast<int>(0.5+ (pMetric.y - bBoxNative.lowerLeft.y) / yScale);
		pImage.x = ::lround( (pMetric.x - bBoxNative.lowerLeft.x) / xScale); //  xOffset
		pImage.y = frameHeight-1 - ::lround((pMetric.y - bBoxNative.lowerLeft.y) / yScale);
	}


	/// Scales image coordinates (i,j) to map coordinates (x,y) in the pixel \b centres.
	/**
	 *  \par i - horizontal image coordinate
	 *  \par j - vertical image coordinate
	 *  \par x - horizontal map coordinate in meters
	 *  \par y - vertical map coordinate in meters
	 * Note that i increases to right, j downwards.
	 */
	inline
	virtual
	void pix2m(int i, int j, double & x, double & y) const {
		x = (static_cast<double>(i)+0.5)*xScale + bBoxNative.lowerLeft.x;
		y = (static_cast<double>(frameHeight-1 - j)+0.5)*yScale + bBoxNative.lowerLeft.y;
	}

	/// Scales image coordinates (i,j) to map coordinates (x,y) in the pixel \b centres.
	inline
	virtual
	void pix2m(const drain::Point2D<int> & pImage, drain::Point2D<double> & pMetric) const {
		pMetric.x = (static_cast<double>(pImage.x)+0.5)*xScale + bBoxNative.lowerLeft.x;
		pMetric.y = (static_cast<double>(frameHeight-1 - pImage.y)+0.5)*yScale + bBoxNative.lowerLeft.y;
	}

	/// Scales image coordinates (i,j) to map coordinates (x,y) of the lower left corner pixel.
	/**
	 *  \par i - horizontal image coordinate
	 *  \par j - vertical image coordinate
	 *  \par x - horizontal map coordinate (often metric)
	 *  \par y - vertical map coordinate (often metric)
	 *
	 *  Note that i increases to right, j downwards.
	 */
	inline
	virtual
	void pix2LLm(int i, int j, double & x, double & y) const {
		x = (static_cast<double>(i))*xScale + bBoxNative.lowerLeft.x;
		y = (static_cast<double>(frameHeight-1 - j))*yScale + bBoxNative.lowerLeft.y;
		// y = (static_cast<double>(frameHeight-1 - j))*yScale + bBoxNative.lowerLeft.y;
	}





	/// Sets the projection of the image as a Proj string.
	inline
	void setProjection(const std::string & projDef){
		projGeo2Native.setProjectionDst(projDef);
		updateProjection();
	}

	/// Sets the projection of the image as a EPSG code.
	inline
	void setProjectionEPSG(int epsg){
		projGeo2Native.dst.setProjection(epsg);
		updateProjection();
	}

	/// Updates bboxes, if needed.
	void updateProjection();

	/// Returns the projection of the composite image as a proj4 std::string.
	inline
	const std::string & getProjection() const { return projGeo2Native.getProjectionDst();};

	/// Returns the source projection (should be radian array, lon & lat).
	//  Consider: rename src/input projection coord system
	inline
	const std::string & getCoordinateSystem() const { return projGeo2Native.getProjectionSrc();};


	/// Return the actual geographical boundingBox suggested by implied by input data.
	inline
	//const drain::Rectangle<double> & getDataBBoxD() const { return dataBBoxD; };
	const drain::Rectangle<double> & getDataBBoxNat() const { return dataBBoxNat; };

	/// Return the common overlapping geographical area (intersection) implied by input data.
	inline
	//const drain::Rectangle<double> & getDataOverlapD() const { return dataOverlapD; };
	const drain::Rectangle<double> & getDataOverlapBBoxNat() const { return dataOverlapBBoxNat; };

	/// Extend to include this input
	/**
	 *  Book keeping of input bbox.
	 */
	void updateDataExtentDeg(const drain::Rectangle<double> &inputBBoxDeg);

	/**
	 *  Native means that rectangular shapes will not be distorted.
	 */
	void updateDataExtentNat(const drain::Rectangle<double> &inputBBoxNat);


	inline
	bool isLongLat() const {
		return projGeo2Native.isLongLat();
	}

	/// Radial to metric
	drain::Proj6 projGeo2Native;

	//std::ostream & toOStr(std::ostream & ostr) const ;
	virtual
	std::ostream & toStream(std::ostream & ostr) const;



protected:

	// drain::image::AreaGeometry frameGeometry; would be fine, but unsigned caused conversion/cast underflows
	int frameWidth;
	int frameHeight;

	/// Geographical scope in Radians.
	drain::Rectangle<double> bBoxR;

	/// Geographical scope in Degrees.
	drain::Rectangle<double> bBoxD;

	/// Geographical scope in meters, or degrees in case of long/lat
	drain::Rectangle<double> bBoxNative;


	// experimental
	/// For deriving extent (~union) of input data
	drain::Rectangle<double> dataBBoxNat;

	// experimental
	/// For deriving common extent (~intersection) of input data
	drain::Rectangle<double> dataOverlapBBoxNat;

	/// Utility for deriving extent (degrees) required by input data
	// drain::Rectangle<double> dataBBoxD;

	/// Utility for deriving extent (degrees) required by input data
	//drain::Rectangle<double> dataOverlapD;

	// ... needed in mapping...
	double xScale = 0.0;
	double yScale = 0.0;

};

inline
std::ostream & operator<<(std::ostream & ostr, const GeoFrame & frame){
	frame.GeoFrame::toStream(ostr);
	ostr << '\n';
	return ostr;
}


} // ::image
} // ::drain

#endif /*COMPOSITOR_R_H_*/

// Drain
