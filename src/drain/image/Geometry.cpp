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

#include <algorithm>
#include <sstream>

#include "Geometry.h"
//#include "DistanceTransformOp.h"

namespace drain
{

namespace image
{


size_t Geometry::getChannelIndex(const std::string & index) const {

	// consider: conv to lower case

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);


	if (index.empty()){
		mout.warn() << "index str empty, returning 0" << mout.endl;
		return 0;
	}

	size_t i = 0;

	switch (index.at(0)) {
	case 'r':  // red
		i = 0;
		break;
	case 'g':  // green
		i =  1;
		break;
	case 'b':  // blue
		i =  2;
		break;
	case 'a':  // alpha
		i =  getImageChannelCount();
		break;
	default:
		/// Number
		std::stringstream sstr(index);
		sstr >> i;
		if ((i == 0) && (index != "0"))
			throw std::range_error(index + "<-- Image::getChannelIndex: unknown channel symbol");
	}

	if (i >= getChannelCount()){
		mout.warn() << "index " << i << " larger than channelCount " << getChannelCount() << mout.endl;
	}

	return i;

}



}

}

// Drain
