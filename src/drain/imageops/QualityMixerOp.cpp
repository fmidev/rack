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


void QualityMixerOp::traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.note() << "src: " << src.getGeometry() << ", dst: "  << dst.getGeometry() << mout.endl;

	if (src.getGeometry() != dst.getGeometry()){
		mout.error() << "geometries not same" << mout.endl;
	}

	// assume 1 alpha channel!
	const Channel & srcW = src.getAlpha();
	Channel & dstW = dst.getAlpha();

	const size_t width  = dstW.getWidth();
	const size_t height = dstW.getHeight();
	size_t address;
	float w1, w2, wSumInv;
	const float weight1 = coeff;
	const float weight2 = 1.0f - coeff;
	ImageTray<const Channel>::const_iterator sit;
	ImageTray<Channel>::iterator dit;
	for (size_t i = 0; i < width; ++i) {
		for (size_t j = 0; j < height; ++j) {
			address = dstW.address(i, j);
			w1 = srcW.get<float>(address);
			w2 = dstW.get<float>(address);
			if ((w1>0.0) || (w2>0.0)){
				w1 *= weight1;
				w2 *= weight2;
				wSumInv = 1.0f/(w1+w2);
				sit = src.begin();
				dit = dst.begin();
				while (sit != src.end()){
					dit->second.put(address, wSumInv * (w1*sit->second.get<float>(address) + w2*dit->second.get<float>(address)) );
					++sit;
					++dit;
				}
				dstW.put(address, w1+w2); //
			}
			//}

		}
	}
}



/*
void QualityMixerOp::process(const ImageFrame &src, const ImageFrame &srcWeight, const ImageFrame &src2, const ImageFrame &src2Weight, Image &dst, Image &dstWeight) const {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	makeCompatible(src,dst);
	makeCompatible(srcWeight,dstWeight);

	if (mout.isDebug(4)){
		std::cerr << "QualityMixerOp::filter\n";
		src.toOStr(std::cerr);
		srcWeight.toOStr(std::cerr);
		dst.toOStr(std::cerr);
		dstWeight.toOStr(std::cerr);
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
*/

}

}

// Drain
