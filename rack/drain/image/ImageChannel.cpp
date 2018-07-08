/**

    Copyright 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include "ImageChannel.h"

namespace drain {


namespace image {


Channel & MultiChannel::getChannel(size_t i){

	if (i >= getChannelCount()){
		Logger mout(getImgLog(), "MultiChannel", __FUNCTION__);
		mout.error() << "channel index (" << i << ") overflow (" << getChannelCount() << " channels)" << mout.endl;
	}

	updateChannelVector();

	return channelVector[i];
}


const Channel & MultiChannel::getChannel(size_t i) const {

	if (i >= getChannelCount()){
		Logger mout(getImgLog(), "MultiChannel", __FUNCTION__);
		mout.error() << "channel index (" << i << ") overflow (" << getChannelCount() << " channels)" << mout.endl;
	}

	updateChannelVector();

	return channelVector[i];
}

Channel & MultiChannel::getAlphaChannel(size_t k){

	if (k >= getAlphaChannelCount()){
		Logger mout(getImgLog(), "MultiChannel", __FUNCTION__);
		mout.error() << "channel index (" << k << ") overflow (" << getAlphaChannelCount() << " alpha channels)" << mout.endl;
	}

	updateChannelVector();

	const size_t a = getImageChannelCount();
	return getChannel(a);
}

const Channel & MultiChannel::getAlphaChannel(size_t k) const {

	if (k >= getAlphaChannelCount()){
		Logger mout(getImgLog(), "MultiChannel", __FUNCTION__);
		mout.error() << "channel index (" << k << ") overflow (" << getAlphaChannelCount() << " alpha channels)" << mout.endl;
	}

	updateChannelVector();

	const size_t a = getImageChannelCount(); //???
	return getChannel(a);
}




void MultiChannel::updateChannelVector() const {

	Logger mout(getImgLog(), "MultiChannel", __FUNCTION__);

	const size_t n = getChannelCount();

	//if (n > channelVector.size()){ FAIL, old views unchanged
	channelVector.resize(n);
	for (size_t k = 0; k < n; ++k) {
		ChannelView & channel = channelVector[k];
		channel.setView(*this,k); // sets: channel.scalingPtr = this->scaling;

		// Special for alpha channels:
		if (k >= geometry.getImageChannelCount()){
			channel.useOwnScaling();
			if (!channel.getScaling().isPhysical()){
				const std::type_info & t = channel.getType();
				if (Type::call<typeIsSmallInt>(t)){
					channel.getScaling().setPhysicalScale(t, 0.0, 1.0);
					mout.debug() << "using own physical scaling for channel[" << k << "], " << channel.getScaling() << mout.endl;
				}
			}
		}
	}
	//};

}


}  // image::

}  // drain::
