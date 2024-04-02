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
#include <drain/Log.h>
#include "MultiThresholdOp.h"

namespace drain {

namespace image {




/// Apply to single channel.
void MultiThresholdOp::traverseChannel(const Channel & src, Channel &dst) const {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	const drain::ValueScaling & srcScale = src.getScaling();
	const drain::ValueScaling & dstScale = dst.getScaling();

	Image::const_iterator  sit = src.begin();
	Image::iterator dit  = dst.begin();

	drain::Range<double> targetScaled;
	targetScaled.min = dstScale.inv(target.min);
	targetScaled.max = dstScale.inv(target.max);

	mout.warn("targetScaled: " , targetScaled );

	double s;
	while (true){

		if (sit == src.end())
			return;

		s = srcScale.fwd(*sit);
		if (s < range.min){
			*dit  = targetScaled.min;
		}
		else if (s > range.max){
			*dit  = targetScaled.max;
		}
		else {
			*dit = dstScale.inv(s);
		}


		++sit;
		++dit;

	}


};

/// Apply to single channel with alpha.
void MultiThresholdOp::traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {

	const drain::ValueScaling & srcScale      = src.getScaling();
	const drain::ValueScaling & srcAlphaScale = srcAlpha.getScaling();
	const drain::ValueScaling & dstScale      = dst.getScaling();
	const drain::ValueScaling & dstAlphaScale = dstAlpha.getScaling();

	Image::const_iterator  sit = src.begin();
	Image::const_iterator sait = srcAlpha.begin();
	Image::iterator dit  = dst.begin();
	Image::iterator dait = dstAlpha.begin();

	drain::Range<double> targetScaled;
	targetScaled.min = dstScale.inv(target.min);
	targetScaled.max = dstScale.inv(target.max);

	const double alphaMin = dstAlphaScale.inv(0.0); // or 0?

			double s;
	while (true){

		if (sit == src.end())
			return;

		s = srcScale.fwd(*sit);
		if (s < range.min){
			*dit  = targetScaled.min;
			*dait = alphaMin;
		}
		else if (s > range.max){
			*dit  = targetScaled.max;
			*dait = alphaMin;
		}
		else {
			*dit  = dstScale.inv(s);
			*dait = dstAlphaScale.inv(srcAlphaScale.fwd(*sait));
		}

		++sit; ++sait;
		++dit; ++dait;

	}

};




}  // namespace image

}  // namespace drain

// Drain
