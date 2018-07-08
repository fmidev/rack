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
#include "util/FunctorPack.h"
#include "image/Image.h"
#include "CatenatorOp.h"

namespace drain
{

namespace image
{

void ChannelCatenatorOp::makeCompatible(const ImageFrame &src, Image &dst) const {
	dst.initialize(src.getType(), src.getWidth(), src.getHeight()*src.getChannelCount(), 1);
}

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

