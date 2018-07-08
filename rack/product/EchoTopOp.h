/*

    Copyright 2014  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#ifndef ECHOTOP2_OP_H_
#define ECHOTOP2_OP_H_

//#include <algorithm>
//#include "RackOp.h"
#include "CumulativeProductOp.h"

namespace rack
{

using namespace drain::image;


/// Height of echo exceeding a reflectivity threshold.
class EchoTopOp : public CumulativeProductOp {

public:

	EchoTopOp(bool top = true, double minDBZ = 20.0, double dbzRef = -50.0, double hRef = 15000.0, bool aboveSeaLevel=true) : //, const std::string & type="C", double gain=0.01, long int nbins=0, double rscale=0) :
		CumulativeProductOp(__FUNCTION__, "Computes maximum (or minimum) altitude of echo.", "WAVG,8,1"), top(top) {

		parameters.reference("minDBZ", this->minDBZ = minDBZ, "dBZ");
		parameters.reference("dbzCoord", this->dbzRef = dbzRef, "dBZ");
		parameters.reference("altitudeCoord", this->hRef = hRef, "metres");
		parameters.reference("altitudeReference", this->aboveSeaLevel = aboveSeaLevel, "0=radar site|1=sea level");

		odim.product = "ETOP";
		odim.quantity = "HGHT";
		odim.type = "C";

		dataSelector.quantity = "^DBZH$";
		dataSelector.path = ".*/data[0-9]+/?$";  // groups

	};


	double minDBZ;
	double dbzRef; // = -50.0;
	double hRef; // = 15000.0;

	//bool aboveSeaLevel;

	const bool top;



protected:

	virtual
	void updateODIM(PolarODIM & odim) const {
		// odim.prodpar = altitude // FAILS because prodpar is an std::string.
		odim["what:prodpar"] = minDBZ;
	};

	//void processData(const Data<PolarSrc> & data, PolarAccumulator & accumulator) const;
	void processData(const Data<PolarSrc> & data, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const;

};





}  // ::rack

#endif /*POLARMax_H_*/
