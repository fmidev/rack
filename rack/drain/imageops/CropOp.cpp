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
#include "CropOp.h"

namespace drain {

namespace image {


void CropOp::makeCompatible(const ImageFrame & src, Image & dst) const {

	drain::Logger mout(getImgLog(), name, __FUNCTION__);

	if (!dst.typeIsSet())
		dst.setType(src.getType());

	dst.setGeometry(width, height, src.getImageChannelCount(), src.getAlphaChannelCount());

	mout.debug(1) << "dst: " << dst << mout.endl;

}


void CropOp::traverseChannel(const Channel & src, Channel & dst) const {

	//Image srcView;

	drain::Logger mout(getImgLog(), name, __FUNCTION__);

	mout.debug() << parameters << mout.endl;

	//makeCompatible(src, dst);
	const size_t channels = std::min(src.getChannelCount(), dst.getChannelCount());

	if (channels > 1)
		mout.warn() << "channels: " <<channels << mout.endl;

	CoordinateHandler2D handler(src.getWidth(), src.getHeight(), src.getCoordinatePolicy());

	Point2D<int> p;
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			p.setLocation(i0+i, j0+j);
			handler.handle(p);
			dst.put(i,j, src.get<double>(p) );
			//for (size_t k = 0; k < channels; ++k)
			//	dst.put(i, j, k, src.get<double>(p.x, p.y, k) );
		}
	}


}

}  // namespace image

}  // namespace drain
