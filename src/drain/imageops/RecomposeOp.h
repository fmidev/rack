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
#ifndef Recompose_OP_H_
#define Recompose_OP_H_

#include "ImageOp.h"

namespace drain
{

namespace image
{


/// Create vertical image composed of separate views.
/**

Compose an image to a panel displaying red, green and blue channels:
\code
  drainage image.png --iRecompose rgb -o recompose-rgb.png
\endcode

\~exec
make graphic-tr.png
make graphic-tr-16b.png
\~

Compose an image to a panel displaying red, green and blue channels:
\code
  drainage graphic-tr.png     --iRecompose ia  -o recompose-ia.png
  drainage graphic-tr-16b.png --iRecompose ia  -o recompose-ia-16b.png
\endcode

 */	
class RecomposeOp : public ImageOp
{

public:

	inline
	RecomposeOp(const std::string & views = "fia") : ImageOp (__FUNCTION__, "Recompose image.") {
		 parameters.link("views", this->views = views, "i=image,f=imageWithAlpha,r=red,g=green,b=blue,a=alpha");
	};

	inline
	RecomposeOp(const RecomposeOp & op) : ImageOp(op) {
		parameters.copyStruct(op.getParameters(), op, *this);
	}

	virtual
	void getDstConf(const ImageConf & src, ImageConf & dst) const;

	virtual
	void process(const ImageFrame & src, Image & dst) const;

	//virtual
	//void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;


protected:

	std::string views;

};

} // image::

} // drain::

#endif // Recompose_OP_H_
