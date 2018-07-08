/*

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef DRAIN_PROJ4_H_
#define DRAIN_PROJ4_H_


#include <string>
#include <iostream>
#include <stdexcept>

#include <proj_api.h>

// // using namespace std;


namespace drain
{

/*! A wrapper for the famous Proj4 class.
 *  
 *
 */ 
class Proj4
{
public:

	Proj4();

	Proj4(const Proj4 &p);

	virtual ~Proj4(); 
    
	/// Sets source projection.
	inline
	void setProjectionSrc(const std::string &str){ _setProjection(str, projSrc); };

    /// Sets destination projection.
	inline
    void setProjectionDst(const std::string &str){ _setProjection(str, projDst); };
   
	//inline 	projPJ getProjectionSrc(){ return projSrc; };

	//inline 	projPJ getProjectionDst(){ return projDst; };


	/// Forward projection.
	inline
	void projectFwd(double & x, double & y) const {
		/// TODO: is this needed?
		if (projSrc == NULL)
			throw std::runtime_error("Proj4::project(): projSrc NULL");

		if (projDst == NULL)
			throw std::runtime_error("Proj4::project(): projDst NULL");

		pj_transform(projSrc, projDst, 1, 1, &x, &y, NULL);

	};

	/// Forward projection.
	inline
	void projectFwd(const double & x, const double & y,double & x2, double & y2) const {

		/// TODO: is this needed?
		if (projSrc == NULL)
			throw std::runtime_error("Proj4::project(): projSrc NULL");

		if (projDst == NULL)
			throw std::runtime_error("Proj4::project(): projDst NULL");

		x2 = x;
		y2 = y;
		pj_transform(projSrc, projDst, 1, 1, &x2, &y2, NULL);

	}

    /// Forward projection.
    inline
    void projectFwd(double & x, double & y, double & z) const
    {
    	/// TODO: is this needed?
    	if (projSrc == NULL)
    		throw std::runtime_error("Proj4::project(): projSrc NULL");

    	if (projDst == NULL)
    		throw std::runtime_error("Proj4::project(): projDst NULL");

    	//double z = 0.0;
    	pj_transform(projSrc, projDst, 1, 1, &x, &y, &z);

    };

    /// Todo: projections for vectors
    ///inline


    /// Inverse projection.
    inline
    void projectInv(double & x, double & y) const
    {
    	if (projSrc == NULL)
    		throw std::runtime_error("Proj4::project(): projSrc NULL");

    	if (projDst == NULL)
    		throw std::runtime_error("Proj4::project(): projDst NULL");

    	pj_transform(projDst,projSrc, 1, 1, &x, &y, NULL);

    };


    /// Inverse projection from (x2,y2) to (x,y).
    inline
    void projectInv(const double & x2, const double & y2, double & x, double & y) const
    {
    	if (projSrc == NULL)
    		throw std::runtime_error("Proj4::project(): projSrc NULL");

    	if (projDst == NULL)
    		throw std::runtime_error("Proj4::project(): projDst NULL");

    	x = x2;
    	y = y2;
    	pj_transform(projDst,projSrc, 1, 1, &x, &y, NULL);

    };

    /// Inverse projection.
    inline
    void projectInv(double & x, double & y, double & z) const
    {
    	if (projSrc == NULL)
    		throw std::runtime_error("Proj4::project(): projSrc NULL");

    	if (projDst == NULL)
    		throw std::runtime_error("Proj4::project(): projDst NULL");

    	//double z = 0.0;
    	pj_transform(projDst,projSrc, 1, 0, &x, &y, &z);

    };

    

   	/// Returns the projection std::string applied by the last setProjection call.
   	/**! Hence, does not reconstruct it from the allocated proj structure.
   	 */
    inline
    const std::string & getProjectionSrc() const {

    	if (projSrc == NULL){
    		projStrSrc.clear();
    		//	throw std::runtime_error("Proj4::project(): projSrc NULL");
    	}
    	else {
    		//projStrSrc.assign(pj_get_def(projSrc, 0));
    		char *s = pj_get_def(projSrc, 0);
    		projStrSrc.assign(s);
    		//delete s;
    		free(s);
    	}
    	return projStrSrc;
    };

    inline
    const std::string & getProjectionDst() const {
    	if (projDst == NULL){
    		projStrDst.clear();
    		//throw std::runtime_error("Proj4::project(): projDst NULL");
    	}
    	else {
    		char *s = pj_get_def(projDst, 0);
    		projStrDst.assign(s);
    		//delete s;
    		free(s);
    	}
       	return projStrDst;
    };

    
    inline
    std::string getErrorString() const {
    	//projErrorStr = pj_strerrno(*pj_get_errno_ref());
    	return std::string(pj_strerrno(*pj_get_errno_ref()));
    };

	inline
	bool isLongLat() const {
		return pj_is_latlong(projDst);
		//return pj_is_geocent(projSrc);
	}

    inline
    bool isSet() const {
    	return ((projSrc != NULL) && (projDst != NULL));
    }
        
protected:

    void _setProjection(const std::string &src, projPJ &p);

	//std::string projStr;  // obsolete?
	//mutable std::string projErrorStr;
	
    projPJ projSrc;
    projPJ projDst;

private:
    
    mutable std::string projStrSrc;
    mutable std::string projStrDst;
 
	
};

std::ostream & operator<<(std::ostream & ostr, const Proj4 &p);

}

#endif /*PROJ4_H_*/
