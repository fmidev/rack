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

#include "image/FilePng.h"
#include "QualityOverrideOp.h"

namespace drain
{

namespace image
{

void QualityOp::makeCompatible(const ImageFrame & src, Image & dst) const  {

	drain::Logger mout(getImgLog(), name+"(ImageOp)", __FUNCTION__);

	mout.debug(2) << "src: " << src << mout.endl;

	if (dst.isSame(src)){
		mout.debug() << "dst == src, ok" << mout.endl;
		return;
	}

	if (!dst.typeIsSet()){
		dst.setType(src.getType());
		// dst.setScale(src.getScale()); // NEW
	}


	if (!Type::call<drain::typeIsInteger>(dst.getType())){
		//dst.scaling.setScale(src.scaling.getScale());
		dst.setScaling(src.getScaling());
		if (dst.getType() != src.getType()){
			mout.info() << "Adopting src scaling for dst (different storage types)" << mout.endl;
		}
	}


	dst.setGeometry(src.getGeometry());
	/*
	/// TODO: copy alpha, fill alpha?
	if (dst.setGeometry(src.getGeometry())){
		if (dst.getAlphaChannelCount()){
			mout.warn() << "resetting alpha channel" << mout.endl;
			dst.getAlphaChannel().fill(dst.scaling.getMax<int>());
		}
	}
	*/

	dst.setCoordinatePolicy(src.getCoordinatePolicy());

	mout.debug(3) << "dst: " << dst << mout.endl;

}


void QualityThresholdOp::traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {

	// quality threshold
	const double t = srcAlpha.getScaling().inv(threshold);

	// Lowest quality, as a code value
	const double zero = dstAlpha.getScaling().inv(0);

	// mout.warn() << " t=" << t << mout.endl;

	Channel::const_iterator  sit = src.begin();
	Channel::const_iterator sait = srcAlpha.begin();
	Channel::iterator  dit = dst.begin();
	Channel::iterator dait = dstAlpha.begin();

	/// Only alpha is thresholded
	if (isnan(replace)){
		while (sit != src.end()){
			if (*sait < t){
				*dit = *sit;
				*dait = zero;
			}
			else { // copy
				*dit  = *sit;
				*dait = *sait; // sometimes need to skip this?
			}
			++sit; ++sait;
			++dit; ++dait;
		}
	}
	/// If alpha is thresholded, data value replaced
	else {
		const double r = src.getScaling().inv(replace);
		while (sit != src.end()){
			if (*sait < t){
				*dit  = r;
				*dait = zero;
			}
			else { // copy
				*dit  = *sit;
				*dait = *sait; // sometimes need to skip this?
			}
			++sit; ++sait;
			++dit; ++dait;
		}
	}
}

}

}
