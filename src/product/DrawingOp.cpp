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

	drain::Logger mout(__FILE__, __FUNCTION__);

	const Data<PolarSrc> & srcData = srcSweep.getFirstData();

	Data<PolarDst> & dstData = dstProduct.getData(srcData.odim.quantity);

	if (dstData.data.isEmpty()){
		dstData.data.copyDeep(srcData.data); // todo: scale
		dstData.odim.updateFromMap(srcData.odim);
		if (dstData.odim.distinguishNodata("VRAD"))
			mout.note("forced nodata=" ,  dstData.odim.nodata );
		mout.debug("copy dstData: " , EncodingODIM(dstData.odim) );
	}

	PolarSector w;
	//mout.warn("ray=" , ray1 , ',' , ray2 , ", bins=" , bin1 , ',' , bin2 );

	if ((shape.length()>1) && (shape != "direction") && (shape != "range") && (shape != "arc") && (shape != "sector")){
		mout.error("unknown shape type: " , shape );
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
		for (int i = w.binRange.min; i<w.binRange.max; ++i){
			dstData.data.put(i, w.rayRange.min, marker);
		}
		break;
	case 'r': // range(s)
		w.setRange(p1,p2);
		w.adjustIndices(dstData.odim);
		if (w.binRange.max > w.binRange.min){
			for (size_t j=0; j<dstData.odim.area.height; ++j){
				dstData.data.put(w.binRange.min, j, marker);
				dstData.data.put(w.binRange.max, j, marker);
			}
		}
		else {
			for (size_t j=0; j<dstData.odim.area.height; ++j){
				dstData.data.put(w.binRange.min, j, marker);
			}
		}
		break;
	case 'a': // arc
		w.setAzimuth(p1,p2);
		w.setRange(p3,p3);
		w.adjustIndices(dstData.odim);
		for (int j=w.rayRange.min; j<w.rayRange.max; ++j){
			j1 = w.getSafeRay(dstData.odim, j);
			dstData.data.put(w.binRange.min, j1, marker);
		}
		break;
	case 's': // sector
		w.setAzimuth(p1,p2); //odim.geometry.height;
		w.setRange(p3,p4);
		w.adjustIndices(dstData.odim);
		mout.debug(w );
		for (int j=w.rayRange.min; j<w.rayRange.max; ++j){
			j1 = w.getSafeRay(dstData.odim, j);
			dstData.data.put(w.binRange.min, j1, marker);
			dstData.data.put(w.binRange.max, j1, marker);
		}
		j1 = w.getSafeRay(dstData.odim, w.rayRange.min);
		j2 = w.getSafeRay(dstData.odim, w.rayRange.max);
		//j0 = (w.ray2+dstData.odim.geometry.height) % dstData.odim.geometry.height;
		for (int i = w.binRange.min; i<w.binRange.max; ++i){
			dstData.data.put(i, j1, marker);
			dstData.data.put(i, j2, marker);
		}
		break;
	default:
		mout.error("Unknown shape code: " , shape );
		break;
	}

	//@= dstData.updateTree();
	//@= dstProduct.updateTree(odim);

}


}  // rack::

// Rack
