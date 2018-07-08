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
#ifndef COPYOP_H_
#define COPYOP_H_

//#include "ImageOp.h"
#include "FunctorOp.h"

namespace drain
{

namespace image
{
// help copy

/** Copies images or channels to another image. Does not scale intensities. Target type will be set, if unset.
  @see ScaleOp
  The type may be one of the followings:
  - '8'  :  unsigned char
  - '16' :  unsigned short
  - 'src' : copied from the source
  - '*' :  keep current, or if undefined, copy from the source

To copy green channel to red:
 \code
 drainage image.png --copy f --view g --copy r --view f  -o copy.png
 \endcode

 To extract the third channel (#2, blue):
 \code
 drainage image.png --view 2  -o blue.png
 drainage image.png --view b  -o blue2.png
 \endcode

 To convert an image to a 16-bit image:
 \code
 drainage image.png --target S --copy f  -o copy16bit.png
 \endcode

To create alpha channel from channel #1 (green):
 \code
 drainage image.png --copy f --view b --copy a  --view f -o copy-rgba.png
 \endcode

\see ChannelCatenatorOp and \see VerticalCatenatorOp .
In \b drainage, see also \c --inputAlpha .

 */	


class CopyFunctor : public UnaryFunctor {

public:

	CopyFunctor(double scale = 1.0, double bias = 0.0) : UnaryFunctor(__FUNCTION__, "Copies current view to: f=full image, i=image channels, a=alpha channel(s), 0=1st, 1=2nd,...", scale, bias){
		//this->parameters.reference("srcView", srcView = "f");
		this->parameters.reference("dstView", dstView = "");
		this->parameters.reference("scale", this->scale);
		this->parameters.reference("bias", this->bias);
		//this->parameters.reference("LIMIT", this->LIMIT);
	};

	//virtual
	inline
	double operator()(double s) const {
		return this->scale*s + this->bias;
	};

	//std::string srcView;
	std::string dstView;

protected:

	CopyFunctor(const std::string & name, const std::string & description) : UnaryFunctor(name, description){};

};

class CopyOp : public UnaryFunctorOp<CopyFunctor> {

public:

	CopyOp(double scale = 1.0, double bias = 0, bool LIMIT=true) : UnaryFunctorOp<CopyFunctor>(true, true) {
		this->functor.setScale(scale, bias);
		//this->parameters.reference("LIMIT", this->LIMIT=LIMIT);
	}

	virtual
	void makeCompatible(const ImageFrame & src, Image & dst) const;


	inline
	void process(const ImageFrame & srcFrame, Image & dstImage) const {
		drain::Logger mout(getImgLog(), this->name+"(ImageOp)[const ImageFrame &, Image &]", __FUNCTION__);
		mout.debug(1) << "calling makeCompatible()" << mout.endl;
		makeCompatible(srcFrame, dstImage);
		ImageView dst2; //(dstImage, functor.dstView);
		dst2.setView(dstImage, functor.dstView);
		ImageTray<const Channel> srcTray; //(srcFrame);
		srcTray.setChannels(srcFrame);
		ImageTray<Channel> dstTray; //(dst2);
		dstTray.setChannels(dst2);
		traverseChannels(srcTray, dstTray);
	}

	/*
	virtual
	inline
	void traverse rame(const ImageFrame & src, ImageFrame & dst) const {
		ImageView dst2;
		dst2.setView(dst, functor.dstView);
		UnaryFunctorOp<CopyFunctor>::traverse rame(src, dst2);
	}
	*/

	virtual
	inline
	void traverseChannel(const Channel &src, Channel &dst) const {
		Logger mout(getImgLog(), name, __FUNCTION__);
		mout.debug() << "start" << mout.endl;
		UnaryFunctorOp<CopyFunctor>::traverseChannel(src, dst);
	}

	virtual
	inline
	void traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {
		Logger mout(getImgLog(), name, __FUNCTION__);
		/*
		mout.warn() << "src     " << src << mout.endl;
		mout.warn() << "srcAlpha" << srcAlpha << mout.endl;
		mout.warn() << "dst     " << dst << mout.endl;
		mout.warn() << "dstAlpha" << dstAlpha << mout.endl;
		*/
		UnaryFunctorOp<CopyFunctor>::traverseChannel(src, dst);
		UnaryFunctorOp<CopyFunctor>::traverseChannel(srcAlpha, dstAlpha);

	}


};

} // image::

} // drain::

#endif /*COPYOP_H_*/
