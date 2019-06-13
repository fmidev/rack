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

//#include "Geometry.h"

#include "util/Rectangle.h"

#include "util/Geo.h"

#include "util/Proj4.h"  // for geographical projection of radar data bins

//#include "image/Image.h"
//#include "Accumulator.h"



//#include "Coordinates.h" // for site coords and bin coords.

// using namespace std;

namespace drain
{

namespace image
{


/// Array with georeferencing support.
/*!

 */
class GeoFrame { //  See also

public:

	typedef unsigned int icoord_t;

	/// Default constructor. The channels are DATA, COUNT, WEIGHT, WEIGHT2
	GeoFrame(unsigned int width = 0, unsigned int height = 0);

	// GeoFrame(const GeoFrame & gf);

	//, unsigned int imageChannels=1,unsigned int alphaChannels=2);
	virtual inline
	~GeoFrame(){
	};

	// Notice that someday this does NOT allocate memory. @see allocate();
	virtual
	void setGeometry(unsigned int width, unsigned int height);

	/// Return the nominal width, not the width of the memory array which does not have to be allocated.
	inline
	int getFrameWidth() const {return frameWidth;};

	/// Return the nominal height, not the height of the memory array which does not have to be allocated.
	inline
	int getFrameHeight() const {return frameHeight;};


	/// Sets bounding box in degrees in the target coordinate system.
	inline
	void setBoundingBoxD(double lonLL, double latLL, double lonUR, double latUR){
		//static const double D2R = M_PI/180.0;
		setBoundingBoxR(DEG2RAD*lonLL, DEG2RAD*latLL, DEG2RAD*lonUR, DEG2RAD*latUR);
	}

	/// Sets bounding box in degrees in the target coordinate system.
	inline
	void setBoundingBoxD(const drain::Rectangle<double> & bboxD){
		setBoundingBoxD(bboxD.lowerLeft.x, bboxD.lowerLeft.y, bboxD.upperRight.x, bboxD.upperRight.y);
		//static const double D2R = M_PI/180.0;
		//setBoundingBoxR(DEG2RAD*bboxD.lowerLeft.x, DEG2RAD*bboxD.lowerLeft.y, D2R*bboxD.upperRight.x, D2R*bboxD.upperRight.y);
	}

	/// Sets bounding box in radians in the target coordinate system.
	inline
	void setBoundingBoxR(const drain::Rectangle<double> & bboxR){
		setBoundingBoxR(bboxR.lowerLeft.x, bboxR.lowerLeft.y, bboxR.upperRight.x, bboxR.upperRight.y);
	}

	/// Sets bounding box in radians in the target coordinate system.
	void setBoundingBoxR(double lonLL, double latLL, double lonUR, double latUR);


	/// Sets bounding box in meters in the target coordinate system.
	void setBoundingBoxM(double xLL, double yLL, double xUR, double yUR);

	/// Sets bounding box in meters in the target coordinate system.
	inline
	void setBoundingBoxM(const drain::Rectangle<double> & bboxM) {
		setBoundingBoxM(bboxM.lowerLeft.x, bboxM.lowerLeft.y, bboxM.upperRight.x, bboxM.upperRight.y);
	}




	/// Returns the geographical scope in Degrees.
	inline
	const drain::Rectangle<double> & getBoundingBoxD() const { return extentD; };

	/// Returns the geographical scope in Meters.
	inline
	const drain::Rectangle<double> & getBoundingBoxM() const { return extentM; };

	/// Returns the geographical scope in Radians.
	inline
	const drain::Rectangle<double> & getBoundingBoxR() const { return extentR; };

	void getCenterPixel(drain::Rectangle<double> & pixelD) const;

	/// Return vertical resolution of a pixel in meters (if metric) or degrees (if unprojected, "latlon").
	double getYScale() const;

	/// Return vertical resolution of a pixel in meters (if metric) or degrees (if unprojected, "latlon").
	double getXScale() const;


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
		double x,y; // metric
		projR2M.projectFwd(lon*DEG2RAD, lat*DEG2RAD, x, y);
		m2pix(x,y, i,j);
		//j = height-1-j;
	}

	inline
	void deg2pix(const drain::image::Point2D<double> & loc, drain::image::Point2D<int> & pix) const {
		deg2pix(loc.x, loc.y, pix.x, pix.y);
	}

	/// Calculates the geographic coordinates of the center of a pixel at (i,j).
	virtual inline
	void pix2deg(int i, int j, double & lon, double & lat) const {
		double x, y; // metric
		pix2m(i,j, x,y);
		projR2M.projectInv(x,y, lon,lat);
		lon *= RAD2DEG;
		lat *= RAD2DEG;
	}

	/// Calculates the geographic coordinates of the center of a pixel at (i,j).
	inline
	void pix2deg(const drain::image::Point2D<int> & pix, drain::image::Point2D<double> & loc) const {
		pix2deg(pix.x, pix.y, loc.x, loc.y);
	}

	/// Calculates the geographic coordinates of the lower left corner of a pixel at (i,j).
	virtual inline
	void pix2LLdeg(int i,int j, double & lon, double & lat) const {
		double x, y; // metric
		pix2LLm(i,j, x,y);
		//pix2m(i,height-1-j,x,y);
		projR2M.projectInv(x,y, lon,lat);
		lon *= RAD2DEG;
		lat *= RAD2DEG;
	}


	virtual inline
	void m2deg(const double & x, const double & y, double & lon, double & lat) const {
		projR2M.projectInv(x,y, lon,lat);
		lon *= RAD2DEG;
		lat *= RAD2DEG;
	}

	/* UNTESTED
	inline virtual
	void LLdeg2m(const double & lon, const double & lat, double & x, double & y) const {
			projR2M.projectFwd(lon,lat, x,y);
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
	inline
	virtual
	void m2pix(const double & x, const double & y, int & i, int & j) const {
		i = static_cast<int>(0.5+ (x - extentM.lowerLeft.x) / xScale); //  xOffset
		j = frameHeight-1 - static_cast<int>(0.5+ (y - extentM.lowerLeft.y) / yScale);
		//j = 1-1 + static_cast<int>((y - extentM.lowerLeft.y) / yScale);
	}


	/// Scales image coordinates (i,j) to geographic map coordinates (x,y) in the pixel \b centres.
	/**
	 *  \par i - horizontal image coordinate
	 *  \par j - vertical image coordinate
	 *  \par x - horizontal map coordinate in meters
	 *  \par y - vertical map coordinate in meters
	 * Note that i increases to right, j downwards.
	 *
	 *
	 */
	inline
	virtual
	void pix2m(const int & i, const int & j, double & x, double & y) const {
		x = (static_cast<double>(i)+0.5)*xScale + extentM.lowerLeft.x;
		y = (static_cast<double>(frameHeight-1 - j)+0.5)*yScale + extentM.lowerLeft.y;
		//y = static_cast<double>(1-1 + j)*yScale + extentM.lowerLeft.y;
	}

	/// Scales image coordinates (i,j) to geographic map coordinates (x,y) of the lower left corner pixel.
	/**
	 *  \par i - horizontal image coordinate
	 *  \par j - vertical image coordinate
	 *  \par x - horizontal map coordinate in meters
	 *  \par y - vertical map coordinate in meters
	 * Note that i increases to right, j downwards.
	 */
	inline
	virtual
	void pix2LLm(const int & i, const int & j, double & x, double & y) const {
		x = (static_cast<double>(i))*xScale + extentM.lowerLeft.x;
		y = (static_cast<double>(frameHeight-1 - j))*yScale + extentM.lowerLeft.y;
		//y = static_cast<double>(1-1 + j)*yScale + extentM.lowerLeft.y;
	}


	/// Geometric scaling.
	void updateScaling();


	/// Returns true, if the geographical extent has been set.
	/**
	 * 	This function is used in creating single-radar products where the bounding box will be
	 *  matched to that of the radar scope, by default.
	 */
	inline
	bool isDefined() const {
		return (extentD.getArea() > 0.0);
	};


	/// Sets the projection of the composite image as a proj4 std::string.
	void setProjection(const std::string & projDef);

	/// Returns the projection of the composite image as a proj4 std::string.
	inline
	const std::string & getProjection() const { return projR2M.getProjectionDst();};

	/// Returns the source projection (should be radian array, lon & lat).
	inline
	const std::string & getCoordinateSystem() const { return projR2M.getProjectionSrc();};


	/// Return the actual geographical extent suggested by implied by input data.
	inline
	const drain::Rectangle<double> & getDataExtentD() const { return dataExtentD; };

	/// Return the common overlapping geographical area (intersection) implied by input data.
	inline
	const drain::Rectangle<double> & getDataOverlapD() const { return dataOverlapD; };

	void updateDataExtent(const drain::Rectangle<double> &inputExtentD);

	inline
	bool isLongLat(){
		return projR2M.isLongLat();
	}

	/// Radial to metric
	drain::Proj4 projR2M;

	std::ostream & toOStr(std::ostream & ostr) const ;

protected:

	int frameWidth;
	int frameHeight;

	/// Geographical scope in Radians.
	drain::Rectangle<double> extentR;

	/// Geographical scope in Degrees.
	drain::Rectangle<double> extentD;

	/// Geographical scope in Meters.
	drain::Rectangle<double> extentM;


	// ... needed in mapping...
	double xScale;
	double yScale;


	/// Utility for deriving extent (degrees) required by input data
	drain::Rectangle<double> dataExtentD;

	/// Utility for deriving extent (degrees) required by input data
	drain::Rectangle<double> dataOverlapD;



};

inline
std::ostream & operator<<(std::ostream & ostr, const GeoFrame & frame){
	frame.toOStr(ostr);
	ostr << '\n';
	return ostr;
}


} // ::image
} // ::drain

#endif /*COMPOSITOR_R_H_*/

// Drain
