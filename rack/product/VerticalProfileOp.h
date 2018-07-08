/*

    Copyright 2012 -    Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack software for C++.

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
