/**

    Copyright 2001-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

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
