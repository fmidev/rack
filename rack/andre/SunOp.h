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

#ifndef SunOP2_H_
#define SunOP2_H_

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
class SunOp: public DetectorOp {

public:

	/**
	 * \param reflMax - maximum expected reflectance of Suneors
	 * \param maxAltitude - maximum expected altitude of Suneors
	 * \param reflDev - fuzzy width of  \c reflMax
	 * \param devAltitude - fuzzy width of \c maxAltitude
	 */
	SunOp(double width = 1.0, double sensitivity=0.5) :
		DetectorOp("Sun","Draw the sun beam", ECHO_CLASS_SUN){
		parameters.reference("width", this->width = width, "deg");
		parameters.reference("sensitivity", this->sensitivity = sensitivity, "0...1");
		dataSelector.quantity = "^DBZH$";
	};

	/// Threshold for reflectance Z.
	double width;

	/// Intensity in which sector marked.
	double sensitivity;

protected:

	virtual
	void processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const;
	//void filterImage(const PolarODIM &odim, const Image &src, Image &dst) const;

};



}

#endif /* Sun_OP_H_ */
