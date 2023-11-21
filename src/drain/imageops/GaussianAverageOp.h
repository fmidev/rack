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
#ifndef GaussianBlurOP_H_
#define GaussianBlurOP_H_


#include "drain/image/GaussianWindow.h"
#include "WindowOp.h"

namespace drain
{

namespace image
{

/// Sliding window averaging operator with optional weighting support.
/**

  Implemented as two consequent SlidingStripeOp's: horizontal filtering followed by vertical.

  \code
     drainage shapes.png --iGaussianAverage 25   -o gaussianAverage.png
     drainage shapes.png     --iGaussianAverage 5:25  -o gaussianAverage-vert.png
  \endcode

  If the source image contains several channels, each channel is treated separately.

  \code
     drainage image-rgba.png --iGaussianAverage 25    -o gaussianAverageWeighted-rgba.png
  \endcode


  \see FastAverageOp

 */
class GaussianAverageOp : public WindowOp<Window<GaussianWindowConf> > //WindowOp<GaussianStripe<WindowCore> >
{
public:

	/**
	 *  \param halfWidth - iDistance relative to width and height, where gaussian kernel obtains value 0.5.
	 */
	GaussianAverageOp(int width=1, int height=0, double halfwidth=0.5);

	inline
	GaussianAverageOp(const GaussianAverageOp & op) : WindowOp<Window<GaussianWindowConf> >(op) {
	  //this->parameters.copyStruct(op.getParameters(), op, *this);
		this->parameters.copyStruct(op.getParameters(), op.conf, conf);
	};

	/// Delegates the invocation separately for each channel.
	/**
	 *   If alpha channels are present, delegates to void process(const ImageFrame &src,const ImageFrame &srcWeight, Image &dst,Image &dstWeight) const
	 */
	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		this->traverseChannelsSeparately(src, dst);
	}

	virtual
	void traverseChannel(const Channel & src, Channel &dst) const ;

	virtual
	void traverseChannel(const Channel & src, const Channel & srcAlpha, Channel &dst, Channel & dstAlpha) const ;



protected:

};



}  // image::

}  // drain::

#endif

// Drain
