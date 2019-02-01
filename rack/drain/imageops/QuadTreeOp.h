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
	 *  Other classes - like str operators - may call these also directly, but then compatibility must be ensured.
	 */
	virtual
	void process(const ImageFrame & src, ImageFrame & dst) const;

	/// The main functionality called by traverseFrame() after image compatibility check and tmp allocations
	/**
	 *  Other classes - like str operators - may call these also directly, but then compatibility must be ensured.
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

// Drain
