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
#ifndef FLOODFILL2_H
#define FLOODFILL2_H

#include <sstream>
#include <ostream>

#include "image/Coordinates.h"
#include "image/FilePng.h"
#include "image/SegmentProber.h"

#include "ImageOp.h"

namespace drain
{
namespace image
{

/// A fill operation for one color.
/**  Uses SegmentProber<> in semi-recursive image traversal.
 *   \author Markus.Peura@fmi.fi
 \code
   drainage shapes.png --floodFill  100,100,min=50,value=128 -o shapes-fill.png
 \endcode
 */
class FloodFillOp : public ImageOp {

public:

	FloodFillOp(int i=0, int j=0, double min=1.0, double max=255.0, double value=128.0);

	//virtual void makeCompatible(const ImageFrame & src,Image & dst) const;
	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		//drain::Logger mout(this->name+"(ImageOp::)[const ImageTray &, ImageTray &]", __FUNCTION__);
		traverseChannelsSeparately(src, dst);
	}

	virtual
	void traverseChannel(const Channel & src, Channel & dst) const;

	int i0;
	int j0;
	double max;
	double min;
	double value;

};


}
}
#endif /* FLOODFILL_H_ */


// Drain
