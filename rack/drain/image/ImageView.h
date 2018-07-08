/**

    Copyright 2017 -   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#ifndef IMAGE_VIEW_H_
#define IMAGE_VIEW_H_ "ImageView 1.0,  2017/10 Markus.Peura@fmi.fi"

#include <stddef.h>  // size_t


#include "ImageChannel.h"

namespace drain
{

/// Namespace for images and image processing tools.
namespace image
{



/// ImageFrame that also has channels.
/**
 *
 */
class ImageView : public MultiChannel {

public:

	inline	ImageView() {};

	ImageView(const ImageFrame & src, size_t channelStart, size_t channelCount, bool catenate=false) {
		ImageFrame::setView(src, channelStart, channelCount, catenate);
	}


	/// Views the whole image
	inline
	void setView(const ImageFrame & src){
		ImageFrame::setView(src, 0, src.getChannelCount());
	}

	/// Views a single channel. Resulting channel type is image, regardless of target channel type (image or alpha),
	/**
	 *  If you want the type to be alpha channel, call setView(target, imageChannelCount, 1)
	 */
	inline
	void setView(const ImageFrame & src, size_t channel){
		ImageFrame::setView(src, channel, 1);
		geometry.setChannelCount(1,0);
	}


	///  Sets image to view another image.
	void setView(const ImageFrame & src, const std::string & view);

};




} // image::
} // drain::

#endif /* IMAGE_FRAME_H_*/
