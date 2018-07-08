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
#ifndef CROP_OP_H_
#define CROP_OP_H_

#include "ImageOp.h"

namespace drain
{

namespace image
{
// help copy

/**

To copy part of an image to another image:
\code
  drainage image.png --crop 100,100  -o crop.png
\endcode

Offset can be given as third and fourth arguments:
\code
  drainage image.png --crop 100,100,+100,+100  -o crop-offset.png
\endcode

Cropping area can be larger than the original image, and the offsets can be negative.
\code
  drainage image.png --crop 640,400,-100,-100  -o crop-larger.png
\endcode

The coordinates outside the image are applied using coordinate handler, which defines the handling separately for each direction.
By default, the \i coodinate \i policy is \c LIMIT , which identifies the external pixels with the the edge pixels.


 */	
class CropOp : public ImageOp
{

public:

	inline
	CropOp(int width=0, int height=0, int i0=0, int j0=0) : ImageOp (__FUNCTION__, "Crop image.") {
		 parameters.reference("width", this->width  = width);
		 parameters.reference("height",this->height = height);
		 parameters.reference("i", this->i0 = i0);
		 parameters.reference("j", this->j0 = j0);
		 //setParameters(p);
	};

	virtual
	void makeCompatible(const ImageFrame & src, Image & dst) const;

	/*
	virtual
	void process(const ImageFrame & src, Image & dst) const{
		drain::Logger mout(getImgLog(), name, __FUNCTION__);

		//mout.debug() << "delegating back to ImageOp::processOverlappingWithTemp" << mout.endl;
		if (processOverlappingWithTemp(src, dst))
			return;

		makeCompatible(src, dst);

		ImageTray<const Channel> srcTray;
		srcTray.setChannels(src);

		ImageTray<Channel> dstTray;
		dstTray.setChannels(dst);

		traverseChannels(srcTray, dstTray);

	}
	*/

	inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		drain::Logger mout(this->name, __FUNCTION__);
		traverseChannelsSeparately(src, dst);
	}

	virtual
	void traverseChannel(const Channel & src, Channel & dst) const;


protected:

	int i0;
	int j0;
	int width;
	int height;

};

} // image::

} // drain::

#endif /*CROP_OP_H_*/
