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


#include <drain/Log.h>
#include <ostream>
#include <fstream>
#include "drain/image/Sampler.h"
#include "ImageMod.h"



namespace drain
{

namespace image
{


void ImageMod::process(Image & dst) const {

	//std::cerr << "TEST" << std::endl;
	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL this->name+"(ImageModifier::)", __FUNCTION__);
	initialize(dst);
	//ImageFrame & dstFrame = dst;
	mout.debug2("forwarding to: traverseChannel(Channel &)" );
	ImageTray<Channel> dstTray;
	dstTray.setChannels(dst);

	traverseChannels(dstTray);
}

void ImageMod::traverseChannels(ImageTray<Channel> & dst) const {
	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL this->name+"(ImageModifier::)", __FUNCTION__);
	mout.debug2("forwarding to: processChannelsSeparately(Channel &)" );
	processChannelsSeparately(dst);
}


void ImageMod::processChannelsSeparately(ImageTray<Channel> & dst) const {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL this->name+"(ImageModifier::)", __FUNCTION__);

	if (dst.hasAlpha()){

		ImageTray<Channel>::map_t::iterator ait = dst.alpha.begin();

		for (ImageTray<Channel>::map_t::iterator dit = dst.begin(); dit != dst.end(); ++dit){
			mout.debug2("invoke traverseFrame(d,a) for image #" , dit->first );
			Channel & data  = dit->second;
			Channel & alpha = ait->second;
			traverseChannel(data, alpha);
			if (++ait == dst.alpha.end()){
				// if alpha channels used, re-use them, avoiding re-initialization
				ait = dst.alpha.begin();
			}
		}
	}
	else {
		for (ImageTray<Channel>::map_t::iterator it = dst.begin(); it != dst.end(); ++it){
			mout.debug2("invoke traverseFrame(d) for image #" , it->first );
			Channel & data  = it->second;
			//initialize(data);
			traverseChannel(data);
		}
	}

}


// "Final" ie DONT CHANGE THIS. Important that the invocations converge to traverseFrame(ImageTray<Channel> & dst) .
void ImageMod::traverseChannels(ImageFrame & dst) const {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	ImageTray<Channel> dstChannels;
	dstChannels.setChannels(dst);
	/*
	for (size_t i = 0; i < dst.getImageChannelCount(); ++i) {
		//process(dst.getChannel(i));
		dstChannels.append(dst.getChannel(i));
	}

	for (size_t i = 0; i < dst.getAlphaChannelCount(); ++i) {
		dstChannels.appendAlpha(dst.getAlphaChannel(i));
	}
	*/

	traverseChannels(dstChannels);
}


/*

*/


}
}


// Drain
