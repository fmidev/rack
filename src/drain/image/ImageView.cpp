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

#include "ImageView.h"

namespace drain {

namespace image {


void ImageView::setView(const ImageFrame & src, const std::string & view) {

	const char v = view.empty() ? '*' : view.at(0);
	channelVector.clear();

	switch (v) {
	case 'F':
		ImageFrame::setView(src, 0, src.getChannelCount(), true); // Concatenate vertically
		break;
	case '*': // 2017
		// no break
	case 'f':
		setView(src);
		break;
	case 'i':  // image channels (excluding alpha)
		ImageFrame::setView(src, 0, src.getImageChannelCount());
		break;
	default:
		setView(src, src.getGeometry().getChannelIndex(view));
	}
	//updateChannelVector();
}



}  // image::
}  // drain::

// Drain
