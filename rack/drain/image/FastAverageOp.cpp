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

#include "FastAverageOp.h"
#include "FilePng.h"


namespace drain
{

namespace image
{


void FastAverageOp::traverse(const Image &src,Image &dst) const {

	MonitorSource mout(iMonitor, name, __FUNCTION__);
	// 	mout.warn() << *this << mout.endl;

	Image tmp;
	makeCompatible(src,tmp);
	//makeCompatible(src,dst); // already done

	SlidingStripeAverage<SlidingStripeHorz<WindowConfig> > window1(width);
	window1.setSrc(src);
	window1.setDst(tmp);
	mout.debug() << window1 << mout.endl;
	window1.slide();

	SlidingStripeAverage<SlidingStripeVert<WindowConfig> > window2((height>0) ? height : width);
	window2.setSrc(tmp);
	window2.setDst(dst);
	mout.debug() << window2 << mout.endl;
	window2.slide();

	dst.setScale(src.getScale());
}

/// Calls SlidingStripeOp<T,T2>::filter() separately for each image channel. This is natural for many operations, such as averaging.


// Raise filterUnweighted
void FastAverageOp::traverse(const Image &src, const Image &srcAlpha,  Image &dst, Image &dstAlpha) const {

	MonitorSource mout(iMonitor, name, "traverse(src,srcW,dst,dstW)");

	//mout.warn() << src      << mout.endl;
	//mout.warn() << srcAlpha << mout.endl;

	Image tmp(src);
	Image tmpAlpha(srcAlpha);

	//makeCompatible(src,dst);
	//makeCompatible(srcAlpha,dstAlpha);
	makeCompatible(src, tmp);
	makeCompatible(srcAlpha, tmpAlpha);
	makeCompatible(src,dst);
	makeCompatible(srcAlpha,dstAlpha);
	//FilePng::write(src, name+"-d.png");
	//FilePng::write(srcAlpha, name+"-q.png");


	SlidingStripeAverageWeighted<SlidingStripeHorz<WindowConfig> > window1(width);
	window1.setSrc(src);
	window1.setSrcWeight(srcAlpha);
	window1.setDst(tmp);
	window1.setDstWeight(tmpAlpha);
	window1.slide();
	//FilePng::write(tmp, name+"-D.png");
	//FilePng::write(tmpAlpha, name+"-Q.png");

	SlidingStripeAverageWeighted<SlidingStripeVert<WindowConfig> > window2((height>0) ? height : width);
	window2.setSrc(tmp);
	window2.setSrcWeight(tmpAlpha);
	window2.setDst(dst);
	window2.setDstWeight(dstAlpha);
	window2.slide();

	dst.setScale(src.getScale()); // ??

}

}

}

// Drain
