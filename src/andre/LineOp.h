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

#ifndef EMITTER_OP_H_
#define EMITTER_OP_H_


//#include "drain/imageops/GammaOp.h"
//#include "drain/imageops/HighBoostOp.h"


#include <drain/image/ImageFile.h>
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
		DetectorOp(__FUNCTION__, "Detects lines caused by electromagnetic interference.", "emitter.line")
	{
		REQUIRE_STANDARD_DATA = true;
		parameters.link("lengthMin",  this->lengthMin = lengthMin, "km");
		parameters.link("widthMax", this->widthMax = widthMax, "deg");
		parameters.link("reflMin", this->reflMin = reflMin, "dBZ");
		parameters.link("reflMin2", this->reflMin2 = reflMin2, "dBZ");
		parameters.link("reflMin3", this->reflMin3 = reflMin3, "dBZ");
		//parameters.link("sensitivity",  this->sensitivity,  sensitivity);
		// parameters.link("enhancement",  this->enhancement,  enhancement);
		// parameters.link("eHeight",  this->eHeight, eHeight);
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
	void runDetector(const PlainData<PolarSrc> &src, PlainData<PolarDst> &dst) const;
};

}

#endif /* POLARTOCARTESIANOP_H_ */

// Rack
