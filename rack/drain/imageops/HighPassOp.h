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
#ifndef HIGHPASS_OP_H_
#define HIGHPASS_OP_H_

//#include "SlidingWindowAverage.h"
#include "ImageOp.h"

//#include "drain/util/FunctorPack.h"
//#include "drain/image/File.h"
//#include "drain/imageops/FunctorOp.h"
#include "drain/imageops/ImageOp.h"

//#include "FastAverageOp.h"


namespace drain
{

namespace image
{

/// Finds details at one-pixel scale, defined as difference of image and its low-pass filtered copy.
/**
 *   Implements
 *
 *   The simplest square high-pass filter is the 3x3 filter
 *   -1  -1  -1
 *   -1   8  -1
 *   -1  -1  -1 
 * 
 *   The result can be scaled with "scale" and transposed with "offset".
 *   Internally, applies the fast SlidingStripeAverage and SubstractOp .
 *
 \code
   drainage gray.png --iHighPass '5,5,10.0' -o highPass.png
 \endcode
 */
class HighPassOp : public ImageOp
{
public:

	HighPassOp(int width=1, int height=0, double scale = 1.0, double offset = 0.0);

	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		traverseChannelsSeparately(src, dst);
	}

	/// Apply to single channel with alpha.
	virtual
	void traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const;


	virtual
	void traverseChannel(const Channel & src, Channel & dst) const;

	int width;
	int height;
	double scale;
	double offset;
	bool LIMIT;


};


}

}

#endif /*HIGHPASS_OP_H_*/

// Drain
