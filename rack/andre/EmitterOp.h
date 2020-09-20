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

#ifndef EMITTER_SEGMENT_OP_H_
#define EMITTER_SEGMENT_OP_H_

#include "drain/util/FunctorPack.h"

#include "drain/image/File.h"

#include "drain/imageops/SegmentAreaOp.h"
#include "drain/imageops/RunLengthOp.h"
#include "drain/imageops/DistanceTransformOp.h"

#include "DetectorOp.h"


//using namespace drain::image;

using namespace drain::image;

namespace rack {

/// A quick emitter detection based on subtracting horizontal window medians by vertical ones.
// TODO Optional enchancement.
class EmitterOp: public DetectorOp {

public:

	/** Default constructor
	 *   \param thicknessMax - azimuthal width [degrees]
	 *   \param samplingDistance - position [km] of fuzzy peak weighting the sampling
	 *   \param samplingDistanceWidth - halfwidth of the weighting curve
	 */
	//EmitterOp(int windowWidth=5000, float thicknessMax=10.0, float sensitivity = 0.5, float eWidth = 1.0f, float eHeight = 0.0f) :
	//EmitterOp(double thicknessMax=5.0, int samplingDistance = 100, int samplingDistanceWidth = 50) :
	EmitterOp(double lengthMin=5.0, double thicknessMax=5.0, double sensitivity=0.5) :
		DetectorOp(__FUNCTION__, "Detects electromagnetic interference segments by means of window medians.", "signal.emitter.line")
	{
		parameters.reference("lengthMin", this->lengthMin = lengthMin, "km");
		parameters.reference("thicknessMax",  this->thicknessMax = thicknessMax, "deg");
		parameters.reference("sensitivity",  this->sensitivity = sensitivity, "0...1");
		REQUIRE_STANDARD_DATA = true;
	}

	// void computeAverageBeamDBZ();

	double lengthMin;
	double thicknessMax;
	double sensitivity;

	int samplingDistance;
	int samplingDistanceWidth;

protected:

	virtual
	void processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const;

};

}

#endif /* POLARTOCARTESIANOP_H_ */

// Rack
