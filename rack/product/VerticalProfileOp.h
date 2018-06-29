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

#ifndef VERTICALProfileOp_H_
#define VERTICALProfileOp_H_

#include "VolumeOp.h"
#include "data/VerticalODIM.h"

namespace rack {

/// Computes vertical profiles of reflectivity (DBZH), including raw and polarized reflectivity.
/**
 *   The resulting image is oriented "naturally", ie. minHeight and maxHeight mapped to the bottom and top rows, respectively.
 */
class VerticalProfileOp: public VolumeOp<VerticalProfileODIM> {

public:

	VerticalProfileOp(double minRange=10, double range=100, double minHeight=0, double maxHeight=10000, long int levels=100, double startaz=0.0, double stopaz=359.99, long int azSlots=1) :
		VolumeOp<VerticalProfileODIM>(__FUNCTION__ ,"Computes vertical dBZ distribution in within range [minRange,maxRange] km.") { // std::string type="d", double gain=0.5, double offset=-32.0) :

		parameters.reference("minRange",  odim.minRange = minRange, "km");
		parameters.reference("range",  odim.range = range, "km");
		parameters.reference("minHeight", odim.minheight = minHeight, "m");
		parameters.reference("maxHeight", odim.maxheight = maxHeight, "m");
		parameters.reference("levels", odim.levels = levels);
		parameters.reference("startaz", odim.startaz = startaz, "deg");
		parameters.reference("stopaz", odim.stopaz = stopaz, "deg");
		parameters.reference("azSlots", odim.azSlots = azSlots);

		odim.reference("interval", interval);

		odim.object = "VP";  // used by VolumeOp::processVolume
		odim.product = "VP"; // used by VericalProfileOp::processVolume

		odim.quantity = "";  // will be chosen by dataselector

		allowedEncoding.reference("type", odim.type = "d");
		//allowedEncoding.reference("gain", odim.gain, 0.5);
		//allowedEncoding.reference("offset", odim.offset, -32.0);

		dataSelector.path = ".*/data[0-9]+/?$";
		dataSelector.quantity = "^DBZH$";

	}


	virtual ~VerticalProfileOp(){};

protected:

	mutable
	double interval;

	void setGeometry(const PolarODIM & srcODIM, PlainData<VprDst> & dstData) const {};

	//virtual	void setGeometry(Data<VprDst> & dstData) const;

	virtual
	void processDataSets(const DataSetMap<PolarSrc> & srcSweeps, DataSet<VprDst> & dstProduct) const;



};

}

#endif /* VERTICALProfileOp_H_ */

// Rack
