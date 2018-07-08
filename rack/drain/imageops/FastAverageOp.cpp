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


#include "image/FilePng.h"
#include "FastAverageOp.h"



namespace drain
{

namespace image
{


void FastAverageOp::traverseChannel(const Channel & src, Channel &dst) const {

	Logger mout(getImgLog(), name, __FUNCTION__);

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

	Logger mout(getImgLog(), name, "process(src,srcW,dst,dstW)");
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
