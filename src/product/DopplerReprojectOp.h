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

#ifndef RACK_DOPPLER_REPROJECT_OP
#define RACK_DOPPLER_REPROJECT_OP

//#include "PolarProductOp.h"

//
#include "DopplerOp.h"
//#include "radar/PolarSector.h"


namespace rack {


// for Testing
class DopplerReprojectOp : public DopplerOp {
public:

	inline
	DopplerReprojectOp() : DopplerOp(__FUNCTION__, "Creates virtual ") {

		parameters.link("nyquist", odim.NI = 100.0, "max-unamb-velocity");
		parameters.link("match", matchOriginal=0, "flag(aliased=1,nodata=2)"); // ALIASED=1, NODATA=2
		parameters.link("quantity", odim.quantity = "VRAD", "output-quantity");

		dataSelector.setMaxCount(1);
		//dataSelector.setQuantities("^(AMVU|AMVV|VRAD)$");
		dataSelector.setQuantities("AMVU:AMVV:VRAD");

		//odim.quantity; // VRAD_C
		odim.type = "S";
	}

	//	void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;
	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const;


	int matchOriginal = 0;


};




}  // namespace rack


#endif /* RACKOP_H_ */

// Rack
