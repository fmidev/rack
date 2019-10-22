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
#include <drain/util/Geo.h>

#include "SunOp.h"

#include "radar/Geometry.h"
#include "radar/Sun.h"


using namespace drain::image;

namespace rack {

//void SunOp::filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const {
void SunOp::processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << "start" << mout.endl;
	// mout.debug() << *this << mout.endl;
	// mout.debug(1) << "=>odimIn: " << srcData.odim << mout.endl;

	double sunAzm = 0.0, sunElev = 0.0;
	Sun::getSunPos(srcData.odim.date + srcData.odim.time, srcData.odim.lon*drain::DEG2RAD, srcData.odim.lat*drain::DEG2RAD, sunAzm, sunElev);

	mout.info() << " sunAzm=" << sunAzm << ", sunElev=" << sunElev << mout.endl;

	const drain::FuzzyBell<double> fuzzyPeak(0.0, (width*width)*(drain::DEG2RAD*drain::DEG2RAD), 250.0);

	const size_t w = dstProb.data.getWidth();
	const size_t h = dstProb.data.getHeight();
	double a, e;

	e = (srcData.odim.getElangleR() - sunElev) ;
	e = e*e;

	unsigned char c;
	const float hf = static_cast<float>(h);
	const double M2PI = 2.0 * M_PI;
	for (size_t j = 0; j < h; ++j) {
		a = (M2PI * static_cast<double>(j) / hf) - sunAzm;
		a = a*a;
		c = sensitivity * fuzzyPeak(a + e);
		for (size_t i = 0; i < w; ++i) {
			dstProb.data.put(i, j, c);
		}
	}

	//_createQualityField()


	/*
	const drain::LinearScaling scaleDBZ(srcData.odim.gain, srcData.odim.offset);
	mout.debug(1) << "scaleDBZ: " << scaleDBZ << mout.endl;


	/// Descending fuzzy step, located at (max) altitude.
	const drain::FuzzyStep<double,float> fuzzyAltitude(maxAltitude, -devAltitude);

	/// Descending fuzzy step, located at max intensity
	const drain::FuzzyStep<float,float> fuzzyDBZ(reflMax, -reflDev);

	float s;
	float c1; // fuzzy altitude
	float c2; // fuzzy reflectivity
	//const float maxD = dstProb.data.getMax<float>()-1.0f;  TODO gain,
	const float maxD = 253.0;
	const float eta = srcData.odim.elangle * M_PI/180.0;
	for (int i = 0; i < width; ++i) {
		fuzzyAltitude.filter(Geometry::heightFromEtaBeam(eta,i*srcData.odim.rscale),c1);
		//std::cerr << i << '\t' << scaleDBZ.forward(i) << '\t' << (scaleDBZ.forward(i)*2.0 + 64.0) << std::endl;
		for (int j = 0; j < height; ++j) {
			s = src.get<float>(i,j);
			if ((s == srcData.odim.undetect) || (s == srcData.odim.nodata))
				continue;
			fuzzyDBZ.filter(scaleDBZ.forward(s), c2);
			dstProb.data.put(i, j, maxD*c1*c2);
			//dstProb.data.put(i,j,scaleDBZ.forward(s)*2.0 + 64.0);  // = 0.5, -32
		}
	}

  */

}

}  // rack::

// Rack
