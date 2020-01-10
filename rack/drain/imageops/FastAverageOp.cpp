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


#include "image/FilePng.h"
#include "FastAverageOp.h"



namespace drain
{

namespace image
{


void FastAverageOp::traverseChannel(const Channel & src, Channel &dst) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	if (dst.isEmpty()){
		mout.error() << "dst empty: " << dst << mout.endl;
		return;
	}

	/// Special case
	if ((conf.getWidth()==1) && (conf.getHeight()==1)){
		mout.info() << "Special case: 1x1 window => simple copy" << mout.endl;
		dst.copyData(src);
		return;
	}


	Image tmp(dst.getType());
	makeCompatible(src,tmp);
	mout.debug(3) << "src: " << src << mout.endl;
	mout.debug(3) << "tmp: " << tmp << mout.endl;
	mout.debug(3) << "dst: " << dst << mout.endl;

	// Accelerate computation if (1 x H) or (W x 1) window.
	if (conf.getWidth() > 1){
		SlidingStripeAverage<SlidingStripe<WindowConfig, WindowCore, true> > window1(conf.getWidth()); // <WindowConfig>
		window1.setSrcFrame(src);
		window1.setDstFrame(tmp);
		mout.debug(1) << window1 << mout.endl;
		window1.run();
	}
	else {
		mout.info() << "Special case optimized: 1xH window" << mout.endl;
		tmp.copyDeep(src);
	}

	if (conf.getHeight() > 1){
		SlidingStripeAverage<SlidingStripe<WindowConfig, WindowCore, false> > window2(conf.getHeight()); // <WindowConfig>
		window2.setSrcFrame(tmp);
		window2.setDstFrame(dst);
		mout.debug(1) << window2 << mout.endl;
		window2.run();
	}
	else {
		mout.info() << "Special case optimized: Wx1 window" << mout.endl;
		dst.copyData(tmp);
	}

	//dst.scaling.set(src.scaling); //
}

/// Calls SlidingStripeOp<T,T2>::process() separately for each image channel. This is natural for many operations, such as averaging.


// Raise filterUnweighted
void FastAverageOp::traverseChannel(const Channel & src, const Channel & srcAlpha, Channel & dst, Channel & dstAlpha) const {

	Logger mout(__FUNCTION__, __FILE__); //REPL getImgLog(), name, "process(src,srcW,dst,dstW)");
	//mout.warn() <<  src      << mout.endl;
	//mout.warn() << srcAlpha << mout.endl;

	Image tmp; //(src);
	Image tmpAlpha; //(srcAlpha);

	makeCompatible(src, tmp);
	makeCompatible(srcAlpha, tmpAlpha);
	//mout.warn() << "tmp:      " << tmp      << mout.endl;
	//mout.warn() << "tmpAlpha: " << tmpAlpha << mout.endl;

	// TODO:Generalize to SeparableWindowOp,
	SlidingStripeAverageWeighted<SlidingStripe<WindowConfig, WeightedWindowCore, true> > window1(conf.getWidth());
	window1.setSrcFrame(src);
	window1.setSrcFrameWeight(srcAlpha);
	window1.setDstFrame(tmp);
	window1.setDstFrameWeight(tmpAlpha);
	window1.run();
	//FilePng::write(tmp, name+"-D.png");
	// FilePng::write(tmpAlpha, name+"-Q.png");

	SlidingStripeAverageWeighted<SlidingStripe<WindowConfig, WeightedWindowCore, false> > window2(conf.getHeight());
	window2.setSrcFrame(tmp);
	window2.setSrcFrameWeight(tmpAlpha);
	window2.setDstFrame(dst);
	window2.setDstFrameWeight(dstAlpha);
	window2.run();

	//mout.warn() << "check scaling of the result..." << src.getScaling() << " <=> " << dst.getScaling() << mout.endl;
	//dst.setScaling(src.getScaling()); // ??

}

}

}

// Drain
