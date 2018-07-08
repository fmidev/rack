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
#ifndef ZDRvar3_OP2_H_
#define ZDRvar3_OP2_H_

#include "DetectorOp.h"

using namespace drain::image;

namespace rack {

/**
 *
 */
class ZDRvarOp: public DetectorOp {

public:

	///	Computes variance of ZDR in a window.
	/**
	 *  \param threshold - \c ZDRvar values below this will be considered this anomalies
	 *  \param thresholdWidth - steepness coefficient for the threshold
	 *  \param windowWidth  - optional: median filtering window width in metres
	 *  \param windowHeight - optional: median filtering height width in metres
	 *  \param medianPos    - optional: median position: 0.5 = normal median (morphology: 0.5 > opening; 0.5 < closing)
	 *
	 *  This operator is \e universal , it is computed on DBZ but it applies also to other radar parameters measured (VRAD etc)
	 */
	ZDRvarOp(double threshold=0.5, double thresholdWidth=0.1, double windowWidth=5.0, double windowHeight=5.0, double medianPos=0.3) :
		DetectorOp("__FUNCTION__","Detects clutter. Based on dual-pol parameter ZDRvar . Optional postprocessing: morphological closing. Universal."){
		dataSelector.quantity = "ZDRvar";
		REQUIRE_STANDARD_DATA = false;
		UNIVERSAL = true;
		parameters.reference("threshold", this->threshold = threshold);
		parameters.reference("thresholdWidth", this->thresholdWidth = thresholdWidth);
		parameters.reference("windowWidth", this->windowWidth = windowWidth);
		parameters.reference("windowHeight", this->windowHeight = windowHeight);
		parameters.reference("medianPos", this->medianPos = medianPos);
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

};


}

#endif /* POLARTOCARTESIANOP_H_ */
// koe
