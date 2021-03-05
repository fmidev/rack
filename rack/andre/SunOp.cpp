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
#include "drain/util/Geo.h"

#include "SunOp.h"

#include "radar/Geometry.h"
#include "radar/Sun.h"


using namespace drain::image;

namespace rack {

//void SunOp::filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const {
void SunOp::processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const {

	drain::Logger mout(__FUNCTION__, __FILE__);
	mout.debug() << "start" << mout.endl;
	// mout.debug() << *this << mout.endl;
	// mout.debug2() << "=>odimIn: " << srcData.odim << mout.endl;

	double sunAzm = 0.0;
	double sunElev = 0.0;
	Sun::getSunPos(srcData.odim.date + srcData.odim.time, srcData.odim.lon*drain::DEG2RAD, srcData.odim.lat*drain::DEG2RAD, sunAzm, sunElev);

	mout.info() << " sunAzm=" << sunAzm << ", sunElev=" << sunElev << mout.endl;
	if (sensitivity > 1.0){
		mout.warn() << " sensitivity=" << sensitivity << ">1.0, using 1.0" << mout.endl;
	}
	const double scale = dstProb.odim.scaleInverse(std::min(sensitivity, 1.0));

	double widthD = beamWidth * drain::DEG2RAD;
	const drain::FuzzyBell<double> fuzzyPeak(0.0, widthD*widthD, scale); //dstProb.odim.getMax());
	dstProb.odim.getMax();

	const size_t width  = dstProb.data.getWidth();
	const size_t height = dstProb.data.getHeight();

	double e = (srcData.odim.getElangleR() - sunElev);
	e = 0.5*e*e; // Elevation difference has smaller effect than azimuthal

	double a;

	for (size_t j = 0; j < height; ++j) {
		a = srcData.odim.getAzimuth(j) - sunAzm;
		for (size_t i = 0; i < width; ++i) {
			dstProb.data.put(i, j, fuzzyPeak(a*a + e));
		}
	}

	//_createQualityField()



}

}  // rack::

// Rack
