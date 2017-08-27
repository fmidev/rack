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

#include "FastAverageOp.h" // move
#include "GaussianBlurOp.h"
#include "File.h"


namespace drain
{

namespace image
{

GaussianBlurOp::GaussianBlurOp(int width, int height, double radius) :
		WindowOpP<GaussianStripe>(__FUNCTION__, "Gaussian blur implemented as quick Wx1 and 1xH filtering.") {

	this->conf.width = width;
	this->conf.height = height;
	parameters.reference("radius", this->conf.radius = radius, "distance, relative to width and height, where gaussian kernel obtains value 0.5.");

}


void GaussianBlurOp::traverse(const Image &src, Image &dst) const {

	MonitorSource mout(iMonitor, name, __FUNCTION__);

	Image tmp;
	makeCompatible(src, tmp);
	makeCompatible(src, dst); // unneeded in traverse?

	//const int w = (conf.width>0.0)? conf.width : 3 * conf.radius;
	//GaussianStripeHorz window1(conf.radius, w);
	GaussianStripeHorz window1(conf.width, 0.5*conf.radius*static_cast<double>(conf.width));
	window1.setSrc(src);
	window1.setDst(tmp);
	mout.debug() << window1 << mout.endl;
	window1.traverse();

	const int h = (conf.height>0.0) ? conf.height : conf.width;
	GaussianStripeVert window2(h, 0.5*conf.radius*static_cast<double>(h));
	window2.setSrc(tmp);
	window2.setDst(dst);
	mout.debug() << window2 << mout.endl;
	window2.traverse();

	dst.setScale(src.getScale());
}

void GaussianBlurOp::traverse(const Image &src, const Image &srcWeight, Image &dst, Image &dstWeight) const {

	MonitorSource mout(iMonitor, name, __FUNCTION__);

	Image tmp;
	Image tmpWeight;
	makeCompatible(src, tmp);
	makeCompatible(srcWeight, tmpWeight);
	makeCompatible(src, dst);
	makeCompatible(srcWeight, dstWeight);

	GaussianStripeHorzWeighted window1(conf.width, 0.5*conf.radius*static_cast<double>(conf.width));
	window1.setSrc(src);
	window1.setSrcWeight(srcWeight);
	window1.setDst(tmp);
	window1.setDstWeight(tmpWeight);
	mout.debug() << window1 << mout.endl;
	window1.traverse();
	// File::write(tmp, "gauss1d.png");
	// File::write(tmpWeight, "gauss1w.png");

	const int h = (conf.height>0.0) ? conf.height : conf.width;
	GaussianStripeVertWeighted window2(h, 0.5*conf.radius*static_cast<double>(h));
	window2.setSrc(tmp);
	window2.setSrcWeight(tmpWeight);
	window2.setDst(dst);
	window2.setDstWeight(dstWeight);
	mout.debug() << window2 << mout.endl;
	window2.traverse();

	dst.setScale(src.getScale());

}


}

}

// Drain
