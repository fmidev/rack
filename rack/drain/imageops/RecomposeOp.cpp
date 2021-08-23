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
#include "RecomposeOp.h"

namespace drain {

namespace image {

void RecomposeOp::getDstConf(const ImageConf & src, ImageConf & dst) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	if (dst.typeIsSet() && (dst.getType() != src.getType())){
		mout.special() << "changing dst type -> " << src.getType().name() << mout.endl;
	}

	dst.setType(src.getType());
	//dst.setChann elCount(src.channels); // initial guess (maximum)

	Image dummy(1,1);
	dummy.setChannelCount(src.getImageChannelCount(), src.getAlphaChannelCount()); // maximum

	size_t iChannels = 1;
	size_t aChannels = 0;

	size_t panels = 0;
	for (size_t v=0; v<views.size(); ++v){


		if (views[v] == ':'){
			// Skip '|' border marker
			continue;
		} // warn of 'F', flat?
		++panels;

		std::string s;
		s = views[v];
		ImageView view;
		view.setView(dummy, s);

		iChannels = std::max(iChannels, view.getImageChannelCount());
		aChannels = std::max(aChannels, view.getAlphaChannelCount());
		mout.debug() << "channels... " << s << " (" << iChannels << '+' << aChannels << ')' << mout;

	}

	dst.setChannelCount(iChannels, aChannels);

	dst.setArea(src.getWidth(), panels*src.getHeight());

	mout.debug() << "dst: " << dst << mout.endl;

}



void RecomposeOp::process(const ImageFrame & srcFrame, Image & dstImage) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	mout.debug() << parameters << mout;

	makeCompatible(srcFrame.getConf(), dstImage);

	//  if (channels < 3)
	//	mout.debug() << "small number of channels: " << channels << mout.endl;

	const size_t width  = srcFrame.getWidth();
	const size_t height = srcFrame.getHeight();
	const double gray = dstImage.getConf().getTypeMax<double>()/2.0;

	mout.debug() << "src: " << srcFrame << mout;
	//return;

	if (dstImage.hasAlphaChannel()){
		Channel & dstAlpha = dstImage.getAlphaChannel();
		dstAlpha.fill(dstAlpha.getConf().getTypeMax<double>());
	}

	mout.debug() << "dst: " << dstImage << mout;


	ImageView view;
	std::string viewStr;

	int panel = 0;
	//size_t addressOffset = 0;
	bool BORDER = false;

	//for (size_t v=0; v<views.size(); ++v){
	for (size_t v=0; v<views.size(); ++v){


		while ((views[v] == ':') && (v<views.size())){
			BORDER = true;
			++v;
		}
		size_t addressOffset = dstImage.address(0, panel*height);
		++panel;

		viewStr = views[v];
		view.setView(srcFrame, viewStr);
		mout.debug3() << "view:" << viewStr << ':' << view << mout;


		// dst
		size_t a;
		for (size_t k=0; k<dstImage.getChannelCount(); ++k){

			/// Alpha channels for writing?
			if ((k >= dstImage.getImageChannelCount()) && (k >= view.getChannelCount())){
				break;
			}

			size_t kView = k;

			///
			if (kView >= view.getChannelCount())
				kView = view.getChannelCount()-1; // last channel

			mout.debug() << "copy: " << viewStr << '[' << kView  << "] => dst[" << k << ']' << '+' << v << mout;

			const Channel & src = view.getChannel(kView);
			Channel & dst = dstImage.getChannel(k);

			for (size_t j = 0; j < height; j++) {
				for (size_t i = 0; i < width; i++) {
					a = src.address(i,j);
					dst.put(addressOffset + a, src.get<double>(a));
				}
			}

			if (BORDER){
				mout.debug2() << "border" << mout;
				for (size_t i = 0; i < width; i++) {
					//a = src.address(i,j);
					dst.put(addressOffset + i, gray);
				}
			}


		}

	}



}

}  // namespace image

}  // namespace drain

// Drain
