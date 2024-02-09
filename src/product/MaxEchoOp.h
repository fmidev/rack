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
	//MaxEchoOp(double altitude=1000.0, double devAltitude=500.0, const std::string & accumulationMethod = "MAXIMUM"): //"WAVG,8,1,-32") :
	MaxEchoOp(const drain::Range<double> & altitude = {1000.0,9000.0}, const std::string & accumulationMethod = "MAXIMUM"): //"WAVG,8,1,-32") :
		CumulativeProductOp("MaxEcho","Computes maximum dBZ inside altitude range", accumulationMethod) //"WAVG,2,2,-32") "MAXW") //
		{

		parameters.link("altitude", this->altitude.tuple() = altitude, "metres").fillArray = true;
		// parameters.link("altitude", this->altitude = altitude, "metres");
		// parameters.link("devAltitude", this->devAltitude = devAltitude, "metres");
		parameters.link("accumulationMethod", this->accumulationMethod = accumulationMethod, "MAXIMUM|AVERAGE|WAVG:2:2|MAXW");

		dataSelector.setQuantities("DBZH:DBZ:DBZV"); // ("^DBZH$");
		odim.product = "MAX";
		// odim.quantity = "DBZH";

	};

	void processData(const Data<PolarSrc> & data, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const;
	// void filter(const Hi5Tree &src, const std::map<double,std::string> & srcPaths, Hi5Tree &dst) const;

	drain::Range<double> altitude;
	// double altitude;
	// double devAltitude;


};





}  // ::rack

#endif /*POLARMax_H_*/

// Rack
