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

#include "BioMetOp.h"

#include "radar/Geometry.h"


using namespace drain::image;


namespace rack {

void BiometOp::processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << *this << mout.endl;

	const int width  = srcData.data.getWidth();
	const int height = srcData.data.getHeight();

	mout.debug(1) << "=>srcData.odim: " << srcData.odim << mout.endl;

	/// Descending fuzzy step, located at (max) altitude.
	//const drain::FuzzyStepsoid<double,float> fuzzyAltitude(maxAltitude, -devAltitude);
	const drain::FuzzyStep<double> fuzzyAltitude(maxAltitude + devAltitude, maxAltitude - devAltitude);

	/// Descending fuzzy step, located at max intensity
	//const drain::FuzzyStepsoid<float,float> fuzzyDBZ(reflMax, -reflDev);
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
