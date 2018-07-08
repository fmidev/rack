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
#ifndef QuadTreeOP_H_
#define QuadTreeOP_H_

#include "ImageOp.h"

namespace drain
{

namespace image
{
// help copy


/// QuadTrees the image, stretching the image correspondingly.
/**
  \see ScaleOp

  To shrink an image:
  \code
    rainage image.png --QuadTree 100,100  -o QuadTreeShrink.png
  \endcode

  To expand an image using nearest-pixel interpolation:
  \code
    rainage image.png --QuadTree 1000,800    -o QuadTreeExpand.png
    rainage image.png --QuadTree 1000,800,n  -o QuadTreeExpand.png
  \endcode

  To expand an image using bilinear interpolation:
  \code
  	 rainage image.png --QuadTree 1000,800,b  -o QuadTreeExpandBilinear.png
  \endcode

 */	
class QuadTreeOp : public ImageOp {

public:

	QuadTreeOp(size_t width=0, size_t height=0, const std::string & interpolation = "n") : ImageOp(__FUNCTION__, "Resize geometry and scale intensities") {
		this->parameters.reference("width",  this->width=width, "pix");
		this->parameters.reference("height", this->height=height, "pix");
		this->parameters.reference("interpolation", this->interpolation=interpolation, "n=nearest,b=bilinear");
	}

	size_t width;
	size_t height;
	std::string interpolation;

	virtual
	void makeCompatible(const ImageFrame & src, Image & dst) const;

	/// The main functionality called by traverseFrame() after image compatibility check and tmp allocations
	/**
	 *  Other classes - like other operators - may call these also directly, but then compatibility must be ensured.
	 */
	virtual
	void process(const ImageFrame & src, ImageFrame & dst) const;

	/// The main functionality called by traverseFrame() after image compatibility check and tmp allocations
	/**
	 *  Other classes - like other operators - may call these also directly, but then compatibility must be ensured.
	 */
	virtual
	inline
	void process(const ImageFrame & src, const ImageFrame & srcAlpha, ImageFrame & dst, ImageFrame & dstAlpha) const {
		process(src, dst);
		process(srcAlpha, dstAlpha);
	}



};


} // image::

} // drain::

#endif /*ResizeOP_H_*/
