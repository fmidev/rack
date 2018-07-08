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
#include <drain/util/FunctorPack.h>

#include <drain/imageops/DistanceTransformFillOp.h>
#include <drain/imageops/FastAverageOp.h>
#include <drain/imageops/FunctorOp.h>
#include <drain/imageops/HighPassOp.h>
#include <drain/imageops/MarginalStatisticOp.h>
#include <drain/imageops/RunLengthOp.h>

// debugging
#include <drain/image/File.h>

//#include "odim/ODIM.h"
#include "NoiseOp.h"


// using namespace std;

using namespace drain;
using namespace drain::image;

namespace rack {

/*  Essentially, difference of images filtered with median in two directions.
 *
 *
 */
//void NoiseOp::filterImage(const PolarODIM &srcData.odim, const Image &src, Image &dst) const {
void NoiseOp::processData(const PlainData<PolarSrc> &srcData, PlainData<PolarDst> &dstData) const {

	//drainage em.png --median 10,1,0.25 -o blurH.png
	//drainage em.png --median 1,10,0.75    blurH.png --sub 2 -o sub.png

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << "start" << mout.endl;

	// new

	if ((sensitivity <= 0.0) || (sensitivity > 1.0))
		mout.warn() << "sensitivity should be between 0.0 and 1.0" << mout.endl;

	const double s = 3.0 * (1.0 - sensitivity); // Inversion needed. But 3.0
	drain::FuzzyStepsoid<double> fuzzyStep(s, 0.1*s, 255.0);

	//mout.debug(1) << srcData.odim << mout.endl;

	if (mout.isDebug(10))
		File::write(srcData.data, "andre-noise-0-src.png");

	Image tmp;
	if (mout.isDebug(8))
		tmp.setName("temporary");

	//tmp.setGeometry(1, src.getWidth());  // TODO support for quality field 1xN and Nx1 geometries?

	//SlidingWindowMedianOp median;
	dstData.data.setGeometry(srcData.data.getGeometry());

	/*
	int bin; // histogram bin
	drain::Histogram histogram;
	histogram.setSize(256);
	histogram.setScale(0,255);
    */

	double x;

	const size_t i0 = static_cast<size_t>(1000.0 * (minRange - srcData.odim.rstart) / srcData.odim.rscale);

	/*
	HighPassOp highPass(minLength, 5);
	highPass.offset = 255.0;
	highPass.scale  = 2.0;
	highPass.filter(src, dst);
	if (mout.isDebug(10))
		File::write(dst, "andre-noise-3-highpass.png");
     */
	RunLengthHorzOp rle(1.0);
	rle.process(srcData.data, dstData.data);
	//RemapOp(0, 255).filter(dstData.data, dstData.data);
	UnaryFunctorOp<RemappingFunctor> remap;
	remap.functor.fromValue = 0;
	remap.functor.toValue = 255;
	remap.traverseChannel(dstData.data.getChannel(0), dstData.data.getChannel(0));
	//if (mout.isDebug(10))		File::write(dst, "andre-noise-3-rle.png");
	//FuzzyBellOp(minLength/2, minLength/2+1, 255.0).filter(dstData.data,dstData.data);
	UnaryFunctorOp<FuzzyBell<double> > fuzzyBell;
	fuzzyBell.functor.set(minLength/2, minLength/2+1, 255.0);
	fuzzyBell.traverseChannel(dstData.data.getChannel(0), dstData.data.getChannel(0));
	//FuzzyBellOp(minLength/2, minLength/2+1, 255.0).filter(dstData.data,dstData.data);

	//drainage mili.png --runLengthHorz 1 --remap 0,255 --fuzzyPeak 5,2  -o mili-2-seg.png; display mili-2-seg.png

	if (mout.isDebug(10))
		File::write(dstData.data, "andre-noise-2.png");

	DistanceTransformExponentialOp(minLength, 5).traverseChannel(dstData.data.getChannel(0), dstData.data.getChannel(0));
	if (mout.isDebug(10))
		File::write(dstData.data, "andre-noise-4.png");

	int countFlip;
	int countData;
	int length;

	drain::typeLimiter<int>::value_t limit = dstData.data.getLimiter<int>();

	for (size_t j = 0; j < srcData.data.getHeight(); ++j) {
		countFlip = 0;
		countData = 0;
		length = 0;
		for (size_t i = i0; i < srcData.data.getWidth(); ++i) {
			x = srcData.data.get<double>(i, j);
			if (x != srcData.odim.nodata ){
				++countData;
				if (x != srcData.odim.undetect) {
					++length;
					if (length == minLength)
					  ++countFlip;
					else if (length == 2*minLength)
						--countFlip;
				}
				else {
					length = 0;
				}
			}
			//xx += x;
		}

		//dstData.data.scaling.setLimits(0,250); inside loop?
		/// Maximally countData/(minLength + 1) segments may be found in the image.
		if (countData > 0){
			//  sensitivity tuning not needed?
			int p0;
			int p = sensitivity * (255 * countFlip * (minLength + 1)) / countData;
			for (size_t i = 0; i < dstData.data.getWidth(); ++i){
				p0 = dstData.data.get<double>(i, j);
				if (i < i0)
					//dstData.data.put(i, j, (i*p)/i0);
					dstData.data.put(i, j, p);
				else
					//dstData.data.put(i, j, dstData.data.scaling.limit<int>((p0*p)/255));
					dstData.data.put(i, j, limit((p0*p)/255));
					//dst.put(i, j, ((p0*p)/255));
			}

		}
	}

	/*
	DistanceTransformExponentialOp fill(1, smoothingDeg/360.0 * srcData.odim.nrays);
	fill.filter(tmp,tmp);

	int p;
	for (size_t j = 0; j < dst.getHeight(); ++j) {
		p = tmp.get<int>(j);
		for (size_t i = 0; i < dst.getWidth(); ++i) {
			dst.put(i, j, p);
		}
	}
	 */

	if (mout.isDebug(10))
		File::write(dstData.data,"andre-noise-9-dst.png");


}


}
