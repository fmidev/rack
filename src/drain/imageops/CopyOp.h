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
#ifndef COPYOP_H_
#define COPYOP_H_

//#include "ImageOp.h"
#include "FunctorOp.h"

namespace drain
{

namespace image
{
// help copy

/* Defunct
 *
   The type may be one of the followings:
  - 'C'  :  unsigned char
  - 'S' :  unsigned short
  - 'src' : copied from the source
  - '*' :  keep current, or if undefined, copy from the source

 */

/** Copies images or channels to another image. Does not scale intensities. Target type will be set, if unset.


To copy green channel to red:
 \code
 drainage image.png --iCopy f --view g --iCopy r --view f  -o copy.png
 \endcode

 To extract the third channel (#2, blue):
 \code
 drainage image.png --view 2  -o blue.png
 drainage image.png --view b  -o blue2.png
 \endcode

 To convert an image to a 16-bit image:
 \code
 drainage image.png --target S --iCopy f  -o copy16bit.png
 \endcode

To create alpha channel from channel #1 (green):
 \code
 drainage image.png --iCopy f --view b --iCopy a  --view f -o copy-rgba.png
 \endcode

In \b drainage, see also \c --inputAlpha .

\see VerticalCatenatorOp .
\see ScaleOp



 */	


class CopyFunctor : public UnaryFunctor {

public:

	CopyFunctor(double scale = 1.0, double bias = 0.0) : UnaryFunctor(__FUNCTION__, "Copies current view to: f=full image, i=image channels, a=alpha channel(s), 0=1st, 1=2nd,...", scale, bias){
		//this->parameters.link("srcView", srcView = "f");
		this->parameters.link("dstView", dstView = "");
		this->parameters.link("scale", this->scale);
		this->parameters.link("bias", this->bias);
		//this->parameters.link("LIMIT", this->LIMIT);
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
		//this->parameters.link("LIMIT", this->LIMIT=LIMIT);
	}

	CopyOp(const CopyOp & cmd) : UnaryFunctorOp<CopyFunctor>(cmd) {
		//setParameters(p)
		//this->functor.s
		//this->parameters.link("LIMIT", this->LIMIT=LIMIT);
	}


	virtual
	//void make Compatible(const ImageFrame & src, Image & dst) const;
	void getDstConf(const ImageConf & src, ImageConf & dst) const;


	//inline
	void process(const ImageFrame & srcFrame, Image & dstImage) const;

	virtual
	inline
	void traverseChannel(const Channel &src, Channel &dst) const {
		Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		mout.debug() << "start" << mout.endl;
		UnaryFunctorOp<CopyFunctor>::traverseChannel(src, dst);
	}

	virtual
	inline
	void traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {
		Logger mout(getImgLog(), __FILE__, __FUNCTION__);
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

// Drain
