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
#include "drain/util/Log.h"
#include "CopyOp.h"

namespace drain {

namespace image {

// Consider Image::swap() ?
void CopyOp::getDstConf(const ImageConf & src, ImageConf & dst) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	if (!dst.typeIsSet())
		dst.setType(src.getType());

	if (dst.isEmpty())
		dst.setGeometry(src.getGeometry());
	else
		dst.setArea(src.getGeometry());

	if (functor.dstView.empty()){
		dst.setChannelCount(src.getImageChannelCount(), src.getAlphaChannelCount());
	}
	else if (functor.dstView == "a"){
		//mout.warn("request dst alpha" );
		dst.setAlphaChannelCount(1);
		//dst.getAlphaChannel(0).fill(128);
		//dst.getAlphaChannel(); // create, if nonexistent
	}
	else if ((functor.dstView != "f") && (functor.dstView != "i")){
		//mout.warn("request dst view" , functor.dstView );
		const size_t k = dst.getGeometry().getChannelIndex(functor.dstView);
		//mout.warn(" dst: channels:" , k );
		if (k >= dst.getImageChannelCount())
			dst.setChannelCount(k+1, dst.getAlphaChannelCount());
		mout .debug3() << " dst now: " << dst << mout.endl;

	}

	mout.debug2(" src:       " , src );
	/*
	 drain::ValueScaling s;
	  s.adoptScaling(src.getScaling(), src.getType(), dst.getType());
	  dst.setScaling(s);
	*/
	dst.adoptScaling(src, src.getType(), dst.getType());
	mout.debug2(" dst:       " , dst );

	mout.warn(" dst(orig): " , dst );

}

void CopyOp::process(const ImageFrame & srcFrame, Image & dstImage) const {
	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), this->name+"(ImageOp)[const ImageFrame &, Image &]", __FUNCTION__);
	mout.debug2("calling makeCompatible()" );
	makeCompatible(srcFrame.getConf(), dstImage);
	ImageView dst2; //(dstImage, functor.dstView);
	dst2.setView(dstImage, functor.dstView);
	ImageTray<const Channel> srcTray; //(srcFrame);
	srcTray.setChannels(srcFrame);
	ImageTray<Channel> dstTray; //(dst2);
	dstTray.setChannels(dst2);
	traverseChannels(srcTray, dstTray);
}

/*
void CopyOp::makeCompatible(const ImageFrame & src, Image & dst) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	if (!dst.typeIsSet())
		dst.setType(src.getType());

	if (dst.isEmpty())
		dst.setGeometry(src.getGeometry());

	if (functor.dstView.empty()){
		dst.setChannelCount(src.getImageChannelCount(), src.getAlphaChannelCount());
	}
	else if (functor.dstView == "a"){
		//mout.warn("request dst alpha" );
		dst.setAlphaChannelCount(1);
		dst.getAlphaChannel(0).fill(128);
		//dst.getAlphaChannel(); // create, if nonexistent
	}
	else if ((functor.dstView != "f") && (functor.dstView != "i")){
		//mout.warn("request dst view" , functor.dstView );
		const size_t k = dst.getChannelIndex(functor.dstView);
		//mout.warn(" dst: channels:" , k );
		if (k >= dst.getImageChannelCount())
			dst.setChannelCount(k+1, dst.getAlphaChannelCount());
		mout .debug3() << " dst now: " << dst << mout.endl;

	}

	mout.debug2(" src:       " , src );
	mout.debug3(" dst(orig): " , dst );
	dst.adoptScaling(src.getConf());
	mout.debug2(" dst:       " , dst );

}
*/


}  // namespace image

}  // namespace drain

// Drain
