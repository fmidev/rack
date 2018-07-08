/**

    Copyright 2013-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#ifndef Time_OP_H_
#define Time_OP_H_

#include <drain/util/Time.h>
#include <drain/image/File.h>
#include "DetectorOp.h"

using namespace drain::image;

namespace rack {

/// A radio interference detector.

class TimeOp: public DetectorOp {

public:

	/** Default constructor.
	 *   \param time - reference instant of the quality, format "%Y%m%d%H%M"; if empty, nominal scan time is used.
	 *   \param decayPast   - quality halving time in minutes
	 *   \param decayFuture - quality halving time in minutes; if not set, decayPast is used.
	 */
	TimeOp(std::string time="NOMINAL", double decayPast = 1.0, double decayFuture = -1.0) :
		DetectorOp(__FUNCTION__,"Created quality field based on measurement time for each beam.", ECHO_CLASS_DELAY)
	{
		parameters.reference("time",  this->time = time, "NOMINAL,NOW,<YYYYmmddMMHH>");
		parameters.reference("decayPast",  this->decayPast = decayPast, "mins");
		parameters.reference("decayFuture",  this->decayFuture =  decayFuture, "mins");
		UNIVERSAL = true;
		REQUIRE_STANDARD_DATA = false;


		dataSelector.quantity = "^[A-Z]+$";  // Any quantity is ok...
		dataSelector.path = ".*/data1/?$"; // ... but use the first only.

	}

	virtual
	~TimeOp(){}

	std::string time;
	double decayPast;
	double decayFuture;


protected:

	virtual
	void processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const;
	//void filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const;

};

}

#endif
