/**

    Copyright 2010-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

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
#ifndef RHOHV3_OP2_H_
#define RHOHV3_OP2_H_

#include "DetectorOp.h"

using namespace drain::image;

namespace rack {

///	Marks bins with low \em RhoHV value as probable anomalies.
/**
 *
 */
class RhoHVLowOp: public DetectorOp {

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
	 *  This operator is \e universal , it is computed on DBZ but it applies also to other radar parameters measured (VRAD etc)
	 */
	RhoHVLowOp(double threshold=0.85, double thresholdWidth=0.1, double windowWidth=0.0, double windowHeight=0.0, double medianPos=0.95) :
		DetectorOp("RhoHVLow","Detects clutter. Based on dual-pol parameter RhoHV . Optional postprocessing: morphological closing. Universal.", 131){
		dataSelector.quantity = "RHOHV";
		REQUIRE_STANDARD_DATA = false;
		UNIVERSAL = true;
		parameters.reference("threshold", this->threshold = threshold, "0...1");
		parameters.reference("thresholdWidth", this->thresholdWidth = thresholdWidth, "0...1");
		parameters.reference("windowWidth", this->windowWidth = windowWidth, "metres");
		parameters.reference("windowHeight", this->windowHeight = windowHeight, "degrees");
		parameters.reference("medianPos", this->medianPos = medianPos, "0...1");
		//parameters.reference("area", this->area, area);
	};

	double threshold;
	double thresholdWidth;

	double windowWidth;
	double windowHeight;
	double medianPos;

protected:

	virtual
	void processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const;
	//void filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const;

};


}

#endif /* POLARTOCARTESIANOP_H_ */
// koe
