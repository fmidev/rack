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

// Drain
