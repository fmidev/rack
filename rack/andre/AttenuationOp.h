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

#ifndef Attenuation_OP_H_
#define Attenuation_OP_H_

#include "DetectorOp.h"
//#include "QualityCombinerOp.h"

//#include <drain/image/SegmentAreaOp.h>
//#include <drain/util/Fuzzy.h>
//#include <drain/image/MathOpPack.h>
//#include <drain/image/File.h>
//#include "product/Analysis.h"

//using namespace drain::image;



namespace rack {

/// Simply, "detects" precipitation ie sets its probability.
/**
 *  Sets default class, typically with a low maximum prority.
 *  Applicable in context of str detectors to prevent clear precipitation from being classified to str classes.
 */
class AttenuationOp: public DetectorOp {

public:

	/**
	 * \param prob - maximum expected reflectance of biometeors
	 * \param maxAltitude - maximum expected altitude of biometeors
	 * \param reflDev - fuzzy width of  \c reflMax
	 * \param devAltitude - fuzzy width of \c maxAltitude
	 */
	AttenuationOp() :
		DetectorOp(__FUNCTION__, "Computes attenuation", "distort.attn"){ //ECHO_CLASS_PRECIP){

		parameters.reference("dBZHalfWidth", this->dBZHalfWidth=10.0, "limit of 50% quality");

		// todo: "rain", "snow"
		parameters.reference("p", this->p=0.62, "p");
		parameters.reference("c", this->c=1.12E-7, "c");

		UNIVERSAL = true;
		dataSelector.quantity = "DBZH$";
		dataSelector.count = 1;
		REQUIRE_STANDARD_DATA = false;
	};

	double dBZHalfWidth;
	double p;
	double c;

protected:

	virtual
	void processData(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const;

};




}

#endif /* BIOMET_OP_H_ */

// Rack
