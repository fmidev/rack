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


#include "image/File.h"

#include "GaussianAverageOp.h"


namespace drain
{

namespace image
{

GaussianAverageOp::GaussianAverageOp(int width, int height, double radius) :
		WindowOp<Window<GaussianWindowConf> >(__FUNCTION__, "Gaussian blur implemented as quick Wx1 and 1xH filtering.") {

	this->conf.width = width;
	this->conf.height = height;
	parameters.reference("radius", this->conf.radius = radius, "distance, relative to width and height, where gaussian kernel obtains value 0.5.");

}


void GaussianAverageOp::traverseChannel(const Channel & src, Channel & dst) const {

	Logger mout(getImgLog(), name, __FUNCTION__);

	// TODO: generalize!

	Image tmp(dst.getType());
	makeCompatible(src, tmp);
	//makeCompatible(src, dst); // unneeded in traverse?

	//GaussianStripe<true> window1(conf.width, 0.5*conf.radius*static_cast<double>(conf.width));
	GaussianStripe<true> window1(conf.getWidth(), conf.radius);
	window1.setSrcFrame(src);
	window1.setDstFrame(tmp);
	mout.debug() << window1 << mout.endl;
	window1.run();

	//const int h = (conf.height>0.0) ? conf.height : conf.width;
	//GaussianStripeVert window2(h, 0.5*conf.radius*static_cast<double>(h));
	GaussianStripe<false> window2(conf.getHeight(), conf.radius);
	window2.setSrcFrame(tmp);
	window2.setDstFrame(dst);
	mout.debug() << window2 << mout.endl;
	window2.run();

	// mout.warn() << "could not set scaling now" << mout.endl;
	// dst.getScaling().setScale(src.getScaling().getScale());

}

//void GaussianAverageOp::process(const ImageFrame & src, const ImageFrame & srcWeight, Image & dst, Image & dstWeight) const {
void GaussianAverageOp::traverseChannel(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), name, __FUNCTION__);

	Image tmp(dst.getType());
	Image tmpWeight(dstWeight.getType());
	makeCompatible(src, tmp);
	makeCompatible(srcWeight, tmpWeight);

	//
	GaussianStripeWeighted<true> window1(conf.getWidth(), conf.radius);
	window1.setSrcFrame(src);
	window1.setSrcFrameWeight(srcWeight);
	window1.setDstFrame(tmp);
	window1.setDstFrameWeight(tmpWeight);
	mout.debug() << window1 << mout.endl;
	window1.run();
	// File::write(tmp, "gauss1d.png");
	// File::write(tmpWeight, "gauss1w.png");

	//const int h = (conf.height>0.0) ? conf.height : conf.width;
	//GaussianStripeWeighted<false> window2(h, 0.5*conf.radius*static_cast<double>(h));
	GaussianStripeWeighted<false> window2(conf.getHeight(), conf.radius);
	window2.setSrcFrame(tmp);
	window2.setSrcFrameWeight(tmpWeight);
	window2.setDstFrame(dst);
	window2.setDstFrameWeight(dstWeight);
	mout.debug() << window2 << mout.endl;
	window2.run();

	// mout.warn() << "could not set scaling now" << mout.endl;
	// dst.scaling.setScale(src.scaling.getScale());

}


}

}
