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
#ifndef GaussianBlurOP_H_
#define GaussianBlurOP_H_


#include "image/GaussianWindow.h"
#include "WindowOp.h"

namespace drain
{

namespace image
{

/// Sliding window averaging operator with optional weighting support.
/**

  Implemented as two consequent SlidingStripeOp's: horizontal filtering followed by vertical.


  \code
    drainage shapes.png --gaussianAverage 25   -o gaussianAverage.png
  \endcode

  If the source image contains several channels, each channel is treated separately.

  \code
     drainage shapes.png      --gaussianAverage 5,25  -o gaussianAverage-vert.png
     drainage image-rgba.png --gaussianAverage 25    -o gaussianAverageWeighted.png
  \endcode

  \~exec
  	  # rainage orvokki-rgba.png --gaussianAverage 25,25 -o flower-blurred.png
  \~

 */
class GaussianAverageOp : public WindowOp<Window<GaussianWindowConf> > //WindowOp<GaussianStripe<WindowCore> >
{
public:

	/**
	 *  \param halfWidth - distance relative to width and height, where gaussian kernel obtains value 0.5.
	 */
	GaussianAverageOp(int width=1, int height=0, double halfwidth=0.5);

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
