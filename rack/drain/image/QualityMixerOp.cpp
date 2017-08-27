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


#include "QualityMixerOp.h"

namespace drain
{

namespace image
{

void QualityMixerOp::filter(const Image &src, const Image &srcWeight, const Image &src2, const Image &src2Weight, Image &dst, Image &dstWeight) const {

	drain::MonitorSource mout(name, __FUNCTION__);

	makeCompatible(src,dst);
	makeCompatible(srcWeight,dstWeight);

	if (mout.isDebug(4)){
		std::cerr << "QualityMixerOp::filter\n";
		src.info(std::cerr);
		srcWeight.info(std::cerr);
		dst.info(std::cerr);
		dstWeight.info(std::cerr);
	}

	// const size_t dstAlphaChannelCount = std::max(srcWeight.getAlphaChannelCount(), dstWeight.getAlphaChannelCount());

	Image dstWeightTmp;
	//Image & dstWeightRef = (dstWeight.getAlphaChannelCount() == dstAlphaChannelCount) ?

	for (size_t k=0; k<src.getChannelCount(); k++){

		// weight
		float wS, wD;
		Image::const_iterator s    = src.getChannel(k).begin();
		Image::const_iterator sEnd = src.getChannel(k).end();
		Image::const_iterator sw = srcWeight.begin();
		Image::iterator d = dst.getChannel(k).begin();
		Image::iterator dw = dstWeight.begin();
		while (s != sEnd){

			wS = weight       * static_cast<float>(*sw);
			wD = (1.0-weight) * static_cast<float>(*dw);

			*d  = wS* static_cast<float>(*s) + wD* static_cast<float>(*d);
			*dw = wS + wD;

			s++;
			sw++;
			d++;
			dw++;
		}
	}
}


}

}

// Drain
