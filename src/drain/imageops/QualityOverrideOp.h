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
  # rainage image-rot.png --iCopy f --view r --iCopy a --view f -o image-rot-rgba.png
  make image-rot-rgba.png # exec
\~

 \code
   drainage image-rgba.png image-rot-rgba.png  --iQualityOverride '0.99' -o qualityOverride.png
 \endcode

\~exec
  drainage image-rgba.png      --view F --iResize 240,560 -o qualityOverride-s1.png
  drainage image-rot-rgba.png  --view F --iResize 240,560 -o qualityOverride-s2.png
  drainage qualityOverride.png --view F --iResize 240,560 -o qualityOverride-d.png
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
		parameters.link("advantage", this->advantage = advantage, "0.8..1.2");
		parameters.link("decay", this->decay = decay, "0...1");
	};

	/*
	inline
	QualityOverrideOp(const QualityOverrideOp & op) : QualityOp(op){
		parameters.copyStruct(op.getParameters(), op, *this);
	};
	*/

	virtual
	~QualityOverrideOp(){};

	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;

	virtual
	void traverseChannels(const ImageTray<const Channel> & src1, const ImageTray<const Channel> & src2, ImageTray<Channel> & dst) const;

	// Inapplicable for separate images.
	virtual	inline
	void traverseChannel(const Channel &src, Channel &dst) const {
		Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		mout.error() << "Inapplicable for single channels, needs alpha channels" << mout.endl;
	}

	/// fwd as trays
	virtual	inline
	void traverseChannel(const Channel &src, const Channel &srcWeight, Channel &dst, Channel &dstWeight) const {
		Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		mout.note() << "fwd as trays" << mout.endl;
		traverseAsChannelTrays(src, srcWeight, dst, dstWeight);
	}
	

	quality_t advantage;
	quality_t decay;

};



}

}

#endif /*QUALITYOVERRIDE_H_*/

// Drain
