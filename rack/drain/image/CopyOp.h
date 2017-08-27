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

/** Copies images or channels to another image. Does not scale intensities. Target type will be set, if unset.
  @see ScaleOp
  The type may be one of the followings:
  - '8'  :  unsigned char
  - '16' :  unsigned short
  - 'src' : copied from the source
  - '*' :  keep current, or if undefined, copy from the source

To copy image to another image:
 \code
 drainage image.png --copy f  -o copy.png
 \endcode

 To extract the third channel (#2, blue):
 \code
 drainage image.png --copy 2  -o blue.png
 drainage image.png --copy b  -o blue.png
 \endcode

 To convert an image to a 16-bit image:
 \code
 drainage image.png --target S --copy f  -o copy16bit.png
 \endcode

To create alpha channel from channel #1 (green):
 \code
 drainage image.png --copy f --copy 1,a  -o rgba.png
 \endcode
\see ChannelCatenatorOp and \see VerticalCatenatorOp .
In \b drainage, see also \c --inputAlpha .

 */	


class CopyFunctor : public UnaryFunctor {
    public: //re 

	CopyFunctor(double scale = 1.0, double bias = 0.0) : UnaryFunctor(__FUNCTION__, "Copies current view to: f=full image, i=image channels, a=alpha channel(s), 0=1st, 1=2nd,...", scale, bias){
		this->parameters.reference("srcView", srcView = "f");
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

	std::string srcView;
	std::string dstView;

protected:

	CopyFunctor(const std::string & name, const std::string & description) : UnaryFunctor(name, description){};

};

class CopyOp : public UnaryFunctorOp<CopyFunctor> {
    public: //re 

	CopyOp(double scale = 1.0, double bias = 0, bool LIMIT=true) : UnaryFunctorOp<CopyFunctor>(true, true) {
		this->functor.setScale(scale, bias);
		//this->parameters.reference("LIMIT", this->LIMIT=LIMIT);
	}

	void filter(const Image & src, Image & dst) const {

		MonitorSource mout(iMonitor, name, __FUNCTION__);

		Image src2;
		src2.setView(src, functor.srcView);

		if (dst.isEmpty())
			dst.setGeometry(src2.getGeometry());

		if (functor.dstView == "a"){
			dst.getAlphaChannel(); // create, if nonexistent
		}
		else if ((functor.dstView != "f") && (functor.dstView != "i")){
			const size_t k = dst.getChannelIndex(functor.dstView);
			//mout.warn() << " dst: channels:" << k << mout.endl;
			if (k >= dst.getImageChannelCount())
				dst.setChannelCount(k+1, dst.getAlphaChannelCount());
			mout.debug(3) << " dst now: " << dst << mout.endl;
		}
		Image dst2;
		dst2.setView(dst, functor.dstView);
		traverse(src2, dst2);

	};

};

class CopyOp_OLD : public ImageOp
{

public:

	inline
	CopyOp_OLD(const std::string & p = "") : ImageOp(__FUNCTION__,
			"Copies channels: f=full image, i=image channels, a=alpha channel(s), 0=1st, 1=2nd,...") {
		this->parameters.reference("srcView", srcView = "f");
		this->parameters.reference("dstView", dstView = "0" );
		this->parameters.reference("dstType", dstType = "src");

	};

	void filter(const Image & src, Image & dst) const;

	std::string srcView;
	std::string dstView;

	/// "src", "dst"
	std::string dstType;

};

} // image::

} // drain::

#endif /*COPYOP_H_*/

// Drain
