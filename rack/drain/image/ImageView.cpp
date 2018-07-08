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

#include "ImageView.h"

namespace drain {


namespace image {




void ImageView::setView(const ImageFrame & src, const std::string & view) {

	const char v = view.empty() ? '*' : view.at(0);

	switch (v) {
	case 'F':
		ImageFrame::setView(src, 0, src.getChannelCount(), true);
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
		setView(src, src.getChannelIndex(view));
	}
}



}  // image::
}  // drain::
