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

#include "TransparentOp.h"

namespace drain
{

namespace image
{





// void TransparentOp::make Compatible(const ImageFrame &src,Image &dst) const  {
void TransparentOp::getDstConf(const ImageConf & src, ImageConf & dst) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	//if (!dst.typeIsSet())
	//	dst.setType(src.getType());

	dst.setArea(src.getGeometry());
	// size_t w = src.getWidth();
	// size_t h = src.getHeight();
	// size_t i = src.getImageChannelCount();
	// size_t a = std::max(1UL, src.getAlphaChannelCount());
	// dst.setGeometry(w, h, i, a);

	dst.setChannelCount(src.getImageChannelCount(), std::max(1UL, src.getAlphaChannelCount()));
	mout.debug() << "dst:" << dst << mout.endl;

};

/// Uses that of UnaryFunctorOp<drain::FuzzyStep<double> >!
/**
 *   IDEA: only dst.alpha will be forwarded here.
 */
//void traverseChannel(const Channel &src, Channel & dst) const



void TransparentOp::traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {  //  = 0;
	//drain::Logger mout(this->getName()+"(UnaryFunctorOp)", __FUNCTION__);
	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	if (dst.hasAlpha()){
		mout.fail() << "dst has no alpha channel()" << mout.endl;
		return;
	}

	size_t srcChannels = src.getGeometry().channels.getImageChannelCount();
	if (srcChannels > 1){
		mout.fail() << "src has several image channels, using the last" << (srcChannels-1) << mout.endl;
	}

	mout.note() << "forwarding to: traverseChannel(src.get(), dst.getAlpha())  " << mout.endl;

	UnaryFunctorOp<drain::FuzzyStep<double> >::traverseChannel(src.get(0), dst.getAlpha());

}


void TransparentOp::traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.note() << "forwarding to: traverseChannel(src.get(), dst.getAlphaChannel())  " << mout.endl;
	UnaryFunctorOp<drain::FuzzyStep<double> >::traverseChannel(src, dstAlpha);

};



} // image::

} // drain::

