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
#ifndef Double_SMOOTHEROP
#define Double_SMOOTHEROP "DoubleSmootherOp"

#include "WindowOp.h"

#include "FastAverageOp.h"
#include "BasicOps.h"

namespace drain
{
namespace image
{


/// Smoothes image, and mixes the result with the original image by coeff*100%. See also QuadraticSmootherOp .
/*! This operator produces image
 *  \[
 *  F_2 = (1-c)F + cM{F}
 *  \]
 *  where 
 *  \$F\$ is an image,
 *  \$M\{\}\$ is a \$W\times W\$ FastAverage operator, and
 *  \$c\$ is a coefficient.

\code
	  drainage shapes.png --doubleSmoother 25,25,0.75 -o doubleSmooth.png
\endcode

 */
class DoubleSmootherOp: public WindowOp
{

public:

	DoubleSmootherOp(int width=3, int height=3, double coeff=0.5) :
		WindowOp(__FUNCTION__, "Smoothes image and mixing the result with the original by coeff."){
		this->width  = width;
		this->height = height;
		reference("coeff", this->coeff = coeff);
	};

	virtual
	void filter(const Image & src, Image &dst) const {

		Image tmp;
		FastAverageOp(width,height).filter(src,tmp);
		// MixerOp(coeff).filter(tmp, src, dst);
		BinaryFunctorOp<MixerFunctor> op;
		op.functor.coeff = coeff;
		op.filter(tmp, src, dst);

	}

	double coeff;

protected:

	virtual
	inline
	bool filterWithTmp(const Image & src, Image & dst) const {
		return false;
	};

	/*
	 *   If temporary images are never needed, this function should simply return \c false.
	 */
	virtual
	inline
	bool filterWithTmp(const Image & src, const Image & src2, Image & dst) const {
		return false;
	};

};




} // namespace drain

} //namespace image


#endif

// Drain
