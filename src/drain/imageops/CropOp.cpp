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
#include "CropOp.h"

namespace drain {

namespace image {

void CropOp::getDstConf(const ImageConf & src, ImageConf & dst) const {
//void CropOp::makeCompatible(const ImageFrame & src, Image & dst) const {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
	// unneeded here: if (!dst.typeIsSet()) dst.setType(src.getType());

	dst.setGeometry(width, height, src.getImageChannelCount(), src.getAlphaChannelCount());

	mout.special("dst: " , dst );

}


void CropOp::traverseChannel(const Channel & src, Channel & dst) const {

	//Image srcView;

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.debug(parameters );

	//makeCompatible(src, dst);
	const size_t channels = std::min(src.getChannelCount(), dst.getChannelCount());

	if (channels > 1)
		mout.warn("channels: " ,channels );

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

// Drain
