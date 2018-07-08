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
#include "util/Log.h"
#include "ResizeOp.h"

namespace drain {

namespace image {

void ResizeOp::makeCompatible(const ImageFrame & src, Image & dst) const {
	const size_t w = this->width  ? this->width  : dst.getWidth();
	const size_t h = this->height ? this->height : dst.getHeight();
	dst.initialize(src.getType(), w, h, src.getImageChannelCount(), src.getAlphaChannelCount());
}

void ResizeOp::traverseChannel(const Channel & src, Channel & dst) const {

	drain::Logger mout(getImgLog(), name, __FUNCTION__);

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
