/*

MIT License

Copyright (c) 2026 FMI Open Development / Markus Peura, first.last@fmi.fi

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

#include "ImageData.h"

namespace drain {

namespace image {


void ImageData::createFrom(const Image & data) {

	drain::Logger mout(__FILE__, __FUNCTION__);

	const std::type_info & type = data.getType();
	const int bits = 8 * drain::Type::call<drain::sizeGetter>(type);
	const int bias = drain::Type::call<drain::typeMin, int>(type);
	mout.attention(DRAIN_LOG(bits), ' ', DRAIN_LOG(bias));

	/*
	drain::image::ImageT<uint8_t> dataImage(data.getWidth(), data.getHeight(), 3);
	drain::image::Channel & red   = dataImage.getChannel(0);
	drain::image::Channel & green = dataImage.getChannel(1);
	*/
	//drain::image::Channel & blue  = dataImage.getChannel(2);
	setGeometry(data.getWidth(), data.getHeight(), 3);
	drain::image::Channel & red   = getChannel(0);
	drain::image::Channel & green = getChannel(1);


	int value;

	drain::image::Channel::iterator rit = red.begin();   // More significant bits
	drain::image::Channel::iterator git = green.begin(); // Less significant bits

	if (bits == 8){
		for (drain::image::Image::const_iterator it=data.begin(); it!=data.end(); ++it){
			value = static_cast<int>(*it) - bias;
			*rit =  value & 0xff;
			*git =  0;
			++rit;
			++git;
		}
	}
	else if (bits == 16){
		for (drain::image::Image::const_iterator it=data.begin(); it!=data.end(); ++it){
			value = static_cast<int>(*it) - bias;
			*rit = (value>>8) & 0xff;
			*git =  value     & 0xff;
			++rit;
			++git;
		}
	}
	else {
		mout.error(__LINE__, "unsupported bit depth: ", bits);
	}
};

}

}
