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
		parameters.reference("dbzReference", this->dbzRef = dbzRef, "dBZ");
		parameters.reference("altitudeReference", this->hRef = hRef, "metres");
		parameters.reference("aboveSeaLevel", this->aboveSeaLevel = aboveSeaLevel, "false=radar site|true=sea level");

		odim.product = "ETOP";
		odim.quantity = "HGHT";
		odim.type = "C";

		dataSelector.quantity = "^DBZH$";

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

// Rack
