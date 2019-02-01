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
#ifndef IMAGE_CHANNEL_H_
#define IMAGE_CHANNEL_H_ "ImageFrame 0.9,  2011.09.25 Markus.Peura@fmi.fi"

#include <stddef.h>  // size_t

#include "util/CastableIterator.h"
#include "util/VariableMap.h"
#include "util/TypeUtils.h"
#include "ImageFrame.h"


namespace drain
{

/// Namespace for images and image processing tools.
namespace image
{


/// Image with static geometry
/**
 *    ImageFrame is an image which has a fixed type and geometry.
 *
 *    The pixel array is readable and writable.
 *    Scaling and coordinate handlers can be altered.
 *
 */
class Channel : public ImageFrame {

public:

	inline
	Channel(){
	};

	inline
	Channel(const Channel & channel){
		encoding.scaling.set(channel.getScaling());
	};

	virtual inline
	Channel & getChannel(size_t i){
		if (i != 0)
			throw std::runtime_error("Channel: getChannel(i) with i>0");
		return *this;
	};


	virtual inline
	const Channel & getChannel(size_t i) const {
		if (i != 0)
			throw std::runtime_error("Channel: getChannel(i) with i>0");
		return *this;
	}

	virtual inline
	Channel & getAlphaChannel(size_t i=0){
		throw std::runtime_error("Channel: getAlphaChannel() impossible");
		return *this;
	};

	virtual inline
	const Channel & getAlphaChannel(size_t i=0) const {
		throw std::runtime_error("Channel: getAlphaChannel() impossible");
		return *this;
	};


};

/// View to a single channel.
/**
 *
 */
class ChannelView : public Channel {

public:

	inline
	ChannelView(){
	};

	inline
	ChannelView(const ChannelView & channel) : Channel(channel) {
		//std::cerr << "ChannelView copy constr\n";
	};

	inline
	ChannelView(const ImageFrame & src, size_t channel = 0) {
		setView(src, channel);
	};

	inline
	void setView(const ImageFrame & src, size_t channel){
		ImageFrame::setView(src, channel, 1);
		geometry.setChannelCount(1,0);
	}

	inline
	void setView(const Channel & src){
		ImageFrame::setView(src, 0, 1);
		geometry.setChannelCount(1,0);
	}



};


/// Multi-channel ImageFrame.
class MultiChannel : public ImageFrame {

public:

	inline
	MultiChannel(){};

	inline
	MultiChannel(const MultiChannel & img){
		this->setScaling(img.getScaling());
	};


	Channel & getChannel(size_t i);

	const Channel & getChannel(size_t i) const;

	Channel & getAlphaChannel(size_t i=0);

	const Channel & getAlphaChannel(size_t i=0) const;


	// hide?
	inline
	const std::vector<ChannelView> & getChannelVector() const {
		updateChannelVector();
		return channelVector;
	};

protected:

	void updateChannelVector() const;

	mutable std::vector<ChannelView> channelVector;


};



} // image::
} // drain::

#endif /* IMAGE_FRAME_H_*/

// Drain
