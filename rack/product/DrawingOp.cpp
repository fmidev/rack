/*

    Copyright 2001 - 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include <stdexcept>

#include "radar/PolarSector.h"

#include "DrawingOp.h"

namespace rack {



DrawingOp::~DrawingOp() {
}



void DrawingOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(name, __FUNCTION__);

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
		mout.warn() << w << mout.endl;
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
