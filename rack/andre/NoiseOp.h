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

// Rack
