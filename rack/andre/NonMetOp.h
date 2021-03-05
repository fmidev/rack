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
#ifndef RHOHV3_OP2_H_
#define RHOHV3_OP2_H_

#include "DetectorOp.h"

using namespace drain::image;

namespace rack {

///	Marks bins with low \em RhoHV value as probable anomalies.
/**
 *
 */
class NonMetOp: public DetectorOp {

public:

	///	Default constructor.
	/**
	 *  \param threshold - \c RhoHV values below this will be considered this anomalies
	 *  \param thresholdWidth - steepness coefficient for the threshold
	 *  \param windowWidth  - optional: median filtering window width in metres
	 *  \param windowHeight - optional: median filtering height width in degrees
	 *  \param medianPos    - optional: median position: 0.5 = normal median ( >0.5: conservative for precip)
	 *  //(morphology: 0.5 > opening; 0.5 < closing)
	 *
	 *  This operator is \e universal , it is computed on DBZ but it applies also to str radar parameters measured (VRAD etc)
	 */
	NonMetOp(double threshold=0.5, double thresholdWidth=0.1, double windowWidth=0.0, double windowHeight=0.0, double medianPos=0.95) :
		DetectorOp(__FUNCTION__,"Detects clutter. Based on dual-pol parameter RhoHV . Optional postprocessing: morphological closing. Universal.", "nonmet"){

		dataSelector.quantity = "RHOHV";
		REQUIRE_STANDARD_DATA = false;
		UNIVERSAL = true;

		parameters.link("threshold", this->threshold.tuple(), "0...1[:0...1]");
		this->threshold.min = threshold-thresholdWidth;
		this->threshold.max = threshold+thresholdWidth;
		//parameters.link("thresholdWidth", this->thresholdWidth = thresholdWidth, "0...1");
		parameters.link("windowWidth", this->windowWidth = windowWidth, "metres");
		parameters.link("windowHeight", this->windowHeight = windowHeight, "degrees");
		parameters.link("medianPos", this->medianPos = medianPos, "0...1");
		//parameters.link("area", this->area, area);
	};

	/*
	inline
	NonMetOp(const NonMetOp & op) : DetectorOp(op){
		UNIVERSAL = true;
		std::cerr << __FUNCTION__ << " copy const \n";
	}
	*/

	//double threshold;
	//double thresholdWidth;
	drain::Range<double> threshold;

	double windowWidth;
	double windowHeight;
	double medianPos;

protected:

	virtual
	void processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const;

};


}

#endif /* POLARTOCARTESIANOP_H_ */
// koe

// Rack
