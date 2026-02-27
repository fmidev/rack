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
#ifndef DRAIN_PROJ6_H_
#define DRAIN_PROJ6_H_

#include <cstddef>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

#include <drain/Log.h>
#include "Point.h"
#include "Projector.h"


namespace drain
{

/*! A wrapper for the famous Proj6 class.
 *  
 * proj_context_errno(PJ_CONTEXT *ctx) -> const char *proj_errno_string(int err)
 *
 *
 * IMPORTANT:
 *
 * https://proj.org/development/reference/functions.html#c.proj_create
 * If a proj-string contains a +type=crs option, then it is interpreted as a CRS definition.
 * In particular geographic CRS are assumed to have axis in the longitude, latitude order and with degree angular unit.
 * The use of proj-string to describe a CRS is discouraged. It is a legacy means of conveying CRS descriptions:
 * use of object codes (EPSG:XXXX typically) or WKT description is recommended for better expressivity.
 *
 *
 */ 
class Proj6
{
public:

	Proj6();

	Proj6(const Proj6 &p);

	virtual ~Proj6();


	static inline
	const std::string & getProjVersion(){
		return Projector::proj4version;
	}

	Projector src;
	Projector dst;


	/// Sets source projection.
	/**
	 *
	 */
	/*
	inline
	void setProjectionSrc(const std::string & projDef, Projector::CRS_mode crs=Projector::FORCE_CRS){
		//src.clear();
		src.setProjection(projDef, crs);
		setMapping(true);
	}
	*/

	template <typename ...T>
	inline
	void setProjectionSrc(const T& ...args){
		//src.clear();
		src.setProjection(args...);
		setDirectMapping(true);
	}

	/// Sets source projection, primarily using EPSG code.
	/**
	 *
	 */
	inline
	void setProjectionSrc(const Projector & projDef){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.unimplemented<LOG_ERR>("Not setting ", projDef.getProjDef());
		//src.setProjection(projDef, crs);
		// setMapping(true);
	}

    /// Sets destination projection.
	/*
	inline
	void setProjectionDst(const std::string & projDef, Projector::CRS_mode crs=Projector::FORCE_CRS){
		dst.setProjection(projDef, crs);
		setDirectMapping(true);
	}
	*/

	template <typename ...T>
	inline
	void setProjectionDst(const T & ...args){
		dst.setProjection(args...);
		setDirectMapping(true);
	}


    /// Sets destination projection, primarily using EPSG code.
	inline
	void setProjectionDst(const Projector & projDef){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.unimplemented<LOG_ERR>("Not setting ", projDef.getProjDef());
		// dst.setProjection(projDef, crs);
		// setMapping(true);
	}

    /// Sets source and destination projection. TOOD: EPSG code handling
	inline
	void setProjections(const std::string & projDefSrc, const std::string & projDefDst){
		src.setProjection(projDefSrc, Projector::FORCE_CRS);
		dst.setProjection(projDefDst, Projector::FORCE_CRS);
		setDirectMapping(false);
	}


	//void setMapping(const std::string & proj1, const std::string & proj2);


   	/// Returns the projection std::string applied by the last setProjection call.
   	/**!
   	 *   Get a PROJ string representation of an object.
   	 *
   	 *   Does not reconstruct it from the allocated proj structure.
   	 */
    inline
    const std::string & getProjStrSrc() const {
    	return src.getProjDef();
    	//return getProjection(projSrc, projStrSrc);
    };

    inline
    const std::string & getProjStrDst() const {
    	return dst.getProjDef();
    	//return getProjection(projDst, projStrDst);
    };

    inline
    const Projector & getSrc() const {
    	return src;
    };

    inline
    const Projector & getDst() const {
    	return dst;
    };



	/// Forward projection (in-place)
	inline
	void projectFwd(double & x, double & y) const {
		project<PJ_FWD>(x,y);
	}

	/// Forward projection
	inline
	void projectFwd(double x, double y, double & x2, double & y2) const {
		x2 = x;
		y2 = y;
		project<PJ_FWD>(x2,y2);
	}

	/// Forward projection. Example implementation of project<>() .
	inline
	void projectFwd(drain::Point2D<double> & point) const {
		project<PJ_FWD>(point);
	}

	/// Forward projection. Example implementation of project<>() .
	inline
	void projectFwd(const drain::Point2D<double> & point, drain::Point2D<double> & point2) const {
		project<PJ_FWD>(point2 = point);
	}


	// Inverse projection (in-place)
	inline
	void projectInv(double & x, double & y) const {
		project<PJ_INV>(x,y);
	}

	// Inverse projection
	inline
	void projectInv(double x, double y, double & x2, double & y2) const {
		x2 = x;
		y2 = y;
		project<PJ_INV>(x2,y2);
	}

	// Inverse projection. Example implementation of project<>() .
	inline
	void projectInv(drain::Point2D<double> & point) const {
		project<PJ_INV>(point);
	}

	/// Forward projection. Example implementation of project<>() .
	inline
	void projectInv(const drain::Point2D<double> & point, drain::Point2D<double> & point2) const {
		project<PJ_INV>(point2 = point);
	}



    inline
    void debug(std::ostream & ostr = std::cout, int wkt = -1){

    	ostr << "SRC:\n";
    	src.info(ostr, wkt);
    	ostr << std::endl;

    	ostr << "DST:\n";
    	dst.info(ostr, wkt);
    	ostr << std::endl;
    }



    /// Check if destination projection is longitude-latitude degrees
    inline
	bool isLongLat() const {
		return dst.isLongLat();
	}

    inline
    bool isSet() const {
    	return (src.isSet() && dst.isSet());
    }

    inline
    std::string getErrorString() const {
    	int err = proj_context_errno(pjContext);
    	return proj_errno_string(err);
    	//return "Not Impl."; //std::string(pj_strerrno(*pj_get_errno_ref()));
    };



    /// Detect EPSG code from "+init=epsg:EPSG" argument.
    /**
     *  \param projDef – PROJ.4 projection definition string
     *  \return – EPSG code, if found, else 0.
     *
     *  \see getEpsgDict()
    static inline
	short int pickEpsgCodeOLD(const std::string & projDef){
    	std::list<std::string> projArgs;
    	return pickEpsgCodeOLD(projDef, projArgs);
    }
 */

   // static short int pickEpsgCodeOLD(const std::string & projDef, std::list<std::string> & projArgs);

protected:

	PJ_CONTEXT *pjContext = nullptr; // = proj_context_create();
	PJ *proj = nullptr;            // two-way

	/// Set crs_to_crs projection, if both src and dst projections are set.
	void setDirectMapping(bool lenient);


	/** \tparam POINT_XY – anything with members double x and double y
	 *  \tparam D - PJ_DIRECTION enum value (libproj)
	 */
	template
	<PJ_DIRECTION D,class POINT_XY>
	inline
	void project(POINT_XY & point) const {
		// void project(drain::Point2D<double> & point) const {
		// Note: sizeof not needed, future option for arrays.
		proj_trans_generic(proj, D, &point.x, sizeof(POINT_XY), 1, &point.y, sizeof(POINT_XY), 1, 0, 0, 0, 0, 0, 0);
	}

	template
	<PJ_DIRECTION D>
	inline
	void project(double & x, double & y) const {
		// Note: sizeof not needed, future option for arrays.
		proj_trans_generic(proj, D, &x, sizeof(double), 1, &y, sizeof(double), 1, 0, 0, 0, 0, 0, 0);
	}



	
};

DRAIN_TYPENAME(Proj6);

std::ostream & operator<<(std::ostream & ostr, const Proj6 &p);

} // drain


#endif /*PROJ4_H_*/

