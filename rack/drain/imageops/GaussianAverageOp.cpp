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


#include <drain/image/ImageFile.h>
#include "GaussianAverageOp.h"


namespace drain
{

namespace image
{

GaussianAverageOp::GaussianAverageOp(int width, int height, double radius) :
		WindowOp<Window<GaussianWindowConf> >(__FUNCTION__, "Gaussian blur implemented as quick Wx1 and 1xH filtering.") {

	this->conf.frame.set(width, height);
	parameters.link("radius", this->conf.radius = radius, "distance, relative to width and height, where gaussian kernel obtains value 0.5.");

}


void GaussianAverageOp::traverseChannel(const Channel & src, Channel & dst) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	// TODO: generalize!
	mout.special() << "conf: " << conf.frame << ',' << conf.radius << mout.endl;

	Image tmp(dst.getType());
	makeCompatible(src.getConf(), tmp);
	//makeCompatible(src, dst); // unneeded in traverse?

	mout.special() << "src: "  << src << mout.endl;
	mout.special() << "tmp: "  << tmp << mout.endl;


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

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	Image tmp(dst.getType());
	Image tmpWeight(dstWeight.getType());
	makeCompatible(src.getConf(), tmp);
	makeCompatible(srcWeight.getConf(), tmpWeight);

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

// Drain
