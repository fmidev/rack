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
#ifndef ANDRE_MARK_SECTOR_OP
#define ANDRE_MARK_SECTOR_OP

#include "DetectorOp.h"

// using namespace drain::image;

namespace rack {

///	Create rectangular fake anomaly (value) and 'detect' it with desired probability (prob)
/**
 *  After "detection", typically aRemover or aGapFill can be run.
 *
 */
class MarkSectorOp: public DetectorOp {

public:

	///	Create rectangular fake anomaly (value) and 'detect' it with desired probability (prob)
	/**
	 *
	 *
	 */
	inline
	MarkSectorOp() : // const std::string & value="nodata", double prob=1.0
	DetectorOp(__FUNCTION__, "Create rectangular fake anomaly (value) and 'detect' it with desired probability (prob).", "nonmet.clutter"){
		//RemoverOp(__FUNCTION__, "Create rectangular fake anomaly (value) and 'detect' it with desired probability (prob)."){ // Optional postprocessing: morphological closing.

		dataSelector.setQuantities("^DBZH$");
		REQUIRE_STANDARD_DATA = false;
		UNIVERSAL = true;

		parameters.link("prob",  this->prob  = prob,  "Probability of anomaly");

		parameters.link("i", this->iRange.tuple() = {200,230}, "radial range [bins]"); // ~500m
		parameters.link("j", this->jRange.tuple() = {240,300}, "azimuthal range [bins]"); // deg

		parameters.link("value", this->value = value, "(empty)|value|'nodata'|'undetect'");
		parameters.link("physicalValue", physicalValue,  "treat as quantity-specific real value");

		// TODO: classEntry.code (final code derived from numeric or (partial) string)
		// parameters.link("code", this->code = "clutter.test", "class code"); // deg

	};

	//double value;
	std::string value = ""; // "nodata";
	double prob = 1.0;
	drain::Range<int> iRange;
	drain::Range<int> jRange;
	bool physicalValue = true;

	void runDetection(const DataSet<PolarSrc> & srcDataSet, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & cache) const;


};


}

#endif
// Rack
