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
#include "util/Log.h"
#include "CopyOp.h"

namespace drain {

namespace image {

// Consider Image::swap() ?
void CopyOp::makeCompatible(const ImageFrame & src, Image & dst) const {

	Logger mout(getImgLog(), name, __FUNCTION__);

	if (!dst.typeIsSet())
		dst.setType(src.getType());

	if (dst.isEmpty())
		dst.setGeometry(src.getGeometry());

	if (functor.dstView == "a"){
		//mout.warn() << "request dst alpha" << mout.endl;
		dst.setAlphaChannelCount(1);
		dst.getAlphaChannel(0).fill(128);
		//dst.getAlphaChannel(); // create, if nonexistent
	}
	else if ((functor.dstView != "f") && (functor.dstView != "i")){
		//mout.warn() << "request dst view" << functor.dstView << mout.endl;
		const size_t k = dst.getChannelIndex(functor.dstView);
		//mout.warn() << " dst: channels:" << k << mout.endl;
		if (k >= dst.getImageChannelCount())
			dst.setChannelCount(k+1, dst.getAlphaChannelCount());
		mout.debug(3) << " dst now: " << dst << mout.endl;

	}

	ImageScaling s;
	s.adoptScaling(src.getScaling(), src.getType(), dst.getType());
	mout.debug(1) << " src:       " << mout.endl;
	mout.debug(2) << " dst(orig): " << dst << mout.endl;
	//dst.setOptimalScale(s.getMinPhys(), s.getMaxPhys());
	dst.setScaling(s);
	mout.debug(1) << " dst:       " << dst << mout.endl;

}


}  // namespace image

}  // namespace drain
