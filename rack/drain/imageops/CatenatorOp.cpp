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
#include "drain/util/FunctorPack.h"
#include "drain/image/Image.h"
#include "CatenatorOp.h"

namespace drain
{

namespace image
{

/*
void ChannelCatenatorOp::makeCompatible(const ImageFrame &src, Image &dst) const {
	dst.initialize(src.getType(), src.getWidth(), src.getHeight()*src.getChannelCount(), 1);
}
*/

void ChannelCatenatorOp::process(const ImageFrame &src,Image &dst) const {

	dst.copyDeep(src);
	dst.setGeometry(src.getWidth(), src.getHeight()*src.getChannelCount());

	/*
	typedef drain::typeLimiter<double> LimiterD;
	LimiterD::value_t limiter = Type::call<LimiterD>(dst.getType());

	const int width  = dst.getWidth()>0  ? dst.getWidth()  : src.getWidth();
	const int height = dst.getHeight()>0 ? dst.getHeight() : src.getHeight();
	const int channelsSrc = src.getChannelCount();
	const int channelsDst = dst.getChannelCount();

	typedef drain::typeLimiter<double> LimiterD;
	LimiterD::value_t limiter = Type::call<LimiterD>(dst.getType());

	dst.setGeometry(width, height, channelsDst + channelsSrc);
	const int w = std::min(width, (int)src.getWidth());

	Point2D<int> p;
	for (int k = 0; k < channelsSrc; ++k) {
		const Channel & channelSrc = src.getChannel(k);
		Channel & channelDst = dst.getChannel(channelsDst + k);
		for (p.y = 0; p.y < height; ++p.y) {
			for (p.x = 0; p.x < w; ++p.x) {
				//channelDst.put(p, dst.scaling.limit<float>(scale * channelSrc.get<double>(p) + offset));
				channelDst.put(p, limiter(scale * channelSrc.get<double>(p) + offset));
			}
		}
	}
	*/

}


void VerticalCatenatorOp::process(const ImageFrame &src, Image &dst) const {


	const int width       = dst.getWidth()>0 ? dst.getWidth() : src.getWidth();
	const int heightDst   = dst.getHeight();
	const int channelsDst = dst.getChannelCount();

	const int heightSrc   = src.getHeight();
	const int channelsSrc = src.getChannelCount();

	const int start = width*heightDst*channelsDst;

	dst.setGeometry(width, heightDst*channelsDst + heightSrc*channelsSrc, 1, 0);

	//const double scale   = getParameter("scale",1.0);
	//const double bias = getParameter("bias",0.0);

	typedef drain::typeLimiter<double> LimiterD;
	LimiterD::value_t limiter = Type::call<LimiterD>(dst.getType());

	for (int k = 0; k < channelsSrc; ++k) {
		const int w = std::min(width,static_cast<int>(src.getWidth()));
		for (int j=0; j<heightSrc; j++) {
			for (int i=0; i<w; i++) {
				//dst.put(start+src.address(i,j,k), dst.scaling.limit<double>( scale*src.get<double>(i,j) + offset) );
				dst.put(start+src.address(i,j,k), limiter( scale*src.get<double>(i,j) + offset) );
				//dst.limit<double>( (src.get<double>(i,j)-bias)/scale ) );
			}
		}
	}

}


}

}


// Drain
