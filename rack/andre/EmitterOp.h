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

#ifndef EMITTER_SEGMENT_OP_H_
#define EMITTER_SEGMENT_OP_H_

#include <drain/util/FunctorPack.h>

#include <drain/image/File.h>

#include <drain/imageops/SegmentAreaOp.h>
#include <drain/imageops/RunLengthOp.h>
#include <drain/imageops/DistanceTransformOp.h>

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
		DetectorOp(__FUNCTION__, "Detects electromagnetic interference segments by means of window medians.", ECHO_CLASS_EMITTER)
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
