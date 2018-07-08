/**

    Copyright 2001-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#ifndef PRECIP_OP2_H_
#define PRECIP_OP2_H_

#include "DetectorOp.h"

//#include <drain/image/SegmentAreaOp.h>
//#include <drain/util/Fuzzy.h>
//#include <drain/image/MathOpPack.h>
//#include <drain/image/File.h>
//#include "product/Analysis.h"

using namespace drain::image;



namespace rack {

/// Simply, "detects" precipitation ie sets its probability.
/**
 *  Sets default class, typically with a low maximum prority.
 *  Applicable in context of other detectors to prevent clear precipitation from being classified to other classes.
 */
class PrecipOp: public DetectorOp {

public:

	/**
	 * \param prob - maximum expected reflectance of biometeors
	 * \param maxAltitude - maximum expected altitude of biometeors
	 * \param reflDev - fuzzy width of  \c reflMax
	 * \param devAltitude - fuzzy width of \c maxAltitude
	 */
	PrecipOp(double probMax=0.50, double dbz=20.0, double dbzSpan=+10.0) :
		DetectorOp(__FUNCTION__, "Detects precipitation...", ECHO_CLASS_PRECIP){

		parameters.reference("probMax", this->probMax = probMax, "probability");
		parameters.reference("dbz", this->dbz = dbz, "dBZ");
		parameters.reference("dbzSpan", this->dbzSpan = dbzSpan, "dBZ");

		dataSelector.quantity = "DBZH$";
		REQUIRE_STANDARD_DATA = false;
	};

	///
	double probMax;

	/// Threshold for reflectance Z.
	double dbz;


	/// Fuzzy width // deviation of minDBZ reflectancy.
	double dbzSpan;


protected:

	virtual
	void processData(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const;  // DataSetDst & dst) const;

};



}

#endif /* BIOMET_OP_H_ */
