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
#ifndef HIGHBOOSTOP_H_
#define HIGHBOOSTOP_H_

//#include "SlidingWindowAverage.h"
#include "ImageOp.h"

#include "FastAverageOp.h"
#include "drain/util/FunctorPack.h"


namespace drain
{

namespace image
{


// Returns src - scale*src2
//template <class T=unsigned char,class T2=unsigned char>
/*
class ScaledSubtractionOp: public MathOp
{
public:

   ScaledSubtractionOp() : MathOp(__FUNCTION__, "An arithmetic operator.") {
	   //setParameters(p);
   };

   //		dst = MathOp<T,T2>::limit(static_cast<double>(src) - this->scale*src2 + this->bias);


   inline
   double filterValueD(double src, double src2) const {
	   return (src - scaleD*src2 + offsetD);
   };

};
*/

/// Enhances details by mixing original image and result of HighPassOp op.
/**
 */
class HighBoostOp : public ImageOp
{
public:

	HighBoostOp(int width=3, int height=3, double coeff=0.5) : ImageOp(__FUNCTION__, "Mixture of original and high-pass filtered image") {
		parameters.link("width",  this->width = width);
		parameters.link("height", this->height = height);
		parameters.link("coeff",  this->coeff = coeff);
	};
    
	virtual
	void process(const ImageFrame &src, Image &dst) const {
		
		makeCompatible(src,dst);

		FastAverageOp(width,height).process(src,dst);
		
		BinaryFunctorOp<SubtractionFunctor> sub;
		sub.LIMIT = true;
		sub.functor.setScale(coeff);
		sub.process(src, dst);
		//sub.traverseFrame(src,dst,dst);

		//ScaledSubtractionOp(coeff).process(src,dst,dst);
	};

	int  width;
	int height;
	double coeff;

};


}

}

#endif /*HIGHPASSOP_H_*/

// Drain
