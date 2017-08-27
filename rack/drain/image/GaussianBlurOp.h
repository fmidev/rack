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

//#include "SlidingStripeOp.h"
#include "WindowOp.h"
#include "GaussianWindow.h"

namespace drain
{

namespace image
{

/// Sliding window averaging operator with optional weighting support.
/**

  Implemented as two consequent SlidingStripeOp's: horizontal filtering followed by a vertical one.
  The width and the height, if unset, will be set automatically according to radius.

  \code
    drainage image.png --gaussianBlur 15   -o gaussianBlur.png
  \endcode

  If the source image contains several channels, each channel is treated separately.

 \code
  drainage image.png --gaussianBlur 5,25   -o gaussianBlur-vert.png
  drainage orvokki-rgba.png --gaussianBlur 25,25 -o flower-blurred.png
 \endcode
 */
class GaussianBlurOp : public WindowOpP<GaussianStripe>
{
public:

	/**
	 *  \param halfWidth - distance relative to width and height, where gaussian kernel obtains value 0.5.
	 */
	GaussianBlurOp(int width=1, int height=0, double halfwidth=0.5);

	/// Top-level function that delegates the invocation to each image channel.
	/**
	 *   If alpha channels are present, delegates to void filter(const Image &src,const Image &srcWeight, Image &dst,Image &dstWeight) const
	 */

	///
	virtual
	inline
	void initializeParameters(const Image &src, const Image &dst) const {
	}


	/// Calls SlidingStripeOp<T,T2>::filter() separately for each image channel. This is natural for many operations, such as averaging.
	virtual
	void traverse(const Image &src, Image &dst) const;

	virtual
	void traverse(const Image &src, const Image &srcWeight, Image &dst, Image &dstWeight) const;

protected:

	/// Uses own tmp image, hence returns false.
	virtual
	bool filterWithTmp(const Image & src, Image & dst) const {
		return false;
	}


	/// Uses own tmp image, hence returns false.
	virtual
	bool filterWithTmp(const Image & src, const Image & src2, Image & dst) const {
		return false;
	}


};



}  // image::

}  // drain::

#endif

// Drain
