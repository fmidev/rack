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

#include <algorithm>

//#include "drain/imageops/SegmentAreaOp.h"
#include "drain/util/Fuzzy.h"
//#include "drain/image/MathOpPack.h"

#include "drain/imageops/FastAverageOp.h"
#include "drain/imageops/MarginalStatisticOp.h"
#include "drain/imageops/DistanceTransformFillOp.h"
//#include "drain/imageops/FuzzyOp.h"
//#include "drain/imageops/FuzzyThresholdOp.h"
//#include "drain/imageops/SlidingWindowMedianOp.h"
//#include "drain/image/GammaOp.h"
#include "drain/imageops/HighPassOp.h"
#include "drain/imageops/RunLengthOp.h"

//#include "radar/ODIM.h"
#include "TimeOp.h"



//#include "drain/image/File.h"

// using namespace std;

using namespace drain::image;

namespace rack {

//void TimeOp::filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const {
void TimeOp::processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const {


	drain::Logger mout(__FUNCTION__, __FILE__);
	mout.debug() << "start" << mout.endl;

	drain::Time timeRef;

	if (time == "NOW"){
		const std::string format("%Y%m%d%H%M%S");
		try {
			timeRef.setTime(time, format);
		} catch (std::runtime_error &e) {
			mout.warn() << "warning: parsing failed for time: " << time << ", using format: " << format << mout.endl;
		}
	}
	else if ((time == "NOMINAL") || time.empty()){
		try {
			timeRef.setTime(srcData.odim.date, "%Y%m%d");
			timeRef.setTime(srcData.odim.time, "%H%M%S");
		} catch (std::runtime_error &e) {
			mout.warn() << "warning: parsing failed for scan start time:" <<  srcData.odim.startdate << srcData.odim.starttime << mout.endl;
		}
	}
	/*
	else if ((time == "START")){
		try {
			timeRef.setTime(srcData.odim.startdate, "%Y%m%d");
			timeRef.setTime(srcData.odim.starttime, "%H%M%S");
		} catch (std::runtime_error &e) {
			mout.warn() << "warning: parsing failed for scan start time:" <<  srcData.odim.startdate << srcData.odim.starttime << mout.endl;
		}
	}
	else if ((time == "END")){
		try {
			timeRef.setTime(srcData.odim.enddate, "%Y%m%d");
			timeRef.setTime(srcData.odim.endtime, "%H%M%S");
		} catch (std::runtime_error &e) {
			mout.warn() << "warning: parsing failed for scan start time:" <<  srcData.odim.startdate << srcData.odim.starttime << mout.endl;
		}
	}
	*/
	else {
		try {
			timeRef.setTime(time, "%Y%m%d%H%M%S");
		} catch (std::runtime_error &e) {
			mout.warn() << "warning: parsing failed for user-defined ref time: " << time << ", skipping operation." << mout.endl;
			return;
		}
	}

	mout.debug() << "time" << timeRef.str() << mout.endl;
	mout.info() << srcData.odim.date << ' ' << srcData.odim.time << mout.endl;
	const time_t secondRef = timeRef.getTime();

	const double maxQuality = dstProb.odim.scaleInverse(1.0);

	drain::FuzzyBell<double> fuzzyPast(secondRef, decayPast, maxQuality);
	drain::FuzzyBell<double> fuzzyFuture(secondRef, (decayFuture>=0.0 ? decayFuture :  decayPast), maxQuality);

	drain::Time t;
	t.setTime(srcData.odim.startdate, "%Y%m%d");
	t.setTime(srcData.odim.starttime, "%H%M%S");
	mout.debug() << "startTime: " << t.str() << mout.endl;
	time_t secondStart = t.getTime();

	t.setTime(srcData.odim.enddate, "%Y%m%d");
	t.setTime(srcData.odim.endtime, "%H%M%S");
	mout.debug() << "endTime: " << t.str() << mout.endl;
	time_t secondEnd = t.getTime();

	const double secondsPerBeam = (secondEnd-secondStart)/(double)dstProb.data.getWidth();
	mout.debug() << "secondsPerBeam: " << secondsPerBeam << mout.endl;
	double s, q;

	/// MAIN
	int j2;
	for (size_t j = 0; j < dstProb.data.getHeight(); ++j) {

		if (j < srcData.odim.a1gate)
			j2 = j-srcData.odim.a1gate + dstProb.data.getHeight();
		else
			j2 = j-srcData.odim.a1gate;

		s = secondStart + j2*secondsPerBeam;
		if (s < secondRef )
			q = maxQuality - fuzzyPast(s);
		else
			q = maxQuality - fuzzyFuture(s);
		//mout.debug() << "beam: #" << j << ", q:" << q << mout.endl;
		for (size_t i = 0; i < dstProb.data.getWidth(); ++i) {
			dstProb.data.put(i,j, q);
		}
	}
	//std::cerr << "... end Mainloop." << std::endl;

}


}

// Rack
