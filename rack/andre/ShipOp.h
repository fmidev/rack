/**

    Copyright 2010-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack library for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef SHIPOP_H_
#define SHIPOP_H_

//#include <drain/image/FuzzyOp.h>

#include "DetectorOp.h"

// temp
//#include <drain/radar/Andre.h>


using namespace drain::image;


namespace rack {

/// Detects ships.
/** Detects ships based on their high absolute reflectivity and local reflectivity difference.
 *
 */
class ShipOp: public DetectorOp {
public:

	/**
	 * \param minDBZ - minimum reflectance of ship
	 * \param reflDev - dbz difference in the neighbourhood window
	 * \param windowWidth  - width of the neighbourhood window; in metres
	 * \param windowHeight - height of the neighbourhood window; in degrees
	 */
	ShipOp(double reflMin = 25.0, double reflDev = 15.0, int windowWidth = 1500, double windowHeight = 3.0) :
		DetectorOp(__FUNCTION__,"Detects ships based on their high absolute reflectivity and local reflectivity difference.", ECHO_CLASS_VEHICLE){
		parameters.reference("reflMin", this->reflMin = reflMin, "dBZ");
		parameters.reference("reflDev", this->reflDev = reflDev, "dBZ");
		parameters.reference("windowWidth", this->windowWidth = windowWidth, "m");
		parameters.reference("windowHeight", this->windowHeight = windowHeight, "deg");
		dataSelector.quantity = "^DBZH$";
		REQUIRE_STANDARD_DATA = false;
		//REQUIRE_STANDARD_DATA = true;  // HighPassOp
	};


	double reflMin;
	int windowWidth;
	double windowHeight;
	double reflDev;


protected:

	virtual
	void processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const;
	// void filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const;

};


}  // rack::

#endif /* SHIP_OP_H_ */
