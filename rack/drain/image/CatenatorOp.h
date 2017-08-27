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
#ifndef CATENATOROP_H_
#define CATENATOROP_H_

#include "ImageOp.h"

namespace drain
{

namespace image
{

/// Operator for concatenating images vertically or as channels.
/**
 *
 */
class CatenatorOp : public ImageOp
{
public:
	

	//CatenatorOp(const std::string & p="") : ImageBaseOp("CatenatorOp","Catenates images, mode=vert|depth (vertically or in-depth, by adding channels). Horz not yet implemented.",
	CatenatorOp(double scale=1.0, double offset=0.0) : ImageOp(__FUNCTION__, "Catenates images"){
		reference("scale", scale = 1.0);
		reference("offset", offset = 0.0);
		//setParameters(p);
	};

	virtual ~CatenatorOp(){};
	//void filter(const Image &src,Image &dst) const;

	double scale;
	double offset;

	/// The width of the image is the maximum of dst width, if src initialized, else src width.
	//void catenateInDepth(const Image &src,Image &dst) const;
	//void catenateVertically(const Image &src,Image &dst) const;
};

/** Catenates separate channels to another image. Scales image, if needed.
  @see CopyOp

To copy image...
 \code
 ???? image.png --view F  -o copy.png
 \endcode
*/
class ChannelCatenatorOp : public CatenatorOp
{
public:

	void filter(const Image &src, Image &dst) const; // { filter(src, dst, gain, offset); };

};


/** Catenates separate channels to another image.
  @see CopyOp

To copy image...
 \code
 ???? image.png --view F  -o copy.png
 \endcode
*/
class VerticalCatenatorOp : public CatenatorOp
{
public:

	void filter(const Image &src, Image &dst) const;
};




}

}

#endif /*CATENATOR_H_*/

// Drain
