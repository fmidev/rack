/**

    Copyright 2001 - 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)

    This file is part of AnoRack, radar data processing utilities for C++.

 *  Created on: Mar 7, 2011
 *      Author: mpeura
 */

#ifndef SPECKLEOP2_H_
#define SPECKLEOP2_H_

#include "DetectorOp.h"

using namespace drain::image;

namespace rack {

/**
 *
 */
class SpeckleOp: public DetectorOp {

public:

	///	Computes sizes of segments having intensity over reflMin.
	/**
	 *  \param reflMin - threshold reflectivity
	 *  \param area - fuzzy threshold, at which the likelihood is 50%
	 *
	 *  This operator is \e universal , it is computed on DBZ but it applies also to other radar parameters measured (VRAD etc)
	 */
	inline
	SpeckleOp(double reflMin=0.0, int area=9) :
		DetectorOp(__FUNCTION__,"Detects speckle noise. Universal: uses DBZ data as input but applies to all data in a sweep group.", ECHO_CLASS_NOISE){
		dataSelector.quantity = "^DBZH$";
		UNIVERSAL = true;
		parameters.reference("reflMin", this->reflMin = reflMin, "dBZ");
		parameters.reference("area", this->area = area, "bins");
		REQUIRE_STANDARD_DATA = false;
	};

	double reflMin;
	int area;
	//std::string area;

protected:

	virtual
	void processData(const PlainData<PolarSrc> &src, PlainData<PolarDst> &dst) const;

};


}

#endif /* POLARTOCARTESIANOP_H_ */
