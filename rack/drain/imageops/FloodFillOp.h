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

