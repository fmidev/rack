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

#include <stdexcept>

#include "radar/PolarSector.h"

#include "DrawingOp.h"

namespace rack {



DrawingOp::~DrawingOp() {
}



void DrawingOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	const Data<PolarSrc> & srcData = srcSweep.getFirstData();

	Data<PolarDst> & dstData = dstProduct.getData(srcData.odim.quantity);

	if (dstData.data.isEmpty()){
		dstData.data.copyDeep(srcData.data); // todo: scale
		dstData.odim.updateFromMap(srcData.odim);
		if (dstData.odim.distinguishNodata("VRAD"))
			mout.note() << "forced nodata=" <<  dstData.odim.nodata << mout.endl;
		mout.debug() << "copy dstData: " << EncodingODIM(dstData.odim) << mout.endl;
	}

	PolarSector w;
	//mout.warn() << "ray=" << ray1 << ',' << ray2 << ", bins=" << bin1 << ',' << bin2 << mout.endl;

	if ((shape.length()>1) && (shape != "direction") && (shape != "range") && (shape != "arc") && (shape != "sector")){
		mout.error() << "unknown shape type: " << shape << mout.endl;
		return;
	}

	int j1,j2;

	const char shapecode = shape.empty() ? 'n' : shape.at(0);  // n=none

	switch (shapecode) {
	case 'n':
		// just copy, draw nothing
		break;
	case 'd': // direction
		w.setAzimuth(p1);
		w.setRange(p2,p3);
		w.adjustIndices(dstData.odim);
		for (int i = w.bin1; i<w.bin2; ++i){
			dstData.data.put(i, w.ray1, marker);
		}
		break;
	case 'r': // range(s)
		w.setRange(p1,p2);
		w.adjustIndices(dstData.odim);
		if (w.bin2 > w.bin1){
			for (int j=0; j<dstData.odim.nrays; ++j){
				dstData.data.put(w.bin1, j, marker);
				dstData.data.put(w.bin2, j, marker);
			}
		}
		else {
			for (int j=0; j<dstData.odim.nrays; ++j){
				dstData.data.put(w.bin1, j, marker);
			}
		}
		break;
	case 'a': // arc
		w.setAzimuth(p1,p2);
		w.setRange(p3,p3);
		w.adjustIndices(dstData.odim);
		for (int j=w.ray1; j<w.ray2; ++j){
			j1 = w.getSafeRay(dstData.odim, j);
			dstData.data.put(w.bin1, j1, marker);
		}
		break;
	case 's': // sector
		w.setAzimuth(p1,p2); //odim.nrays;
		w.setRange(p3,p4);
		w.adjustIndices(dstData.odim);
		mout.debug() << w << mout.endl;
		for (int j=w.ray1; j<w.ray2; ++j){
			j1 = w.getSafeRay(dstData.odim, j);
			dstData.data.put(w.bin1, j1, marker);
			dstData.data.put(w.bin2, j1, marker);
		}
		j1 = w.getSafeRay(dstData.odim, w.ray1);
		j2 = w.getSafeRay(dstData.odim, w.ray2);
		//j0 = (w.ray2+dstData.odim.nrays) % dstData.odim.nrays;
		for (int i = w.bin1; i<w.bin2; ++i){
			dstData.data.put(i, j1, marker);
			dstData.data.put(i, j2, marker);
		}
		break;
	default:
		mout.error() << "Unknown shape code: " << shape << mout.endl;
		break;
	}

	//@= dstData.updateTree();
	//@= dstProduct.updateTree(odim);

}


}  // rack::

// Rack
