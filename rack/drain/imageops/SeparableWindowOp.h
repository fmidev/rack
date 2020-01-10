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
#ifndef SeparableWindowOP_H_
#define SeparableWindowOP_H_


#include "WindowOp.h"

namespace drain
{

namespace image
{

/// Sliding window averaging operator with optional weighting support.
/**

  Implemented as two consequent SlidingStripeOp's: horizontal filtering followed by a vertical one.
  The width and the height, if unset, will be set automatically according to radius.

  \code
    ainage image.png --gaussianAverage 15   -o gaussianAverage.png
  \endcode

  If the source image contains several channels, each channel is treated separately.

  \code
     ainage image.png        --gaussianAverage 5,25  -o gaussianAverage-vert.png
     ainage orvokki-rgba.png --gaussianAverage 25,25 -o flower-blurred.png
  \endcode

 */
class SeparableWindowOp : public WindowOp<GaussianStripe<WindowCore> >
{
public:

	/**
	 *  \param halfWidth - distance relative to width and height, where gaussian kernel obtains value 0.5.
	 */
	SeparableWindowOp(int width=1, int height=0, double halfwidth=0.5);

	/// Top-level function that delegates the invocation to each image channel.
	/**
	 *   If alpha channels are present, delegates to void process(const ImageFrame &src,const ImageFrame &srcWeight, Image &dst,Image &dstWeight) const
	 */

	///
	virtual
	inline
	void initializeParameters(const ImageFrame &src, const ImageFrame &dst) const {
	}

	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		this->processChannelsSeparately(src, dst);
	}


	virtual
	void traverseChannel(const Channel & src, Channel &dst) const ;

	virtual
	void traverseChannel(const Channel & src, const Channel & srcAlpha, Channel &dst, Channel & dstAlpha) const ;



protected:

};



SeparableWindowOp::SeparableWindowOp(int width, int height, double radius) :
		WindowOp<GaussianStripe<WindowCore> >(__FUNCTION__, "Gaussian blur implemented as quick Wx1 and 1xH filtering.") {

	this->conf.width = width;
	this->conf.height = height;
	parameters.reference("radius", this->conf.radius = radius, "distance, relative to width and height, where gaussian kernel obtains value 0.5.");

}


void SeparableWindowOp::traverseChannel(const Channel & src, Channel & dst) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	// TODO: generalize!

	Image tmp;
	makeCompatible(src, tmp);
	//makeCompatible(src, dst); // unneeded in traverse?


	// GaussianStripeHorz window1(conf.width, 0.5*conf.radius*static_cast<double>(conf.width));
	// GaussianStripe2<true> window1(conf.width, 0.5*conf.radius*static_cast<double>(conf.width));
	SlidingWindow<WindowConfig,WindowCore> window1;

	// Jotestin setParams
	window.setSrc(src);
	window.setDst(tmp);
	mout.debug() << window << mout.endl;
	window.runHorz();

	window.setSrc(tmp);
	window.setDst(dst);
	mout.debug() << window << mout.endl;
	window.runVert();

	dst.scaling.setScale(src.scaling.getScale());
}

//void SeparableWindowOp::process(const ImageFrame & src, const ImageFrame & srcWeight, Image & dst, Image & dstWeight) const {
void SeparableWindowOp::traverseChannel(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	Image tmp;
	Image tmpWeight;
	makeCompatible(src, tmp);
	makeCompatible(srcWeight, tmpWeight);
	//makeCompatible(src, dst);
	//makeCompatible(srcWeight, dstWeight);

	GaussianStripeWeighted2<true> window1(conf.width, 0.5*conf.radius*static_cast<double>(conf.width));


	window1.setSrc(src);
	window1.setSrcFrameWeight(srcWeight);
	window1.setDst(tmp);
	window1.setDstFrame1Weight(tmpWeight);
	mout.debug() << window1 << mout.endl;
	window1.run();
	// File::write(tmp, "gauss1d.png");
	// File::write(tmpWeight, "gauss1w.png");

	const int h = (conf.height>0.0) ? conf.height : conf.width;
	GaussianStripeWeighted2<false> window2(h, 0.5*conf.radius*static_cast<double>(h));
	window2.setSrc(tmp);
	window2.setSrcFrameWeight(tmpWeight);
	window2.setDst(dst);
	window2.setDstFrame1Weight(dstWeight);
	mout.debug() << window2 << mout.endl;
	window2.run();

	dst.scaling.setScale(src.scaling.getScale());

}



}  // image::

}  // drain::

#endif

// Drain
