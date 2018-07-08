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
#ifndef QUALITYOVERRIDEOP_H_
#define QUALITYOVERRIDEOP_H_


//#include "ImageOp.h"
#include "QualityOp.h"

namespace drain
{

namespace image
{


/// Overwrites pixels of destination image in locations where destination weight is lower.
/**

\~exec
  # onvert  image.png -rotate 180 image-rot.png
  # rainage image-rot.png --copy f --view r --copy a --view f -o image-rot-rgba.png
  make image-rot-rgba.png
\~

 \code
   drainage image-rgba.png image-rot-rgba.png  --qualityOverride '0.99' -o qualityOverride.png
 \endcode

\~exec
  drainage image-rgba.png      --view F --resize 240,560 -o qualityOverride-s1.png
  drainage image-rot-rgba.png  --view F --resize 240,560 -o qualityOverride-s2.png
  drainage qualityOverride.png --view F --resize 240,560 -o qualityOverride-d.png
  convert -frame 1 +append qualityOverride-{s1,s2,d}.png -resize 560x560 qualityOverride-panel.png
\~

 \see QualityMixerOp
 \see BlenderOp

 */
class QualityOverrideOp : public QualityOp
{

public:

	typedef float quality_t;

	/**
	 *  \param advantage - coefficient by which src alpha is multiplied before comparison with dst alpha.
	 *  \param decay     - coefficient by which final (winning) alpha is multiplied in dst data.
	 *
	 */
	QualityOverrideOp(quality_t advantage = 1.0, quality_t decay = 1.0): QualityOp(__FUNCTION__,
			"Compares two images, preserving pixels having higher alpha value. Src alpha is pre-multiplied with advantage."){
		parameters.reference("advantage", this->advantage = advantage, "0.8..1.2");
		parameters.reference("decay", this->decay = decay, "0...1");
	};

	virtual
	~QualityOverrideOp(){};

	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;

	virtual
	void traverseChannels(const ImageTray<const Channel> & src1, const ImageTray<const Channel> & src2, ImageTray<Channel> & dst) const;

	// Inapplicable for separate images.
	virtual	inline
	void traverseChannel(const Channel &src, Channel &dst) const {
		Logger mout(getImgLog(), name, __FUNCTION__);
		mout.fatal() << "Inapplicable for single channels, needs alpha channels" << mout.endl;
	}

	/// fwd as trays
	virtual	inline
	void traverseChannel(const Channel &src, const Channel &srcWeight, Channel &dst, Channel &dstWeight) const {
		Logger mout(getImgLog(), name, __FUNCTION__);
		mout.note() << "fwd as trays" << mout.endl;
		traverseAsChannelTrays(src, srcWeight, dst, dstWeight);
	}
	

	quality_t advantage;
	quality_t decay;

};



}

}

#endif /*QUALITYOVERRIDE_H_*/
