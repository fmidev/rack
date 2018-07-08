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
#ifndef CCOROP_H_
#define CCOROP_H_

#include "DetectorOp.h"

using namespace drain::image;

namespace rack {

/**
 *
 */
class CCorOp: public DetectorOp {

public:

	///	Computes sizes of segments having intensity over minDBZ.
	/**
	 *  \param reflHalfWidth - \c CCOR values below this will be considered this anomalies
	 *  \param thresholdWidth - steepness coefficient for the reflHalfWidth
	 *  \param windowWidth  - optional: median filtering window width in metres
	 *  \param windowHeight - optional: median filtering height width in metres
	 *  \param medianPos    - optional: median position: 0.5 = normal median (morphology: 0.5 > opening; 0.5 < closing)
	 *
	 *  This operator is \e universal , it is computed on DBZ but it applies also to other radar parameters measured (VRAD etc)
	 */
	CCorOp(double reflHalfWidth=40) :
		DetectorOp(__FUNCTION__,"Detects clutter. Based on difference of DBZH and TH. At halfwidth PROB=0.5. Universal.", ECHO_CLASS_CLUTTER){ // Optional postprocessing: morphological closing.
		dataSelector.path = ".*/data[0-9]+/?$";
		dataSelector.quantity = "^(TH|DBZH)$";
		REQUIRE_STANDARD_DATA = false;
		UNIVERSAL = true;
		parameters.reference("reflHalfWidth", this->reflHalfWidth = reflHalfWidth, "dBZ");
		//parameters.reference("thresholdWidth", this->thresholdWidth, thresholdWidth);
		//parameters.reference("windowWidth", this->windowWidth, windowWidth);
		//parameters.reference("windowHeight", this->windowHeight, windowHeight);
		//parameters.reference("medianPos", this->medianPos, medianPos);
		//parameters.reference("area", this->area, area);
	};

	double reflHalfWidth;
	// double thresholdWidth;
	// double windowWidth;
	// double windowHeight;
	// double medianPos;

	//void filter(const HI5TREE &src, HI5TREE &dst) const;


	/// NEW POLICY => DetectorOpNEW
	/*
	virtual
	void processDataSets(const DataSetMap<PolarSrc> & srcVolume, DataSetDstMap & dstVolume) const;
	*/

	/// NEW POLICY
	virtual
	void processDataSet(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dst, DataSet<PolarDst> & aux) const;  // DataSetDst & dst) const;





};


}

#endif
