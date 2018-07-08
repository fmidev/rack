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
#ifndef MAX_ECHO_OP2_H_
#define MAX_ECHO_OP2_H_

#include <algorithm>



//#include "RackOp.h"
#include "CumulativeProductOp.h"

namespace rack
{

using namespace drain::image;


/// The maximum dBZ value in each a vertical column.
/** The main idea is to compute one equidistant arc at a time.

 */
//
// TODO: selecting certain elevations (allowed range)
class MaxEchoOp : public CumulativeProductOp {

public:

	/// Experimental parameter handling

	/// Todo: rscale*,nbins*,nrays*,beamwidth*
	MaxEchoOp(double altitude=1000.0, double devAltitude=500.0, const std::string & accumulationMethod = "MAXIMUM"): //"WAVG,8,1,-32") :
		CumulativeProductOp("MaxEcho","Computes...", accumulationMethod) //"WAVG,2,2,-32") "MAXW") //
		{

		parameters.reference("altitude", this->altitude = altitude, "metres");
		parameters.reference("devAltitude", this->devAltitude = devAltitude, "metres");
		parameters.reference("accumulationMethod", this->accumulationMethod = accumulationMethod, "MAXIMUM|AVERAGE|WAVG:2:2|MAXW");
		//dataSelector.path = "^.*/data[0-9]+/?$";  ///

		dataSelector.quantity = "^DBZH$";

		odim.product = "MAX";

	};



	void processData(const Data<PolarSrc> & data, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const;
	// void filter(const HI5TREE &src, const std::map<double,std::string> & srcPaths, HI5TREE &dst) const;

	double altitude;
	double devAltitude;



};





}  // ::rack

#endif /*POLARMax_H_*/
