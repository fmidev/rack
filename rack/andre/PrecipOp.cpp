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

#include "PrecipOp.h"

#include "radar/Geometry.h"
#include "radar/Analysis.h"


using namespace drain::image;


namespace rack {

void PrecipOp::processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << *this << mout.endl;
	mout.debug(1) << "=>srcData.odim: " << srcData.odim << mout.endl;

	/// Descending fuzzy step, located at (max) altitude.
	//  const drain::FuzzyStepsoid<double,float> fuzzyAltitude(maxAltitude, -devAltitude);

	RadarFunctorOp<drain::FuzzyBell<double> > dbzFuzzifier;
	dbzFuzzifier.odimSrc = srcData.odim;
	dbzFuzzifier.functor.set(dbz, dbzSpan);
	dbzFuzzifier.process(srcData.data, dstData.data);

}

}  // rack::
