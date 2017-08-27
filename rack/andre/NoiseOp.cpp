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


//#include <drain/image/SegmentAreaOp.h>
#include <drain/util/Fuzzy.h>
//#include <drain/image/MathOpPack.h>

#include <drain/image/FastAverageOp.h>
#include <drain/image/MarginalStatisticOp.h>
#include <drain/image/DistanceTransformFillOp.h>
//#include <drain/image/FunctorOp.h>
//#include <drain/image/FuzzyThresholdOp.h>
//#include <drain/image/SlidingWindowMedianOp.h>
//#include "image/GammaOp.h"
#include <drain/image/HighPassOp.h>
#include <drain/image/RunLengthOp.h>

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

	drain::MonitorSource mout(name, __FUNCTION__);
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
	rle.filter(srcData.data, dstData.data);
	//RemapOp(0, 255).filter(dstData.data, dstData.data);
	UnaryFunctorOp<RemappingFunctor> remap;
	remap.functor.fromValue = 0;
	remap.functor.toValue = 255;
	remap.filter(dstData.data, dstData.data);
	//if (mout.isDebug(10))		File::write(dst, "andre-noise-3-rle.png");
	//FuzzyBellOp(minLength/2, minLength/2+1, 255.0).filter(dstData.data,dstData.data);
	UnaryFunctorOp<FuzzyBell<double> > fuzzyBell;
	fuzzyBell.functor.set(minLength/2, minLength/2+1, 255.0);
	fuzzyBell.filter(dstData.data,dstData.data);
	//FuzzyBellOp(minLength/2, minLength/2+1, 255.0).filter(dstData.data,dstData.data);

	//drainage mili.png --runLengthHorz 1 --remap 0,255 --fuzzyPeak 5,2  -o mili-2-seg.png; display mili-2-seg.png

	if (mout.isDebug(10))
		File::write(dstData.data, "andre-noise-2.png");

	DistanceTransformExponentialOp(minLength, 5).filter(dstData.data,dstData.data);
	if (mout.isDebug(10))
		File::write(dstData.data, "andre-noise-4.png");

	int countFlip;
	int countData;
	int length;

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

		dstData.data.setLimits(0,250);
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
					dstData.data.put(i, j, dstData.data.limit<int>((p0*p)/255));
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

// Rack
