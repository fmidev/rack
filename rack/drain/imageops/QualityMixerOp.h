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
#ifndef QUALITYMixerOP_H_
#define QUALITYMixerOP_H_


#include "ImageOp.h"

namespace drain
{

namespace image
{

/** Overwrites pixels of destination image in locations where destination weight is lower.
 \code
   drainage image-rgba.png image-rot-rgba.png --qualityMixer '0.20' -o qualityOverride20.png
   drainage image-rgba.png image-rot-rgba.png --qualityMixer '0.50' -o qualityOverride50.png
   drainage image-rgba.png image-rot-rgba.png --qualityMixer '0.80' -o qualityOverride80.png
 \endcode
 */
class QualityMixerOp : public ImageOp
{

public:

	QualityMixerOp(double couff = 0.5): ImageOp(__FUNCTION__, "Weighted blending of two images."){
		parameters.reference("coeff", this->coeff = coeff);
	};


	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;

	virtual
	void traverseChannels(const ImageTray<const Channel> & src1, const ImageTray<const Channel> & src2, ImageTray<Channel> & dst) const
	{
		Logger mout(getImgLog(), name, __FUNCTION__);
		mout.fatal() << "not implemented" << mout.endl;
	}
	
	double coeff;

};



}

}

#endif /*QUALITYMixer_H_*/
