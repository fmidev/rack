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

#ifndef BioMetOP2_H_
#define BioMetOP2_H_

#include "DetectorOp.h"

//#include <drain/image/SegmentAreaOp.h>
#include <drain/util/Fuzzy.h>
//#include <drain/image/MathOpPack.h>
//#include <drain/image/File.h>


using namespace drain::image;



namespace rack {

/// Detects birds and insects.
/**
 *
 *
 */
class BiometOp: public DetectorOp {

public:

	/**
	 * \param reflMax - maximum expected reflectance of biometeors
	 * \param maxAltitude - maximum expected altitude of biometeors
	 * \param reflDev - fuzzy width of  \c reflMax
	 * \param devAltitude - fuzzy width of \c maxAltitude
	 */
	BiometOp(double reflMax=-10.0, int maxAltitude=500, double reflDev=5.0, int devAltitude=1000) :
		DetectorOp(__FUNCTION__, "Detects birds and insects.", ECHO_CLASS_INSECT){

		parameters.reference("reflMax", this->reflMax = reflMax, "dBZ");
		parameters.reference("maxAltitude", this->maxAltitude = maxAltitude, "m");
		parameters.reference("reflDev", this->reflDev = reflDev, "dBZ");
		parameters.reference("devAltitude", this->devAltitude = devAltitude, "m");
		dataSelector.quantity = "DBZH$";
		REQUIRE_STANDARD_DATA = false;
	};

	/// Threshold for reflectance Z.
	double reflMax;

	/// Fuzzy deviation of minDBZ reflectancy.
	double reflDev;

	/// Maximum height of biometeors.
	int maxAltitude;

	/// Fuzzy deviation of maximum height of biometeors.
	int devAltitude;

protected:

	virtual
	void processData(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const;  // DataSetDst & dst) const;

};



}

#endif /* BIOMET_OP_H_ */
