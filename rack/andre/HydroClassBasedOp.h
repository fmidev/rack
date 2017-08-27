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

#ifndef HYDROCLASSBASED_H_
#define HYDROCLASSBASED_H_


#include <drain/image/SlidingWindowMedianOp.h>

#include <drain/image/FastAverageOp.h>
#include <drain/image/HighPassOp.h>
#include <drain/image/SegmentAreaOp.h>
#include <drain/image/RunLengthOp.h>
#include <drain/image/BasicOps.h>


#include <drain/image/DistanceTransformOp.h>
//#include <drain/image/FuzzyOp.h>
//#include <drain/image/FuzzyThresholdOp.h>

#include "DetectorOp.h"

// temp
//#include <drain/radar/Andre.h>


using namespace drain::image;


namespace rack {

/// Sets probabilities of non-meteorological echoes based on precomputed hydrometeor classification.
/**
 *
 */
class HydroClassBasedOp: public DetectorOp {
public:

	/**
	 * \param minDBZ - minimum reflectance
	 *
	 */
	HydroClassBasedOp(double nonMet = 0.5, unsigned int NOMET = 1) :
		DetectorOp(__FUNCTION__, "Sets probabilities of non-meteorological echoes based on precomputed hydrometeor CLASS.", 132){
		dataSelector.quantity = "HCLASS";
		REQUIRE_STANDARD_DATA = false;
		UNIVERSAL = true;
		parameters.reference("nonMet", this->nonMet = nonMet);
		//parameters.reference("smooth", this->smooth, smooth);
		parameters.reference("NOMET", this->NOMET = NOMET); // VAISALA code
	};


	double nonMet;
	unsigned int NOMET;
	//double smooth;


protected:

	//virtual
	//void filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const;
	virtual
	void processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const;

};


}  // rack::

#endif /* POLARTOCARTESIANOP_H_ */

// Rack
