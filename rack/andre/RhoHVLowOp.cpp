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

#include "RhoHVLowOp.h"

#include <drain/util/Fuzzy.h>

#include <drain/image/File.h>
#include <drain/imageops/SlidingWindowMedianOp.h>

//#include <drain/imageops/SegmentAreaOp.h>
//#include <drain/image/MathOpPack.h>

using namespace drain::image;

namespace rack {

//void RhoHVLowOp::filterImage(const PolarODIM &srcData.odim, const Image &src, Image &dst) const {

void RhoHVLowOp::processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const {


	drain::Logger mout(name, __FUNCTION__);
	//mout.debug() << parameters << mout.endl;

	/// Assumes that at least range 2...253 is intensities (not nodata or undetected)
	//op.setParameter("max", src.getMax<double>()-2.0);

	//drain::FuzzyStepsoid<double, double> f(odimIn.scaleInverse(threshold), odimIn.scaleInverse(threshold + thresholdWidth) - odimIn.scaleInverse(threshold) ); BUG
	const unsigned int QMAX = dstProb.odim.scaleInverse(1.0);
	//drain::FuzzyStepsoid<double, double> fuzzyStep(threshold, -fabs(thresholdWidth), QMAX);
	//drain::FuzzyStep<double> fuzzyStep(threshold+thresholdWidth, threshold-thresholdWidth, QMAX);
	drain::FuzzyStep<double> fuzzyStep(threshold - thresholdWidth, threshold + thresholdWidth, QMAX);
	mout.debug() << "fuzzy step:"  << mout.endl;

	Image::const_iterator it = srcData.data.begin();
	Image::iterator dit = dstProb.data.begin();
	while (it != srcData.data.end()){
		if (*it != srcData.odim.nodata){
			if (*it != srcData.odim.undetect){
				*dit = fuzzyStep(srcData.odim.scaleForward(*it));
				//*dit = 64 + fuzzyStep(srcData.odim.scaleForward(*it))/ 2;
			}
			else {
				*dit = 0.0;
			}
		}
		++it; ++dit;
	}

	/// Morphological closing
	if ((windowWidth>0) && (windowHeight>0)){
		const CoordinatePolicy & coordPolicy = srcData.data.getCoordinatePolicy();
		dstProb.data.setCoordinatePolicy(coordPolicy);

		Image tmp;
		tmp.setCoordinatePolicy(coordPolicy);
		const int w = windowWidth / srcData.odim.rscale;
		const int h = windowHeight * 360.0 / srcData.odim.nrays;
		SlidingWindowMedianOp median;

		median.setSize(w,h);
		median.conf.percentage = medianPos;
		median.traverseChannel(dstProb.data.getChannel(0), dstProb.data.getChannel(0));

		/*
		median.filter(dstProb.data, tmp);
		median.setSize(w*2,h*2);
		median.conf.percentage = 1.0 - medianPos;
		median.filter(tmp, dstProb.data);
		*/
	}

	it = srcData.data.begin();
	dit = dstProb.data.begin();
	while (it != srcData.data.end()){
		if (*it != srcData.odim.nodata){
			if (*it != srcData.odim.undetect){
				*dit = (QMAX - static_cast<int>(*dit));
				//*dit = fuzzyStep(srcData.odim.scaleForward(*it));
				//*dit = 64 + fuzzyStep(srcData.odim.scaleForward(*it))/ 2;
			}
			else {
				*dit = 0.0;
			}
		}
		++it; ++dit;
	}

}


}
