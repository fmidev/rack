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
		DetectorOp(__FUNCTION__,"Detects ships based on their high absolute reflectivity and local reflectivity difference.", "nonmet.artef.vessel.ship"){
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

// Rack
