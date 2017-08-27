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

#ifndef THR_OP2_H_
#define THR_OP2_H_

#include "DetectorOp.h"

//#include <drain/image/SegmentAreaOp.h>
#include <drain/util/Fuzzy.h>
//#include <drain/image/MathOpPack.h>
#include <drain/image/File.h>

#include "radar/Geometry.h"



using namespace drain::image;



namespace rack {

/**  DEFUNCT / OBSOLETE
 */
class FThresholdOp: public DetectorOp {

public:

	FThresholdOp(const std::string & p = "") : // todo modernize
		DetectorOp(__FUNCTION__,"Fuzzy threshold operator."){
		//pathFilter = "**/data1";
		//propertyFilter["what:quantity"] = "DBZ.*";
		dataSelector.quantity = "DBZ.*";

		parameters.reference("maxDBZ", maxDBZ = 0);
		parameters.reference("dBZWidth", dBZWidth = 5);
	};

	int maxDBZ;
	int dBZWidth;

protected:

	virtual
	void processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const;
	//void filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const;

};



}

#endif /* Threshold_OP_H_ */

// Rack
