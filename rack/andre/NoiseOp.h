/**

    Copyright 2013-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#ifndef Noise_OP_H_
#define Noise_OP_H_

#include <drain/image/File.h>
#include "DetectorOp.h"

using namespace drain::image;

namespace rack {

/// A radio interference detector.

class NoiseOp: public DetectorOp {

public:

	/** Default constructor.
	 *   \param minRange - starting distance for detection
	 *   \param minLength - minimum segment length [bins] to be considered
	 *   \param sensitivity - parameter for detection sensitivity, from 0.0 to 1.0 (sensitive).
	 */
	NoiseOp(double minRange = 50, int minLength = 10, double sensitivity = 2.0) :
		DetectorOp(__FUNCTION__,"Detects sectors contaminated by electromagnetic interference.", ECHO_CLASS_NOISE)
	{
		parameters.reference("minRange",  this->minRange = minRange, "km");
		parameters.reference("minLength",  this->minLength = minLength, "bins");
		parameters.reference("sensitivity",  this->sensitivity =  sensitivity, "0..1");
		UNIVERSAL = true;
		REQUIRE_STANDARD_DATA = true;
	}

	int minLength;
	double sensitivity;
	double minRange;

protected:

	virtual
	void processData(const PlainData<PolarSrc> &src, PlainData<PolarDst> &dst) const;
	//void filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const;

};

}

#endif
