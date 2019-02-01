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

	const size_t a = getImageChannelCount();
	return getChannel(a);
}




void MultiChannel::updateChannelVector() const {

	Logger mout(getImgLog(), "MultiChannel", __FUNCTION__);

	const size_t n = getChannelCount();

	channelVector.resize(n);

	for (size_t k = 0; k < n; ++k) {

		ChannelView & channel = channelVector[k];

		// Test if segment viewed already (lazy init, prevents from re-scaling
		// This way only new ones initialized, to prevent resetting channel specific scalings (and coord policies).
		if (ChannelView(*this,k).hasSameSegment(channel)){
			mout.debug(1) << "channel[" << k << "] already in use, leaving scaling intact " << channel.getScaling() << mout.endl;
		}
		else {
			// links scaling to target image: channel.scalingPtr = this->scaling;
			channel.setView(*this,k);

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
	}

}


}  // image::

}  // drain::
