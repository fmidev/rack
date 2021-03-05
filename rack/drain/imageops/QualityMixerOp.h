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
#ifndef QUALITYMixerOP_H_
#define QUALITYMixerOP_H_


#include "ImageOp.h"

namespace drain
{

namespace image
{

/// Overwrites pixels of destination image in locations where destination weight is lower.
/**

 \~exec
   make image-rgba.png     #exec
   make image-rot-rgba.png #exec
 \~

 \code
   drainage image-rgba.png image-rot-rgba.png --iQualityMixer '0.20' -o qualityOverride20.png
   drainage image-rgba.png image-rot-rgba.png --iQualityMixer '0.50' -o qualityOverride50.png
   drainage image-rgba.png image-rot-rgba.png --iQualityMixer '0.80' -o qualityOverride80.png
 \endcode
 */
class QualityMixerOp : public ImageOp
{

public:

	QualityMixerOp(double couff = 0.5): ImageOp(__FUNCTION__, "Weighted blending of two images."){
		parameters.link("coeff", this->coeff = coeff);
	};


	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;

	virtual
	void traverseChannels(const ImageTray<const Channel> & src1, const ImageTray<const Channel> & src2, ImageTray<Channel> & dst) const
	{
		Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		mout.fatal() << "not implemented" << mout.endl;
	}
	
	double coeff;

};



}

}

#endif /*QUALITYMixer_H_*/

// Drain
