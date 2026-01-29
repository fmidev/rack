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
#ifndef DRAIN_PROJECTOR_H_
#define DRAIN_PROJECTOR_H_

#include <drain/util/EnumUtils.h>
#include <cstddef>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

#include <proj.h>

// #include "Dictionary.h"
// #include "Point.h"


namespace drain
{

class Proj6;


// Helper class for containing a single (one-way) Proj object and its
class Projector {

	friend class Proj6;

	static const SprinterLayout projDefLayout; // space-separated, =, no hypens  (" ","","=", "","");


public:

	static const std::string proj4version;

	typedef drain::Dictionary<std::string,std::string> ProjDef;

	//typedef enum {PROJ4, PROJ6, CRS} version;
	typedef enum {
		ACCEPT_CRS, // Do not touch
		REMOVE_CRS, // Delete "+type=crs" from projDef, warn if EPSG:<code> syntax used
		FORCE_CRS   // Add "+type=crs"
	} CRS_mode;


	/// Each projector has three (3) versions of project definition
	typedef enum {
		ORIG,    // Supplied by user
		MODIFIED, // EPSG-converted and filtered
		// FINAL,    //
		PROJ4, // Final, formulated by projlib
		PROJ5, // Final, formulated by projlib
		SIMPLE // Final, simplified for backward compatibility: "+type=crs" and "+init=epsg:..." pruned.
	} PROJDEF_variant;


	inline  // NEW 2024: proj_context_create()
	Projector(const std::string & projDef = "", CRS_mode crs=ACCEPT_CRS) : pjContext(proj_context_create()), pj(nullptr), epsg(0){
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
			// CONSIDER projeDef.empty() check in setProjection(projDef, crs); ?
			projDefs = {{ORIG,""}, {MODIFIED,""}, {PROJ4,""}, {PROJ5,""}, {SIMPLE,""}};
		}
		else {
			setProjection(projDef, crs);
		}

	}

	// 	Moved to .cpp for version 6/7/8 problems (context_clone)
	Projector(const Projector & pr);


	virtual inline
	~Projector(){
		proj_destroy(pj);
		proj_context_destroy(pjContext); // NEW 2024
	}


	static
	int extractEPSG(const std::string & projDef);


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
	void createProjection(CRS_mode crs);

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
    void info(std::ostream & ostr = std::cout, int wkt = -1) const {
		/*
		for (const auto & entry: projDefs){
			ostr << entry.first << ": '" << entry.second << "'\n";
		}
		*/
		info(pj, ostr, wkt);
    }

    inline
    std::string getErrorString() const {
    	int err = proj_context_errno(pjContext);
    	return proj_errno_string(err);
    	//return "Not Impl."; //std::string(pj_strerrno(*pj_get_errno_ref()));
    };

// To be protected?


protected:

    /// Dump misc information, for debugging
    void info(PJ *pj, std::ostream & ostr = std::cout, int wkt = -1) const ;

	/// Metadata for PROJ (introduced in latest versions, currently not used by Drain & Rack )
	PJ_CONTEXT *pjContext;

    /// Essential member: the pointer to a PJ object. This is the primary description of state.
	PJ *pj;

    /// Secondary description of state.
    ProjDef projDefDict;

	///
	int epsg = 0;

	mutable
	std::map<PROJDEF_variant,std::string> projDefs;

	void storeProjDef(const std::string & str);

    static
    void getProjDefStr(const ProjDef & dict, std::stringstream & sstr, const std::set<std::string> & excludeKeys = {"+type"});

	// static
	// int extractEPSGold(const ProjDef & projDefDict);

	/// For setting projection.
	/**
	 * \param projStr - string in Proj format
	 * \param CRS     - set or unset
	 */
	// PJ * getProjection(const std::string & projStr, CRS_mode crs=ACCEPT_CRS) const;

    static
    bool isLongLat(const PJ *prj);


};

//template <>
//const drain::EnumDict<Projector::PROJDEF_variant>::dict_t  drain::EnumDict<Projector::PROJDEF_variant>::dict;
DRAIN_ENUM_DICT(Projector::PROJDEF_variant);

DRAIN_ENUM_OSTREAM(Projector::PROJDEF_variant);


} // drain


#endif /*PROJ4_H_*/

