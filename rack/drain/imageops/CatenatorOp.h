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
		parameters.reference("scale", this->scale = scale);
		parameters.reference("offset", this->offset = offset);
		//setParameters(p);
	};

	virtual ~CatenatorOp(){};
	//void process(const ImageFrame &src,Image &dst) const;

	double scale;
	double offset;

	/// The width of the image is the maximum of dst width, if src initialized, else src width.
	//void catenateInDepth(const MultiFrame &src,Image &dst) const;
	//void catenateVertically(const MultiFrame &src,Image &dst) const;
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

	/// Modifies the geometry and type of dst.
	/*  This default implementation
	 */
	virtual
	void makeCompatible(const ImageFrame & src, Image & dst) const;

	void process(const ImageFrame &src, Image &dst) const; // { process(src, dst, gain, offset); };

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

	void process(const ImageFrame &src, Image &dst) const;
};




}

}

#endif /*CATENATOR_H_*/
