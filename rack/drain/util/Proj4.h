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

// Drain
