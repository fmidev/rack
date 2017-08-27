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
#ifndef GAMMAOP_H_
#define GAMMAOP_H_

#include <math.h>

//#include <limits>

//#include "BufferedImage.h"
#include "SequentialImageOp.h"

// using namespace std;

namespace drain
{

namespace image
{

/// Gamma correction. Intensity is mapped as f' = f^(gamma)
/*!
   Adjusts the brightness such that intensities remain monotonously inside the original scale.
   Unline in direct linear scaling neither undeflow nor overflow occurs and details remain
   detectable down to applied bit resolution.

   \code
   drainage gray.png --gamma 2.0 -o gamma-bright.png
   drainage gray.png --gamma 0.5 -o gamma-dark.png
   \endcode


   \code
   drainage color.png --gamma 2.0 -o gamma-color-bright.png
   drainage color.png --gamma 0.5 -o gamma-color-dark.png
   \endcode

 *  NOTE. Design for parameters may vary in future, since multichannel image could be handled by giving
 *  a value for each: 1.2,1.4,0.7 for example. 
 */
class GammaOp : public SequentialImageOp
{
public:

	GammaOp(double gamma = 1.0) : SequentialImageOp(__FUNCTION__, "Gamma correction for brightness."){
		parameters.reference("gamma", this->gamma = gamma, "positive scalar");
	};

	inline
	void initializeParameters(const Image &src, const Image &src2, const Image &dst) const {
		gammaInv = 1.0 / gamma;
		scaleSrc = 1.0 / src.getMax<float>();
		scaleDst = dst.getMax<float>();
	};

	inline
	double filterValueD(double src, double src2) const {
		return scaleDst * pow(src*scaleSrc, gammaInv);
	};

	double gamma;

protected:

	mutable double gammaInv;
	mutable double scaleSrc;
	mutable double scaleDst;

};

}
}

#endif /*GAMMAOP_H_*/

// Drain
