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
#ifndef QUADRATIC_SMOOTHEROP
#define QUADRATIC_SMOOTHEROP "QuadraticSmootherOp Markus.Peura@iki.fi"

#include "WindowOp.h"

#include "FastAverageOp.h"
#include "BasicOps.h"
//#include "SlidingStripeAverageOp.h"

namespace drain
{
namespace image
{


/// Smoothes image and mixes the result with the original by coeff*100%. See also QuadraticSmootherOp .
/*! This operator produces
   \[
   F2 = (1-c)F + cM\{ (1-c)F + cM\{F\} \}
      = (1-c)F + (1-c)cM{F} + c^2M^2{F}
   \]
   where
   \$F\$ is an image,
   \$M\{\}\$ is a \$W x W\$ FastAverage operator, and
   \$c\$ is the mixing coefficient between 0.0 and 1.0.

   \code
   	  drainage shapes.png --quadraticSmoother 25,25,0.75 -o quadSmooth.png
   \endcode
 */
class QuadraticSmootherOp: public WindowOp
{

public:

	QuadraticSmootherOp(int width=5, int height=5, double coeff=0.5) :
		WindowOp("QuadraticSmootherOp", "Smoothes image twice, mixing original image with coeff*100%."){
		this->width = width;
		this->height = height;
		parameters.reference("coeff", this->coeff = coeff);
	};

	virtual
	inline
	void filter(const Image & src, Image & dst) const {

		FastAverageOp avg(width,height);
		BinaryFunctorOp<MixerFunctor> mix;
		mix.functor.coeff = 1.0-coeff;
		//MixerOp mix(1.0-coeff);
		Image tmp;

		avg.filter(src, tmp);
		//mix.filter(tmp, src, tmp);
		mix.filter(src, tmp);

		avg.filter(tmp, dst);
		mix.filter(src, dst);
		// mix.filter(dst, src, dst);

	}

protected:

	double coeff;

};




} // namespace drain

} //namespace image


#endif

// Drain
