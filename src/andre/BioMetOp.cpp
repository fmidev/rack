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

#include "BioMetOp.h"

#include "radar/Geometry.h"


using namespace drain::image;


namespace rack {

void BiometOp::runDetector(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	//mout.warn(*this);

	const int width  = srcData.data.getWidth();
	const int height = srcData.data.getHeight();

	// mout.note("=>srcData.odim: ", srcData.odim);
	// mout.note("=>dstData: \n", dstData);

	/// Descending fuzzy step, located at (max) altitude.
	//  const drain::FuzzyStepsoid<double,float> fuzzyAltitude(maxAltitude, -devAltitude);
	const drain::FuzzyStep<double> fuzzyAltitude(maxAltitude + devAltitude, maxAltitude - devAltitude);

	/// Descending fuzzy step, located at max intensity
	//  const drain::FuzzyStepsoid<float,float> fuzzyDBZ(reflMax, -reflDev);
	const drain::FuzzyStep<double> fuzzyDBZ(reflMax + reflDev, reflMax - reflDev);

	double s;
	float c1; // fuzzy altitude
	float c2; // fuzzy reflectivity

	const float eta = srcData.odim.elangle * M_PI/180.0;
	const float dstMAX = dstData.odim.scaleInverse(1.0);
	for (int i = 0; i < width; ++i) {
		c1 = fuzzyAltitude(Geometry::heightFromEtaBeam(eta, srcData.odim.getBinDistance(i)));
		for (int j = 0; j < height; ++j) {
			s = srcData.data.get<double>(i,j);
			if ((s == srcData.odim.undetect) || (s == srcData.odim.nodata))
				continue;
			c2 = fuzzyDBZ(srcData.odim.scaleForward(s));
			dstData.data.put(i, j, c1*c2*dstMAX);
		}
	}


}

}  // rack::

// Rack
