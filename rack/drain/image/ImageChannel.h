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
#ifndef IMAGE_CHANNEL_H_
#define IMAGE_CHANNEL_H_ "ImageFrame 0.9,  2011.09.25 Markus.Peura@fmi.fi"

#include <stddef.h>  // size_t

#include "util/CastableIterator.h"
#include "util/VariableMap.h"
#include "util/TypeUtils.h"

/*
#include "Geometry.h"
#include "Coordinates.h"
#include "ImageLike.h"
#include "ImageScaling.h"
*/
#include "ImageFrame.h"


namespace drain
{

/// Namespace for images and image processing tools.
namespace image
{


/// Image with static geometry
/**
 *    ImageFrame is an image which has predefined area and channel geometry.
 *    Its pixel array is readable and writable.
 *
 */
class Channel : public ImageFrame {

public:

	inline
	Channel(){
	};

	inline
	Channel(const Channel & channel){
		scaling.set(channel.getScaling());
	};

	virtual inline
	Channel & getChannel(size_t i){
		if (i!=0)
			throw std::runtime_error("Channel: getChannel(i) with i>0");
		return *this;
	};


	virtual inline
	const Channel & getChannel(size_t i) const {
		if (i!=0)
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

/// Image with static geometry
/**
 *    ImageFrame is an image which has predefined area and channel geometry.
 *    Its pixel array is readable and writable.
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
	void setView(const ImageFrame & src, size_t channel){
		ImageFrame::setView(src, channel, 1);
		geometry.setChannelCount(1,0);
	}

	inline
	void setView(const Channel & src){
		ImageFrame::setView(src, 0, 1);
		geometry.setChannelCount(1,0);
	}


protected:


};


/*
inline // tODO REMOVE
std::ostream & operator<<(std::ostream &ostr, const Channel & src){
	src.toOStr(ostr);
	return ostr;
}
*/



//typedef ImageFrame Channel; // Maybe misleading

/*
inline
std::ostream & operator<<(std::ostream &ostr, const Channel &image){
	//ostr << image.getGeometry() << ' ' << 'X';
	image.toOStr(ostr);
	return ostr;
}
*/

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
