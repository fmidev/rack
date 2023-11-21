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


#include "drain/image/FilePng.h"
#include "FastAverageOp.h"



namespace drain
{

namespace image
{

void FastAverageOp::getDstConf(const ImageConf & src, ImageConf & dst) const {

	//if ((dst.getScale()==0.0) || !dst.typeIsSet())
	dst.setEncoding(src.getEncoding());

	dst.setGeometry(src.getGeometry()); //geometry = src.geometry;

	dst.setCoordinatePolicy(src.getCoordinatePolicy());

}

void FastAverageOp::traverseChannel(const Channel & src, Channel &dst) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	if (dst.isEmpty()){
		mout.error("dst empty: " , dst );
		return;
	}

	size_t width  = conf.getWidth();
	size_t height = conf.getHeight() ? conf.getHeight() : width;

	/// Special case
	if ((width==1) && (height==1)){
		mout.note("Special case: 1x1 window => simple copy" );
		dst.copyData(src);
		return;
	}

	mout.special(width , 'x' , height );


	Image tmp;
	tmp.setConf(src.getConf());
	//makeCompatible(src.getConf(), tmp);
	mout.debug("src: " , src );
	mout.special("tmp: " , tmp );
	mout.special("dst: " , dst );

	// Accelerate computation if (1 x H) or (W x 1) window.
	if (width > 1){
		SlidingStripeAverage<SlidingStripe<WindowConfig, WindowCore, true> > window1(width); // <WindowConfig>
		window1.setSrcFrame(src);
		window1.setDstFrame(tmp);
		mout.debug2() << window1 << mout.endl;
		window1.run();
		mout.special(window1 );
	}
	else {
		mout.info("Special case optimized: 1xH window" );
		tmp.copyDeep(src);
	}

	if (height > 1){
		SlidingStripeAverage<SlidingStripe<WindowConfig, WindowCore, false> > window2(height); // <WindowConfig>
		mout.debug2() << window2 << mout.endl;
		window2.setSrcFrame(tmp);
		window2.setDstFrame(dst);
		mout.debug2() << window2 << mout.endl;
		window2.run();
	}
	else {
		mout.info("Special case optimized: Wx1 window" );
		dst.copyData(tmp);
	}

	//dst.scaling.set(src.scaling); //
}

/// Calls SlidingStripeOp<T,T2>::process() separately for each image channel. This is natural for many operations, such as averaging.


// Raise filterUnweighted
void FastAverageOp::traverseChannel(const Channel & src, const Channel & srcAlpha, Channel & dst, Channel & dstAlpha) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), name, "process(src,srcW,dst,dstW)");
	//mout.warn(src      );
	//mout.warn(srcAlpha );

	size_t width  = conf.getWidth();
	size_t height = conf.getHeight() ? conf.getHeight() : width;

	/// Special case
	/*
	if ((width==1) && (height==1)){
		mout.note("Special case: 1x1 window => simple copy" );
		dst.copyData(src);
		return;
	}
	*/

	Image tmp(src.getConf());
	Image tmpAlpha(srcAlpha.getConf());

	// makeCompatible(src.getConf(), tmp);
	// makeCompatible(srcAlpha.getConf(), tmpAlpha);
	// tmpAlpha.setPhysicalRange(0, 1, true); // important

	mout.warn("tmp:      " , tmp      );
	mout.warn("tmpAlpha: " , tmpAlpha );



	// TODO:Generalize to SeparableWindowOp,
	SlidingStripeAverageWeighted<SlidingStripe<WindowConfig, WeightedWindowCore, true> > window1(width);
	window1.setSrcFrame(src);
	window1.setSrcFrameWeight(srcAlpha);
	window1.setDstFrame(tmp);
	window1.setDstFrameWeight(tmpAlpha);
	window1.run();
	// FilePng::write(tmp, name+"-D1.png");
	// FilePng::write(tmpAlpha, name+"-A1.png");

	SlidingStripeAverageWeighted<SlidingStripe<WindowConfig, WeightedWindowCore, false> > window2(height);
	window2.setSrcFrame(tmp);
	window2.setSrcFrameWeight(tmpAlpha);
	window2.setDstFrame(dst);
	window2.setDstFrameWeight(dstAlpha);
	window2.run();
	// FilePng::write(dst, name+"-D2.png");
	// FilePng::write(dstAlpha, name+"-A2.png");


	// mout.warn("check scaling: " , src.getScaling() , " ? => " , dst.getScaling() );
	// dst.setScaling(src.getScaling()); // ??

}

}

}

// Drain
