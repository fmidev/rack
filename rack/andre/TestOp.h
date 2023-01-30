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
#ifndef ANDRE_TEST_OP_H_
#define ANDRE_TEST_OP_H_

// #include "RemoverOp.h"
#include "DetectorOp.h"

using namespace drain::image;

namespace rack {

/**
 *
 */
class TestOp: public DetectorOp {

public:

	///	Create rectangular fake anomaly (value) and 'detect' it with desired probability (prob)
	/**
	 *  \param reflHalfWidth - \c CCOR values below this will be considered this anomalies
	 *
	 *
	 */
	inline
	TestOp(double value=0.0, double prob=1.0) :
	DetectorOp(__FUNCTION__, "Create rectangular fake anomaly (value) and 'detect' it with desired probability (prob).", "nonmet.clutter"){
		//RemoverOp(__FUNCTION__, "Create rectangular fake anomaly (value) and 'detect' it with desired probability (prob)."){ // Optional postprocessing: morphological closing.

		dataSelector.quantity = "^DBZH$";
		REQUIRE_STANDARD_DATA = false;
		UNIVERSAL = true;

		parameters.link("value", this->value = value, "value");
		parameters.link("prob",  this->prob  = prob,  "prob");

		parameters.link("i", this->iRange.tuple() = {200,230}, "coord range"); // ~500m
		parameters.link("j", this->jRange.tuple() = {240,300}, "coord range"); // deg
	};

	double value;
	double prob;
	drain::Range<int> iRange;
	drain::Range<int> jRange;

protected:

	void processDataSet(const DataSet<PolarSrc> & srcDataSet, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & cache) const;

	/*
	virtual
	void runDetector(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality,
			PlainData<PolarDst> & dstData, PlainData<PolarDst> & dstProb) const;
	*/
	/// NEW POLICY
	// virtual void processDataSet(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dst, DataSet<PolarDst> & aux) const;  // DataSetDst & dst) const;

};


}

#endif
// Rack
