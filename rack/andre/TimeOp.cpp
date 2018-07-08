/**

    Copyright 2010-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include <algorithm>

//#include <drain/imageops/SegmentAreaOp.h>
#include <drain/util/Fuzzy.h>
//#include <drain/image/MathOpPack.h>

#include <drain/imageops/FastAverageOp.h>
#include <drain/imageops/MarginalStatisticOp.h>
#include <drain/imageops/DistanceTransformFillOp.h>
//#include <drain/imageops/FuzzyOp.h>
//#include <drain/imageops/FuzzyThresholdOp.h>
//#include <drain/imageops/SlidingWindowMedianOp.h>
//#include "image/GammaOp.h"
#include <drain/imageops/HighPassOp.h>
#include <drain/imageops/RunLengthOp.h>

//#include "radar/ODIM.h"
#include "TimeOp.h"



//#include <drain/image/File.h>

// using namespace std;

using namespace drain::image;

namespace rack {

//void TimeOp::filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const {
void TimeOp::processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const {


	drain::Logger mout(name, __FUNCTION__);
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

	/// Main loop
	//std::cerr << "Mainloop...";
	for (size_t j = 0; j < dstProb.data.getHeight(); ++j) {
		s = secondStart + j*secondsPerBeam;
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
