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

#include <proj.h>

#include "Dictionary.h"
#include "Point.h"
// #include "TreeUnordered.h"


namespace drain
{

class Proj6;


// Helper class for containing a single (one-way) Proj object and its
class Projector {

	friend class Proj6;

	static const SprinterLayout projDefLayout; // space-separated, =, no hypens  (" ","","=", "","");

public:

	typedef drain::Dictionary<std::string,std::string> ProjDef;

	//typedef enum {PROJ4, PROJ6, CRS} version;
	typedef enum {
		ACCEPT_CRS, // Do not touch
		REMOVE_CRS, // Delete "+type=crs" from projDef, warn if EPSG:<code> syntax used
		FORCE_CRS   // Add "+type=crs"
	} CRS_mode;


	/// Each projector has three (3) versions of project definition
	typedef enum {
		ORIG,    // Supplied, provided by user
		MODIFIED, // EPSG-converted and filtered
		// FINAL,    //
		PROJ4, // Final, formulated by projlib
		PROJ5, // Final, formulated by projlib
		SIMPLE // Final, simplified for backward compatibility: "+type=crs" and "+init=epsg:..." pruned.
	} PROJDEF_variant;


	inline  // NEW 2024: proj_context_create()
	Projector(const std::string & projDef = "", CRS_mode crs=ACCEPT_CRS) : pjContext(proj_context_create()), pj(nullptr), epsg(0){
	//Projector(const std::string & projDef = "", CRS_mode crs=ACCEPT_CRS) : pjContext(nullptr), pj(nullptr), epsg(0){
		if (projDef.empty()){
			projDefs = {{ORIG,""}, {MODIFIED,""}, {PROJ4,""}, {PROJ5,""}, {SIMPLE,""}};
		}
		else {
			setProjection(projDef, crs);
		}
	}

	inline
	Projector(PJ_CONTEXT *pjContext, const std::string & projDef = "", CRS_mode crs=ACCEPT_CRS) : pjContext(pjContext), pj(nullptr), epsg(0){
		if (projDef.empty()){
			projDefs = {{ORIG,""}, {MODIFIED,""}, {PROJ4,""}, {PROJ5,""}, {SIMPLE,""}};
		}
		else {
			setProjection(projDef, crs);
		}
	}

	inline
	Projector(const Projector & pr) :
		pjContext(proj_context_clone(pr.pjContext)), // NEW 2024
		// pjContext(nullptr), // for safety
		// TODO: CLONE, in version 7.2.
		// pjContext(proj_context_clone(pr.pjContext)), // TODO: flag for own CTX => destroy at end
		pj(proj_clone(pjContext, pr.pj)),
		// projDefDict(pr.projDefDict),
		epsg(pr.epsg)
	{
		// TODO
		//projDefs = {{ORIG,"*"}, {CHECKED,"**"}, {FINAL,"***"}, {INTERNAL,"****"}};
		setProjection(pr.getProjDef(ORIG)); // imitate ?
	}

	virtual inline
	~Projector(){
		proj_destroy(pj);
		proj_context_destroy(pjContext); // NEW 2024
	}

    static
    void getProjDefDict(const std::string & str, ProjDef & dict);

	static
	//int extractEPSG(const std::string & projDef);
	int extractEPSG(const ProjDef & projDefDict);

    static
    void getProjDefStr(const ProjDef & dict, std::stringstream & sstr, const std::set<std::string> & excludeKeys = {"+type"});

	const std::string & getProjDef(PROJDEF_variant v = SIMPLE) const { // For external objects, excluding +init=epsg and +type=crs
		return projDefs[v];
	}



	/// Deletes projection object and clears all the metadata.
	void clear(){
		projDefs = {{ORIG,""}, {MODIFIED,""}, {PROJ4,""}, {PROJ5,""}, {SIMPLE,""}};
		pjContext = nullptr; // TODO: flag for own CTX => destroy at end
		proj_destroy(pj);
		pj = nullptr;
		projDefDict.clear();
		epsg = 0;
	}

	/// Sets projection defined as Proj string.
	/**
	 *
	 */
	void setProjection(const std::string &str, CRS_mode crs=FORCE_CRS);

	/// Sets projection defined as EPSG code.
	/**
	 *
	 */
	void setProjection(int epsg, CRS_mode crs=FORCE_CRS);

	// protect
	void updateProjectionDefs(CRS_mode crs);

	/// Returns true, if PJ object has been set.
	inline
	bool isSet() const {
		return (pj != nullptr);
	}

	// NOTE: compliancy problems Proj.4...6...
    inline
	bool isLongLat() const {
    	// TODO: what if not defined.
    	return isLongLat(pj);
    }

    inline
	int getEPSG() const {
    	return epsg;
    }



	/// Prunes "+init=epsg:<...>" and optionally "+type=crs" codes.
	//static
	// int filterProjStr(const std::string & src, std::ostream & ostr, CRS_mode crs=ACCEPT_CRS);


	inline
    void info(std::ostream & ostr = std::cout, int wkt = -1){

		for (const auto & entry: projDefs){
			ostr << entry.first << ": '" << entry.second << "'\n";
		}
		info(pj, ostr, wkt);
    }

    inline
    std::string getErrorString() const {
    	int err = proj_context_errno(pjContext);
    	return proj_errno_string(err);
    	//return "Not Impl."; //std::string(pj_strerrno(*pj_get_errno_ref()));
    };

// to-be protected:

    /// Dump misc information, for debugging
    void info(PJ *pj, std::ostream & ostr = std::cout, int wkt = -1);

protected:

    // const std::string & getProjection(const PJ *prj, std::string & projDef) const;
    ProjDef projDefDict;

	mutable
	std::map<PROJDEF_variant,std::string> projDefs;


	/// For setting projection.
	/**
	 * \param projStr - string in Proj format
	 * \param CRS     - set or unset
	 */
	// PJ * getProjection(const std::string & projStr, CRS_mode crs=ACCEPT_CRS) const;

    static
    bool isLongLat(const PJ *prj);

	PJ_CONTEXT *pjContext;
	PJ *pj;

	//mutable
	//std::string projDef;

	int epsg;

};

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


	static
	const std::string & getProjVersion();

	Projector src;
	Projector dst;


	/// Sets source projection.
	/**
	 *
	 */
	inline
	void setProjectionSrc(const std::string & projDef, Projector::CRS_mode crs=Projector::FORCE_CRS){
		src.setProjection(projDef, crs);
		setMapping(true);
	}

    /// Sets destination projection.
	inline
	void setProjectionDst(const std::string & projDef, Projector::CRS_mode crs=Projector::FORCE_CRS){
		dst.setProjection(projDef, crs);
		setMapping(true);
	}

    /// Sets source and destination projection.
	inline
	void setProjections(const std::string & projDefSrc, const std::string & projDefDst){
		src.setProjection(projDefSrc, Projector::FORCE_CRS);
		dst.setProjection(projDefDst, Projector::FORCE_CRS);
		setMapping(false);
	}


	//void setMapping(const std::string & proj1, const std::string & proj2);


   	/// Returns the projection std::string applied by the last setProjection call.
   	/**!
   	 *   Get a PROJ string representation of an object.
   	 *
   	 *   Does not reconstruct it from the allocated proj structure.
   	 */
    inline
    const std::string & getProjectionSrc() const {
    	return src.getProjDef();
    	//return getProjection(projSrc, projStrSrc);
    };

    inline
    const std::string & getProjectionDst() const {
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


protected:

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


public:

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


	/// Forward projection.
	/*
	inline
	void projectFwdOLD(double & x, double & y) const {
		PJ_COORD coord;
		coord.lp.lam = x;
		coord.lp.phi = y;
		proj_trans_array(proj, PJ_DIRECTION::PJ_FWD, 1, &coord);
		x = coord.xy.x;
		y = coord.xy.y;
	};
	*/

	/// Forward projection.
	/*
	inline
	void projectFwdOLD(const double & x, const double & y,double & x2, double & y2) const {

		PJ_COORD coord;
		coord.lp.lam = x;
		coord.lp.phi = y;

		proj_trans_array(proj, PJ_DIRECTION::PJ_FWD, 1, &coord);
		x2 = coord.xy.x;
		y2 = coord.xy.y;

	}
	*/

    /// Todo: projections for vectors
    ///inline


    /// Inverse projection.
    /*
	inline
    void projectInvOLD(double & x, double & y) const
    {

    	PJ_COORD coord;
    	coord.xy.x = x;
    	coord.xy.y = y;

    	proj_trans_array(proj, PJ_DIRECTION::PJ_INV, 1, &coord);
    	x = coord.lp.lam;
    	y = coord.lp.phi;

    };
    */

    /// Inverse projection from (x2,y2) to (x,y).
    /*
    inline
    void projectInvOLD(const double & x2, const double & y2, double & x, double & y) const
    {

    	PJ_COORD coord;
    	coord.xy.x = x2;
    	coord.xy.y = y2;

    	proj_trans_array(proj, PJ_DIRECTION::PJ_INV, 1, &coord);
    	x = coord.lp.lam;
    	y = coord.lp.phi;

    };
    */


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

    // typedef drain::Dictionary<int, std::string> epsg_dict_t;
	void setMapping(bool lenient);




	
};

DRAIN_TYPENAME(Proj6);

std::ostream & operator<<(std::ostream & ostr, const Proj6 &p);

} // drain


#endif /*PROJ4_H_*/

