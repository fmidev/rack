/**

    Copyright 2010-2013   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#ifndef EMITTER_OP_H_
#define EMITTER_OP_H_


//#include "imageops/GammaOp.h"
//#include "imageops/HighBoostOp.h"


#include <drain/image/File.h>

#include "DetectorOp.h"


//using namespace drain::image;

using namespace drain::image;

namespace rack {

/// Quick emitter detection based on subtracting a horizontal segment lengths by inversed vertical segment lengths.
// UNDER CONSTRUCTION.
// TODO Optional enchancement.
class LineOp: public DetectorOp {

public:

	/*
	 *
	 *   \param sensitivity - "contrast" between horz and vert medians (0.0...1.0)  [0=detection, 1.0=overdetection]
	 *   \param enhancement  - post-enhancement: median reflMin for directional statistics
	 *   \param eHeight     - post-enhancement: spread radius for directional statistics
	 *
	 */

	/** Default constructor.
	 *   \param lengthMin - lengthMin [kilometres]
	 *   \param widthMax - widthMax [degrees]
	 *   \param reflMin  - threshold for reflectance
	 *   \param reflMin2 - optional 2nd threshold for reflectance
	 *   \param reflMin3 - optional 3rd threshold for reflectance
	 */
	LineOp(double lengthMin=5, double widthMax=10.0, double reflMin=-10.0, double reflMin2=std::numeric_limits<double>::min(), double reflMin3=std::numeric_limits<double>::min()) : //, double sensitivity=1.0) : //, double enhancement = 0.0) :
		DetectorOp(__FUNCTION__, "Detects lines caused by electromagnetic interference.", ECHO_CLASS_EMITTER)
	{
		REQUIRE_STANDARD_DATA = true;
		parameters.reference("lengthMin",  this->lengthMin = lengthMin, "km");
		parameters.reference("widthMax", this->widthMax = widthMax, "deg");
		parameters.reference("reflMin", this->reflMin = reflMin, "dBZ");
		parameters.reference("reflMin2", this->reflMin2 = reflMin2, "dBZ");
		parameters.reference("reflMin3", this->reflMin3 = reflMin3, "dBZ");
		//parameters.reference("sensitivity",  this->sensitivity,  sensitivity);
		// parameters.reference("enhancement",  this->enhancement,  enhancement);
		// parameters.reference("eHeight",  this->eHeight, eHeight);
		dataSelector.quantity = "DBZ.*";
		//double d =
	}


	double lengthMin;
	double widthMax;
	double reflMin;
	double reflMin2;
	double reflMin3;

	double sensitivity;
	//double enhancement;
	//float eHeight;


protected:

	virtual
	//void filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const;
	void processData(const PlainData<PolarSrc> &src, PlainData<PolarDst> &dst) const;
};

}

#endif /* POLARTOCARTESIANOP_H_ */
