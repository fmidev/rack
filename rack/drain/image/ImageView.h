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

// Drain
