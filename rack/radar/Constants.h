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
#ifndef RADAR__CONSTANTS 
#define RADAR__CONSTANTS "radar__constants 1.0, May 27 2006 Markus.Peura@fmi.fi"

//
#include <math.h>


/*! \file
 *  This file contains radar constants...
 */
// 6,370
namespace rack {

  const int EARTH_RADIUSi = 6371000;
  const double EARTH_RADIUS = static_cast<double>(EARTH_RADIUSi);

  const double EARTH_RADIUS_43 = EARTH_RADIUS * 4.0 / 3.0;

  const int     earthRadiusMI = 6371000;  // WHY THIS AS WELL
  const int   earthRadius43MI = (earthRadiusMI * 4) / 3;
  const float earthRadius43MF = static_cast<float>(earthRadiusMI) * 4.0 / 3.0;


  /*! Converts Z from logarithmic to linear scale. 
   *  \f[                                          
   *      Z = 10^{dBZ/10}                          
   *  \f]                                          
   */
  template <class T>                               
  inline
  double dbzToZ(const T &dBZ){
	  //z = ::pow(10.0,dbz*0.10);
	  //return pow(10.0, static_cast<double>(dBZ)/10.0 );
	  return ::pow(10.0,static_cast<double>(dBZ) * 0.1);
  }                                                  

  /*! Converts Z from logarithmic to linear scale.   
   *  \f[                                            
   *      dBZ = 10\log_{10}Z = 10 \ln Z / \ln 10     
   *  \f]                                            
   */
  template <class T>                                 
  inline
  double zToDbz(const T &Z){
	  //static const double coeff = 10.0 / log(10.0);
	  //coeff * log( static_cast<double>(Z) ) ;
	  return 10.0 * ::log10( static_cast<double>(Z) ) ;
  }                                                  


} // ::rack


#endif

// Rack
