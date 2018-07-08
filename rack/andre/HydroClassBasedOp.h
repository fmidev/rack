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

#ifndef HYDROCLASSBASED_H_
#define HYDROCLASSBASED_H_

#include <drain/util/FunctorPack.h>

#include <drain/imageops/SlidingWindowMedianOp.h>
#include <drain/imageops/FastAverageOp.h>
#include <drain/imageops/HighPassOp.h>
#include <drain/imageops/SegmentAreaOp.h>
#include <drain/imageops/RunLengthOp.h>
#include <drain/imageops/DistanceTransformOp.h>

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
