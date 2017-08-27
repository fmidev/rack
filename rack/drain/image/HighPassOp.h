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
#ifndef HIGHPASSOP_H_
#define HIGHPASSOP_H_

//#include "SlidingWindowAverage.h"
#include "ImageOp.h"

#include "FastAverageOp.h"
#include "BasicOps.h"
#include "File.h"


namespace drain
{

namespace image
{

/// Finds details at one-pixel scale, defined as difference of image and its low-pass filtered copy.
/**
 *   Implements
 *
 *   The simplest square high-pass filter is the 3x3 filter
 *   -1  -1  -1
 *   -1   8  -1
 *   -1  -1  -1 
 * 
 *   The result can be scaled with "scale" and transposed with "offset".
 *   Internally, applies the fast SlidingStripeAverage and SubstractOp .
 *
 \code
   drainage gray.png --highPass '5,5,10.0' -o highPass.png
 \endcode
 */
class HighPassOp : public ImageOp
{
public:

	HighPassOp(int width=1, int height=1, double scale = 1.0, double offset = 0.0, bool LIMIT=true) :
		ImageOp(__FUNCTION__,"High-pass filter for recognizing details."){
		parameters.reference("width", this->width = width);
		parameters.reference("height",this->height = height>0 ? height : width);
		parameters.reference("scale", this->scale = scale);
		this->parameters.reference("offset", this->offset = offset);
		this->parameters.reference("LIMIT", this->LIMIT=LIMIT);
	};
    
	virtual
	inline
	void filter(const Image &src, Image &dst) const {

		MonitorSource mout(iMonitor, name, __FUNCTION__);

		const bool USE_TMP = src.hasOverlap(dst);

		Image tmp;
		Image & src2 = USE_TMP ? tmp : dst;




		mout.debug(3) << "fast average, use tmp=" << (int)USE_TMP << mout.endl;
		//File::write(src,"HighPass-01-src.png");
		//mout.warn() << "src: " << src << mout.endl;
		FastAverageOp(width, height).filter(src, src2);
		//mout.warn() << "src2: " << src2 << mout.endl;


		//File::write(src2,"HighPass-02-favg.png");
		//File::write(dst,"HighPass-02-favg.png");
		
		mout.debug(3) << "subtraction" << mout.endl;
		//src2.setScale(src.getScale());
		//dst.setScale(src.getScale());
		BinaryFunctorOp<SubtractionFunctor> op;
		op.functor.setScale(scale, offset);
		op.LIMIT = LIMIT;
		op.filter(src, src2, dst);
		// SubtractionOp(scale, offset, LIMIT).filter(src, src2, dst);
		//mout.warn() << "dst: " << dst << mout.endl;
		
	};

	virtual
	inline
	void traverse(const Image &src, Image &dst) const {

		MonitorSource mout(iMonitor, name, __FUNCTION__);
		mout.warn() << "delegating up, to filter(src, dst)" << mout.endl;
		filter(src, dst);
	}

	int width;
	int height;
	double scale;
	double offset;
	bool LIMIT;


	virtual
	inline
	bool filterWithTmp(const Image & src, Image & dst) const {return false;} ;

	/*
	 */
	virtual
	inline
	bool filterWithTmp(const Image & src, const Image & src2, Image & dst) const {return false;};

};


}

}

#endif /*HIGHPASSOP_H_*/

// Drain
