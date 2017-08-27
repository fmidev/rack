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

/** Overwrites pixels of destination image in locations where destination weight is lower.
 \code
   DDrainage image-rgba.png sample-rgba2.png --qualityMixer '1' -o qualityMixer.png
 \endcode
 */
class QualityMixerOp : public ImageOp
{

public:

	QualityMixerOp(double weight = 0.5): ImageOp("QualityMixerOp",
			"Weighted blending of two images."){
		parameters.reference("weight", this->weight = weight);
	};

	virtual inline
	void filter(const Image &src,Image &dst) const {
		filterUsingAlphas(src, dst);
	};

	virtual inline
	void filter(const Image &src,const Image &srcWeight, Image &dst,Image &dstWeight) const {
		filter(src, srcWeight, dst, dstWeight, dst, dstWeight);
	}

	/// New policy?
	virtual void filter(const Image &src, const Image &srcWeight, const Image &src2, const Image &src2Weight,
		Image &dst, Image &dstWeight) const ;

	
	double weight;

};



}

}

#endif /*QUALITYMixer_H_*/

// Drain
