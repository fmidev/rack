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

#include "Image.h"

namespace drain {


namespace image {



/// Prints images geometry, buffer size and type information,
//  ///// and dumps the array contents.
void Image::dump(std::ostream & ostr) const {
	toOStr(ostr);
	ostr << "begin=" << (long)begin() << '\n';
	ostr << "end=  " << (long)end() << '\n';
}

void Image::swap(Image & img){

	const std::type_info & type = getType();
	const Geometry geometry(getGeometry());
	const ImageScaling scaling(getScaling());

	buffer.swap(img.buffer);

	initialize(img.getType(), img.getGeometry());
	setScaling(img.getScaling());

	img.initialize(type, geometry);
	img.setScaling(scaling);

	img.properties.swap(properties);

}


}  // image::
}  // drain::
