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

#include <cstddef>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

#include <proj.h>

#include "Dictionary.h"
// #include "Point.h"
// #include "TreeUnordered.h"


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
			// clear(); don't use this before designing: proj_context
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
			// clear(); don't use this before designing: proj_context
			projDefs = {{ORIG,""}, {MODIFIED,""}, {PROJ4,""}, {PROJ5,""}, {SIMPLE,""}};
		}
		else {
			setProjection(projDef, crs);
		}

	}

	// 	Moved to .cpp for version 6/7/8 problems (context_clone)
	Projector(const Projector & pr);

	/*
	inline
	Projector(const Projector & pr) :
		pjContext(proj_context_clone(pr.pjContext)), // NEW 2024
		// pjContext(nullptr), // for safety
		// TODO: CLONE, in version 7.2.
		 // TODO: flag for own CTX => destroy at end
		pj(proj_clone(pjContext, pr.pj)),
		// projDefDict(pr.projDefDict),
		epsg(pr.epsg)
	{
		// TODO
		//projDefs = {{ORIG,"*"}, {CHECKED,"**"}, {FINAL,"***"}, {INTERNAL,"****"}};
		setProjection(pr.getProjDef(ORIG)); // imitate ?
	}
		*/

	virtual inline
	~Projector(){
		proj_destroy(pj);
		proj_context_destroy(pjContext); // NEW 2024
	}


	inline
    const ProjDef & getProjDefDict() const {
    	return projDef;
    }

	const std::string & getProjDef(PROJDEF_variant v = SIMPLE) const { // For external objects, excluding +init=epsg and +type=crs
		return projDefs[v];
	}

	/// Extracts numeric EPSG code from plain number or from "EPSG:<code>", +init
	static
	int extractEPSG(const std::string & s);

	/// Traverses entries and searches for "+init..." or "EPSG" entry
	static
	int extractEPSG(const ProjDef & projDefDict);


	/// Deletes projection object and clears all the metadata.
	void clear();
	/*{
		projDefs = {{ORIG,""}, {MODIFIED,""}, {PROJ4,""}, {PROJ5,""}, {SIMPLE,""}};
		pjContext = nullptr; // TODO: flag for own CTX => destroy at end
		proj_destroy(pj);
		pj = nullptr;
		projDef.clear();
		epsg = 0;
	}
	*/

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

    /// Essential Proj.6 pointers.

	PJ_CONTEXT *pjContext = nullptr;
	PJ *pj = nullptr;

	int epsg = 0;

	/// Prunes "+init=epsg:<...>" and optionally "+type=crs" codes.
	/*
	 *   Optionally, prunes "+init=epsg:<...>" and optionally "+type=crs" codes.
	 */
    //static
    //void getProjDefStr(const ProjDef & dict, std::stringstream & sstr, const std::set<std::string> & excludeKeys = {"+type"});
	void getProjDefStr(std::stringstream & sstr, const std::set<std::string> & excludeKeys = {"+type"}) const ;

	// protect
	// void updateProjectionDefs(CRS_mode crs);
    //static
    void getProjDefDict(const std::string & str); // ProjDef & dict);


    /// After ProjDef[s] have been populated, create the actual projection.
    void createProjection(const std::string & projDefStr, CRS_mode crs);


    /// Projection definition as a map(key, value)
    ProjDef projDef;

    /// Variants of Proj.4 "projdef" strings for current projection.
	mutable
	std::map<PROJDEF_variant,std::string> projDefs;

    static
    bool isLongLat(const PJ *prj);


};

} // drain


#endif /*PROJ4_H_*/

