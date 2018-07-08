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
#ifndef DRAIN_Projection_H_
#define DRAIN_Projection_H_


#include <string>
#include <iostream>
#include <stdexcept>


// // using namespace std;


namespace drain
{

/*! A wrapper for the famous Projection class.
 *  
 *
 */ 
class Projection
{
public:

	Projection();

	Projection(const Projection &p);

	virtual ~Projection();
    

	/// Forward projection.
	virtual
	void projectFwd(double & x, double & y) const {};

	/// Forward projection.
	virtual
	void projectFwd(const double & x, const double & y, double & x2, double & y2) const {};

    /// Forward projection.
    virtual
    void projectFwd(double & x, double & y, double & z) const {};


    /// Inverse projection.
    inline
    void projectInv(double & x, double & y) const {};


    /// Inverse projection from (x2,y2) to (x,y).
    inline
    void projectInv(const double & x2, const double & y2, double & x, double & y) const {};

    /// Inverse projection.
    inline
    void projectInv(double & x, double & y, double & z) const {};

    

   	/// Returns the projection std::string applied by the last setProjection call.
   	/**! Hence, does not reconstruct it from the allocated proj structure.
   	 */
    //    const std::string & getProjectionSrc() const {};

    /*
    inline
    std::string getErrorString() const {
    	//projErrorStr = pj_strerrno(*pj_get_errno_ref());
    	return std::string(pj_strerrno(*pj_get_errno_ref()));
    };
      */
    

};

// std::ostream & operator<<(std::ostream & ostr, const Projection &p);

}

#endif /*Projection_H_*/
