/*

MIT License

Copyright (c) 2025 FMI Open Development / Markus Peura, first.last@fmi.fi

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



//#include <drain/util/Fuzzy.h>
#include <drain/util/FunctorBank.h>
//#include "data/DataCoder.h"
#include "AltitudeOp.h"



namespace rack {

void AltitudeOp::runDetector(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData) const {


	drain::Logger mout(__FILE__, __FUNCTION__);

	/*
	mout.debug(*this );
	mout.debug2("=>srcData.odim: " , srcData.odim );

	mout.debug(*this );
	mout.debug2(" => srcData.odim: " , EncodingODIM(srcData.odim) );
	mout.debug2(" => dstData.odim: " , EncodingODIM(dstData.odim) );

	mout.debug2(" => dst: " , dstData.data.getScaling() );
	//const int code = AndreOp::getClassCode(this->classCode);

	// Attn coeff
	//double c = 1.12E-7;
	//double p = 0.62;
	*/

	const size_t width  = srcData.data.getWidth();
	const size_t height = srcData.data.getHeight();

	//const drain::FunctorBank & functorBank = drain::getFunctorBank();
	//const drain::FuzzyBell<double> fuzzifier(0.0, reflHalfWidth);
	drain::UnaryFunctor & ftor = drain::getFunctor(this->functor);

	mout.accept<LOG_ERR>("Ok, obtained functor:", ftor);

	double quality = 0; // yes, not probability
	double obs = 0;
	const size_t i0 = srcData.odim.getBinIndex(5000.0);

	for (size_t j = 0; j < height; ++j) {

		for (size_t i = i0; i < width; ++i) {

			if (srcData.odim.isValue(obs)){

			}
			else {
				dstData.data.putScaled(i, j, 0.75*(1.0 - quality)); // default
				//dstData.data.putScaled(i, j, 1.0 - DataCoder::undetectQualityCoeff*quality); // default
			}
			//dstData.data.putScaled(i, j, 1.0 - quality);

		}
		// mout.warn(marker );

		// marker.process(srcData.data, dstData.data);
		mout.debug2(" => DST: " , dstData.data.getScaling() );

	}
}

}  // rack::
