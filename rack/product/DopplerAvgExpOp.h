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

#ifndef DOPPLER_AvgExpOP_H_
#define DOPPLER_AvgExpOP_H_

#include "DopplerOp.h"


namespace rack {

class DopplerAvgExpOp : public PolarProductOp {
public:

	DopplerAvgExpOp() : PolarProductOp(__FUNCTION__, "Doppler field smoother with exponential decay weighting") {
		parameters.reference("decay", decay = 0.8, "[0.0,1.0]");
		parameters.reference("smoothNess", smoothNess = 0.5, "[0.0,1.0]"); // neighbor weight
		dataSelector.count = 1;
		//dataSelector.quantity = "VRAD";
		dataSelector.quantity = "DBZH";
		//odim.quantity = "RESP";
		//odim.quantity = "VRAD";
		odim.quantity = "DBZH";
		odim.type = "S";
	}

	virtual
	void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;

	virtual
	void processData1D(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;

	//int order;
	double decay;
	double smoothNess;

};

}  // rack::


#endif


