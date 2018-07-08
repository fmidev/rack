/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */


#include "QualityMixerOp.h"

namespace drain
{

namespace image
{


void QualityMixerOp::traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	Logger mout(getImgLog(), name, __FUNCTION__);

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

	drain::Logger mout(name, __FUNCTION__);

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
