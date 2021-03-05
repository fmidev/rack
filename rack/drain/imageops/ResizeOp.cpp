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
#include "drain/util/Log.h"
#include "ResizeOp.h"

namespace drain {

namespace image {

void ResizeOp::getDstConf(const ImageConf &src, ImageConf & dst) const {
//void ResizeOp::makeCompatible(const ImageFrame & src, Image & dst) const {
	const size_t w = this->width  ? this->width  : dst.getWidth();
	const size_t h = this->height ? this->height : dst.getHeight();
	dst.setArea(w, h);
	dst.setChannelCount(src.channels);
	//dst.initialize(src.getType(), w, h, src.getImageChannelCount(), src.getAlphaChannelCount());
}

void ResizeOp::traverseChannel(const Channel & src, Channel & dst) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	const int widthSrc  = src.getWidth();
	const int heightSrc = src.getHeight();
	//const size_t channels  = src.getChannelCount();
	const size_t widthDst  = dst.getWidth();
	const size_t heightDst = dst.getHeight();

	//CoordinateHandler2D handler1(width, height, src.getCoordinatePolicy());
	if (this->interpolation.empty()){
		mout.error() << "interpolation method unset" << mout.endl;
		return;
	}
	const char intMethod = interpolation[0];

	const Point2D<double> aspect(static_cast<double>(widthSrc)/static_cast<double>(widthDst), static_cast<double>(heightSrc)/static_cast<double>(heightDst));
	mout.debug() << "interpolation: " << intMethod << mout.endl;
	mout.debug() << "aspect ratio: " << aspect << mout.endl;


	// Real valued coordinates in the source image
	//Point2D<double> pSrc;
	Point2D<double> pSrcLo;
	Point2D<double> pSrcHi;

	Point2D<double> coeffLo;
	Point2D<double> coeffHi;
	Point2D<int> pLo;
	Point2D<int> pHi;

	double x,y, f;
	int i2, j2;
	// for (size_t k = 0; k < channels; ++k) {
	switch (intMethod) {
	case 'n':
		for (size_t i = 0; i < widthDst; ++i) {
			i2 = (i*widthSrc)/widthDst;
			for (size_t j = 0; j < heightDst; ++j) {
				j2 = (j*heightSrc)/heightDst;
				dst.put(i,j, scale * src.get<double>(i2,j2));
			}
		}
		break;
	case 'b':
		for (size_t i = 0; i < widthDst; ++i) {
			//pSrc.x   = aspect.x * static_cast<double>(i);
			x = aspect.x * static_cast<double>(i);
			f = floor(x);
			pLo.x = static_cast<int>(f);
			coeffHi.x = x-f;
			coeffLo.x = 1.0-coeffHi.x;

			f = floor(x + aspect.x); // <=> i+1
			pHi.x = std::min(widthSrc-1, static_cast<int>(f));

			for (size_t j = 0; j < heightDst; ++j) {

				y = aspect.y * static_cast<double>(j);
				f = floor(y);
				pLo.y =  static_cast<int>(f);
				coeffHi.y = y-f;
				coeffLo.y = 1.0-coeffHi.y;

				f = floor(y + aspect.y); // <=> j+1
				pHi.y = std::min(heightSrc-1, static_cast<int>(f));

				dst.put(i, j, scale * (
						coeffLo.y*(coeffLo.x*src.get<double>(pLo.x, pLo.y) + coeffHi.x*src.get<double>(pHi.x, pLo.y)) +
						coeffHi.y*(coeffLo.x*src.get<double>(pLo.x, pHi.y) + coeffHi.x*src.get<double>(pHi.x, pHi.y)) ) );
				//getRoundedCoords(pSrc, pLo, pHi);
			}
		}
		break;
	default:
		mout.error() << "unknown interpolation method: " << interpolation << mout.endl;
	}

	//	}
}


}  // namespace image

}  // namespace drain

// Drain
