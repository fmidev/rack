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
#ifndef HIGHPASS_OP_H_
#define HIGHPASS_OP_H_

//#include "SlidingWindowAverage.h"
#include "ImageOp.h"

//#include "util/FunctorPack.h"
//#include "image/File.h"
//#include "imageops/FunctorOp.h"
#include "imageops/ImageOp.h"

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
   drainage gray.png --highPass '5,5,10.0' -o highPass.png
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
