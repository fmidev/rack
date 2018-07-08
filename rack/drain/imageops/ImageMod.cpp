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


#include <ostream>
#include <fstream>
#include "util/Log.h"
#include "image/Sampler.h"
#include "ImageMod.h"



namespace drain
{

namespace image
{


void ImageMod::process(Image & dst) const {

	//std::cerr << "TEST" << std::endl;
	drain::Logger mout(this->name+"(ImageModifier::)", __FUNCTION__);
	initialize(dst);
	//ImageFrame & dstFrame = dst;
	mout.debug(1) << "forwarding to: traverseChannel(Channel &)" << mout.endl;
	ImageTray<Channel> dstTray;
	dstTray.setChannels(dst);

	traverseChannels(dstTray);
}

/*
void ImageMod::traverse rame(ImageFrame & dst) const {
	drain::Logger mout(this->name+"(ImageModifier::)", __FUNCTION__);
	mout.debug(1) << "forwarding to: applyAsChannelTray(Channel &)" << mout.endl;
	traverseChannels(dst);
}
*/

/*
//void ImageMod::traverseFrame(Tray<Image> & dst) const {

	drain::Logger mout(this->name, __FUNCTION__);

	Tray<ImageFrame> frameDst;

	for (typename Tray<Image>::map_t::iterator it = dst.begin(); it != dst.end(); ++it){
		mout.debug(2) << "initializing image #" << it->first << mout.endl;
		initialize(it->second);
		ImageFrame & data  = it->second;
		mout.debug(2) << "appending image #" << it->first << mout.endl;
		frameDst.append(data);
	}

	mout.debug(1) << "forwarding to traverseFrame(Tray<ImageFrame> &)" << mout.endl;
	traverseFrame(frameDst);

}
*/

/*
void ImageMod::processImages(Tray<Image> & dst) const {

	drain::Logger mout(this->name, __FUNCTION__);

	// Check if multiple multichannel
	bool MULTICHANNEL = false;
	for (typename Tray<Image>::map_t::iterator it = dst.begin(); it != dst.end(); ++it){

		initialize(it->second);

		ImageTray<Channel> channelDst(it->second);
		if (MULTICHANNEL)
			mout.note() << "input contains several images, and calling traverseFrame(ImageTray<ChannelFrame> &)" << mout.endl;
		mout.debug(2) << "appending image #" << it->first << mout.endl;
		//channelDst.append(it->second);
		traverseChannels(channelDst);
		if (it->second.getChannelCount() > 1)
			MULTICHANNEL = true; // warn next round.
	}

}
*/

/*
//void ImageMod::traverseFrame(Tray<ImageFrame> & dst) const {

	drain::Logger mout(this->name, __FUNCTION__);

	// Check if multiple multichannel
	bool MULTICHANNEL = false;
	for (typename Tray<ImageFrame>::map_t::iterator it = dst.begin(); it != dst.end(); ++it){
		ImageTray<Channel> channelDst;
		if (MULTICHANNEL)
			mout.note() << "input contains several images, and calling traverseFrame(ImageTray<ChannelFrame> &)" << mout.endl;
		mout.debug(2) << "appending image #" << it->first << mout.endl;
		channelDst.append(it->second);
		traverseFrame(channelDst);
		if (it->second.getChannelCount() > 1)
			MULTICHANNEL = true; // warn next round.
	}

}
*/

void ImageMod::traverseChannels(ImageTray<Channel> & dst) const {
	drain::Logger mout(this->name+"(ImageModifier::)", __FUNCTION__);
	mout.debug(1) << "forwarding to: processChannelsSeparately(Channel &)" << mout.endl;
	processChannelsSeparately(dst);
}


void ImageMod::processChannelsSeparately(ImageTray<Channel> & dst) const {

	drain::Logger mout(this->name+"(ImageModifier::)", __FUNCTION__);

	if (dst.hasAlpha()){

		typename ImageTray<Channel>::map_t::iterator ait = dst.alpha.begin();

		for (typename ImageTray<Channel>::map_t::iterator dit = dst.begin(); dit != dst.end(); ++dit){
			mout.debug(1) << "invoke traverseFrame(d,a) for image #" << dit->first << mout.endl;
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
		for (typename ImageTray<Channel>::map_t::iterator it = dst.begin(); it != dst.end(); ++it){
			mout.debug(1) << "invoke traverseFrame(d) for image #" << it->first << mout.endl;
			Channel & data  = it->second;
			//initialize(data);
			traverseChannel(data);
		}
	}

}


// "Final" ie DONT CHANGE THIS. Important that the invocations converge to traverseFrame(ImageTray<Channel> & dst) .
void ImageMod::traverseChannels(ImageFrame & dst) const {

	drain::Logger mout(this->name, __FUNCTION__);

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

