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
*//**

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

	///	Computes sizes of segments having intensity over threshold.
	/**
	 *  \param threshold - threshold reflectivity
	 *  \param area - fuzzy threshold, at which the likelihood is 50%
	 *  \param polar - use bin geometry; else scale to Cartesian bin area
	 *
	 *  This operator is \e universal , it is computed on DBZ but it applies also to str radar parameters measured (VRAD etc)
	 */
	inline
	SpeckleOp(double threshold=-20.0, int area=16, bool invertPolar=false) :

	DetectorOp(__FUNCTION__,"Detects speckle noise. Universal: uses DBZ data as input, applies to all data in a sweep group.", "emitter.noise"){
		dataSelector.quantity = "^DBZH$";
		UNIVERSAL = true;
		REQUIRE_STANDARD_DATA = false;

		parameters.link("threshold", this->threshold = threshold, "dBZ");
		parameters.link("area", this->area = area, "bins");
		parameters.link("invertPolar", this->invertPolar = invertPolar, "bins");
	};

	inline
	SpeckleOp(const SpeckleOp & op) : DetectorOp(op) {
		parameters.copyStruct(op.getParameters(), op, *this);
	}

	double threshold;
	int area;
	bool invertPolar;

protected:

	virtual
	void runDetector(const PlainData<PolarSrc> &src, PlainData<PolarDst> &dst) const;

};


}

#endif /* POLARTOCARTESIANOP_H_ */
