/**

    Copyright 2001 - 2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include "PixelVectorOp.h"

namespace drain
{

namespace image
{


void PixelVectorOp::makeCompatible(const ImageFrame &src, Image &dst) const  {

	drain::Logger mout(getImgLog(), name+"(PixelVectorOp)", __FUNCTION__);

	mout.debug(2) << "src:" << src << mout.endl;

	if (!dst.typeIsSet())
		//dst.setType(src.getType());
		dst.setType<unsigned short>();

	dst.setGeometry(src.getWidth(), src.getHeight(), 1);

	mout.debug(3) << "dst:" << dst << mout.endl;

}


}  // image::
}  // drain::


