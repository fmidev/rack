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
#ifndef MULTITHRESHOLD_OP_H_
#define MULTITHRESHOLD_OP_H_

#include "ImageOp.h"

namespace drain
{

namespace image
{



class MultiThresholdOp : public ImageOp {

public:

	inline
	MultiThresholdOp() : drain::image::ImageOp(__FUNCTION__, "Change values outside given range to target.min and target.max") {
		parameters.link("range",  range.tuple(),  "accepted values [min:max]").fillArray = true;
		//parameters.link("target", target.tuple(), "substitutes for rejected values [low:high]").fillArray = true;
		parameters.link("min", target.min, "value set below range.min"); //.fillArray = true;
		parameters.link("max", target.max, "value set above range.max"); //.fillArray = true;

		range.set(0.0, 1.0);  // linear range
		target.set(0.0,1.0);  // substitite values (2 distinct values)

	};

	drain::Range<double> range;
	drain::Range<double> target;

	virtual inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		// drain::Logger mout(__FUNCTION__, __FILE__);
		traverseChannelsSeparately(src, dst);
		// traverseChannelsEqually(src, dst);
		// traverseChannelsRepeated(src, dst);
		// traverseMultiChannel(src, dst);
	};


	/// Apply to single channel.
    virtual
    void traverseChannel(const Channel & src, Channel &dst) const;

    /// Apply to single channel with alpha.
    virtual
    void traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const;


};

} // image::

} // drain::

#endif /*COPYOP_H_*/

// Drain
