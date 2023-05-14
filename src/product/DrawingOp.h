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

#ifndef Drawing_OP_H_
#define Drawing_OP_H_

#include "drain/util/Variable.h"

#include "PolarProductOp.h"
#include "radar/PolarWindow.h"

namespace rack {


/// Select and visualise a direction, range, sector or arc.
/**
 * - direction,<azm>,<range>[,<range2>]
 * - range,{range}[,{range2}]
 * - sector,{azm1},{azm2},{range}[,{range2}]
 * - arc,{azm1},{azm2},{range}
 *
 */
class DrawingOp : public PolarProductOp {
public:

	DrawingOp(const std::string & name, const std::string &description) : PolarProductOp(name, description){
		dataSelector.quantity = "";
	}

	DrawingOp() : PolarProductOp("DrawingOp", "Visualise a direction[azm,r,r2], range[r,r2], sector[azm,azm2,r,r2] or arc[azm,azm2,r].") {
		parameters.link("shape", shape = "direction", "[direction|range|sector|arc]");
		parameters.link("p1", p1 = 0.0);
		parameters.link("p2", p2 = 0.0);
		parameters.link("p3", p3 = 0.0);
		parameters.link("p4", p4 = 0.0);
		/*
		link("azm1", w.azm1, 0.0, "deg");
		link("range1", w.range1, 0, "km");
		link("azm2", w.azm2, 0.0, "deg");
		link("range2", w.range2, 0, "km");
		link("ray1", w.ray1, 0, "index");
		link("bin1", w.bin1, 0, "index");
		link("ray2", w.ray2, 0, "index");
		link("bin2", w.bin2, 0, "index");
		*/
		parameters.link("marker", this->marker = 255, "intensity");
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

// Rack
