/*

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

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
#ifndef CAPPI2_OP_H_
#define CAPPI2_OP_H_

//#include <algorithm>



//#include "RackOp.h"
#include "CumulativeProductOp.h"

namespace rack
{

using namespace drain::image;


/// Pseudo-CAPPI: the constant altitude planar position indicator product
/**
 */
class CappiOp : public CumulativeProductOp {

public:

	CappiOp(double altitude=1500.0, double weightMin = 0.01, double weightExponent=8.0, bool aboveSeaLevel=true) :
		CumulativeProductOp(__FUNCTION__, "Constant-altitude planar position indicator", "WAVG,1,8,-40")
		{

		parameters.reference("altitude", this->altitude = altitude, "metres");
		parameters.reference("weightMin", this->weightMin = weightMin, "scalar");
		parameters.reference("weightExponent", this->weightExponent = weightExponent, "scalar");
		parameters.reference("aboveSeaLevel", this->aboveSeaLevel = aboveSeaLevel, "0|1");

		odim.product = "PCAPPI";
		dataSelector.quantity = "^DBZH$";

	};

	double altitude;
	double weightExponent;
	double weightMin;

	void processData(const Data<PolarSrc> & data, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const;

protected:


};





}  // ::rack

#endif /*POLARMax_H_*/
