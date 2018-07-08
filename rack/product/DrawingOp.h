/*

    Copyright 2001 - 2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#ifndef Drawing_OP_H_
#define Drawing_OP_H_

#include <drain/util/Variable.h>

#include "PolarProductOp.h"
#include "radar/PolarWindow.h"

namespace rack {


/// Select and visualise a direction, range, sector or arc.
/**
 * - direction,<azm>,<range>[,<range2>]
 * - range,<range>[,<range2>]
 * - sector,<azm1>,<azm2>,<range>[,<range2>]
 * - arc,<azm1>,<azm2>,<range>
 *
 */
class DrawingOp : public PolarProductOp {
public:

	DrawingOp(const std::string & name, const std::string &description) : PolarProductOp(name, description){
		dataSelector.quantity = "";
	}

	DrawingOp() : PolarProductOp("DrawingOp", "Visualise a direction[azm,r,r2], range[r,r2], sector[azm,azm2,r,r2] or arc[azm,azm2,r].") {
		parameters.reference("shape", shape = "direction", "[direction|range|sector|arc]");
		parameters.reference("p1", p1 = 0.0);
		parameters.reference("p2", p2 = 0.0);
		parameters.reference("p3", p3 = 0.0);
		parameters.reference("p4", p4 = 0.0);
		/*
		reference("azm1", w.azm1, 0.0, "deg");
		reference("range1", w.range1, 0, "km");
		reference("azm2", w.azm2, 0.0, "deg");
		reference("range2", w.range2, 0, "km");
		reference("ray1", w.ray1, 0, "index");
		reference("bin1", w.bin1, 0, "index");
		reference("ray2", w.ray2, 0, "index");
		reference("bin2", w.bin2, 0, "index");
		*/
		parameters.reference("marker", this->marker = 255, "intensity");
		dataSelector.quantity = "";
		dataSelector.count = 1;
	};

	virtual ~DrawingOp();



protected:

	std::string shape;
	double p1,p2,p3,p4;

	int marker;


	// Potentially a general utility. rack::Geometry / ODIM?
	//void deriveWindow(const PolarODIM srcOdim, int & ray1, int & ray2, int & bin1, int & bin2) const ;

	// void drawWindow(int ray1, int ray2, int bin1, int bin2, int marker, PlainData<PolarDst> & dstData) const;

	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const ;


	// virtual void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const ;


};


}  // namespace rack


#endif /* RACKOP_H_ */
