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

#include "drain/util/Fuzzy.h"

#include "PrecipOp.h"

#include "radar/Geometry.h"
#include "radar/Analysis.h"


using namespace drain::image;


namespace rack {

void PrecipOp::runDetector(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.debug() << *this << mout.endl;
	mout.debug2() << "=>srcData.odim: " << srcData.odim << mout.endl;

	/// Descending fuzzy step, located at (max) altitude.
	//  const drain::FuzzyStepsoid<double,float> fuzzyAltitude(maxAltitude, -devAltitude);

	RadarFunctorOp<drain::FuzzyBell<double> > dbzFuzzifier;
	dbzFuzzifier.odimSrc = srcData.odim;
	dbzFuzzifier.functor.set(dbz, dbzSpan);
	dbzFuzzifier.process(srcData.data, dstData.data);

}

// TODO: join with default quality
void DefaultOp::runDetector(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug() << *this << mout.endl;
	mout.debug2() << " => srcData.odim: " << EncodingODIM(srcData.odim) << mout.endl;
	mout.debug2() << " => dstData.odim: " << EncodingODIM(dstData.odim) << mout.endl;

	mout.debug2() << " => dst: " << dstData.data.getScaling() << mout.endl;
	//const int code = AndreOp::getClassCode(this->classCode);

	RadarFunctorOp<DataMarker> marker;
	marker.odimSrc = srcData.odim;

	// const PolarODIM & odim = dstData.odim;
	// const double probCode = dstData.odim.scaleInverse(1.0 - QualityCombinerOp::DEFAULT_QUALITY) + 2;
	// mout.warn() << "Using prob (code value) " << probCode << mout.endl;
	// marker.functor.set(dstData.odim.scaleForward(probCode));
	// const double pCode    = dstData.odim.scaleInverse(this->probability);
	// const double pCodeMin = dstData.odim.scaleInverse(1.0 - this->qualityThreshold); // QualityCombinerOp::DEFAULT_QUALITY);

	/*
	if (this->probability <= (1.0 - this->qualityThreshold)){
		mout.warn() << "prob " << this->probability << " [" << pCode << "] of 'default class' ";
		mout        << "does not exceed limit (1 - aDefaultQuality) = " << (1.0 - this->qualityThreshold) << " [" << pCodeMin << ']' << mout.endl;
	}
	*/

	marker.functor.set(this->probability);
	// mout.warn() << marker << mout.endl;
	marker.process(srcData.data, dstData.data);
	mout.debug2() << " => DST: " << dstData.data.getScaling() << mout.endl;

}


}  // rack::

// Rack
