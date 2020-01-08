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

#include <drain/util/Fuzzy.h>

#include "AttenuationOp.h"

//#include "radar/Geometry.h"
//#include "radar/Analysis.h"
#include "data/DataCoder.h"


using namespace drain::image;


namespace rack {

void AttenuationOp::processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << *this << mout.endl;
	mout.debug(1) << "=>srcData.odim: " << srcData.odim << mout.endl;

	mout.debug() << *this << mout.endl;
	mout.debug(1) << " => srcData.odim: " << EncodingODIM(srcData.odim) << mout.endl;
	mout.debug(1) << " => dstData.odim: " << EncodingODIM(dstData.odim) << mout.endl;

	mout.debug(1) << " => dst: " << dstData.data.getScaling() << mout.endl;
	//const int code = AndreOp::getClassCode(this->classCode);
	const size_t width  = srcData.data.getWidth();
	const size_t height = srcData.data.getHeight();

	// Attn coeff
	double c = 1.12E-7;
	double p = 0.62;

	double dbzObs;
	double zTrue;

	double zAttnBin;
	double zAttnCumulated;

	const drain::FuzzyBell<double> fuzzifier(0.0, reflHalfWidth);
	double quality; // yes, not probability

	const size_t i0 = srcData.odim.getBinIndex(5000.0);

	for (size_t j = 0; j < height; ++j) {

		zAttnCumulated = 0.0;
		quality = 1.0;

		for (size_t i = 0; i < i0; ++i) {
			dstData.data.putScaled(i, j, 0);
		}
		for (size_t i = i0; i < width; ++i) {

			dbzObs = srcData.data.get<double>(i,j);


			if (srcData.odim.isValue(dbzObs)){

				dbzObs =  srcData.odim.scaleForward(dbzObs);

				/// Actual Z derived from the actual dBZ,
				zTrue = rack::dbzToZ(dbzObs) + zAttnCumulated;

				/// one-way attenuation per one bin
				/// two-way attenuation per one bin
				// TODO: bin width should be in the exponent?
				zAttnBin = (c*::pow(zTrue,p) + c2*::pow(zTrue,p2)) * srcData.odim.rscale * 2.0;
				zAttnCumulated += zAttnBin;

				quality = fuzzifier(zAttnCumulated);

				dstData.data.putScaled(i, j, 1.0 - quality);
			}
			else {
				dstData.data.putScaled(i, j, 0.9*(1.0 - quality)); // default
				//dstData.data.putScaled(i, j, 1.0 - DataCoder::undetectQualityCoeff*quality); // default
			}
			//dstData.data.putScaled(i, j, 1.0 - quality);

		}
		// mout.warn() << marker << mout.endl;

		// marker.process(srcData.data, dstData.data);
		mout.debug(1) << " => DST: " << dstData.data.getScaling() << mout.endl;

	}
}

}  // rack::
