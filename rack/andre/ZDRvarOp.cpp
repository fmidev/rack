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

#include "ZDRvarOp.h"

#include "drain/util/Fuzzy.h"

#include "drain/imageops/SlidingWindowMedianOp.h"
#include "drain/image/File.h"
//#include "drain/imageops/SegmentAreaOp.h"
//#include "drain/image/MathOpPack.h"

using namespace drain::image;

namespace rack {


//void ZDRvarOp::filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const {
void ZDRvarOp::processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const {

	drain::Logger mout(__FUNCTION__, __FILE__);
	//mout.debug() << parameters << mout.endl;

	/// Assumes that at least range 2...253 is intensities (not nodata or undetected)
	//op.setParameter("max", srcData.data.getMax<double>()-2.0);

	//drain::FuzzyStepsoid<double, double> f(srcData.odim.scaleInverse(threshold), srcData.odim.scaleInverse(threshold + thresholdWidth) - srcData.odim.scaleInverse(threshold) ); BUG
	drain::FuzzyStepsoid<double> f(threshold, thresholdWidth);
	mout.debug() << "fuzzy step:"  << mout.endl;

	const double maxQuality = dstProb.odim.scaleInverse(1.0);
	//SlidingWindowHistogramOp histogramOp(windowWidth);


	Image::const_iterator it = srcData.data.begin();
	Image::iterator dit = dstProb.data.begin();
	while (it != srcData.data.end()){
		if (*it != srcData.odim.nodata){
			if (*it != srcData.odim.undetect){
				*dit = maxQuality * (1.0-f(srcData.odim.scaleForward(*it)));
			}
			else {
				*dit = 0.0;
			}
		}
		++it; ++dit;
	}

	/// Morphological closing
	if ((windowWidth>0) && (windowHeight>0)){
		Image tmp;
		const int w = windowWidth / srcData.odim.rscale;
		const int h = windowHeight * 360.0 / srcData.odim.area.height;
		SlidingWindowMedianOp median;

		median.setSize(w,h);
		median.conf.percentage = medianPos;
		median.process(dstProb.data, tmp);

		median.setSize(w*2,h*2);
		median.conf.percentage = 1.0 - medianPos;
		median.process(tmp, dstProb.data);
	}
}


}

// Rack
