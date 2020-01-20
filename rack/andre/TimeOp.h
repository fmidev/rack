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
	TimeOp(const std::string & time="NOMINAL", double decayPast = 1.0, double decayFuture = -1.0) :
		DetectorOp(__FUNCTION__,"Created quality field based on measurement time for each beam.", "tech.err.time")
	{
		parameters.reference("time",  this->time = time, "NOMINAL,NOW,<YYYYmmddMMHH>");
		parameters.reference("decayPast",  this->decayPast = decayPast, "mins");
		parameters.reference("decayFuture",  this->decayFuture =  decayFuture, "mins");
		UNIVERSAL = true;
		REQUIRE_STANDARD_DATA = false;

		dataSelector.quantity = "^[A-Z]+$";  // Any quantity is ok...

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

// Rack
