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
#ifndef HIGHBOOSTOP_H_
#define HIGHBOOSTOP_H_

//#include "SlidingWindowAverage.h"
#include "ImageOp.h"

#include "FastAverageOp.h"
#include "util/FunctorPack.h"


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
		parameters.reference("width",  this->width = width);
		parameters.reference("height", this->height = height);
		parameters.reference("coeff",  this->coeff = coeff);
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
