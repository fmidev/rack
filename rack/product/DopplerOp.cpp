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
#include "DopplerOp.h"

namespace rack {



DopplerOp::~DopplerOp() {
}


void DopplerOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(name, __FUNCTION__);

	const Data<PolarSrc > & srcData = srcSweep.getData("VRAD");

	if (srcData.data.isEmpty()){
		mout.warn() << "data empty" << mout.endl;
		return;
	}
	//setEncoding(srcData.odim, dstData.odim);


	w.adjustIndices(srcData.odim);
	if (w.ray2 < w.ray1){
		w.ray2 += 360;
	}
	mout.warn() << w.ray1 << '-' << w.ray2 << mout.endl;

	// mout.warn() << "ray=" << w.ray1 << ',' << w.ray2 << ", bins=" << w.bin1 << ',' << w.bin2 << mout.endl;

	size_t count = (w.ray2-w.ray1) * (w.bin2-w.bin1);
	//mout.warn() << "size " << count << mout.endl;

	PlainData<PolarDst> & dstDataU = dstProduct.getData("X");
	PlainData<PolarDst> & dstDataV = dstProduct.getData("Y");

	//const QuantityMap & qm = getQuantityMap();
	//qm.setTypeDefaults(dstDataU, "d"); //typeid(double));
	//dstDataU.setTypeDefaults("d"); //typeid(double));
	dstDataU.setEncoding(typeid(double));
	//
	dstDataU.data.setGeometry(count, 1);
	dstDataU.odim.quantity = "X"; // ???
	//dstDataU.odim.gain = 1.0;
	dstDataU.data.fill(dstDataU.odim.undetect);
	//initDst(srcData, dstDataU);
	//
	//qm.setTypeDefaults(dstDataV, "d"); //typeid(double));
	//dstDataV.setTypeDefaults("d"); //typeid(double));
	dstDataV.setEncoding(typeid(double));
	dstDataV.data.setGeometry(count, 1);
	dstDataV.odim.quantity = "Y";
	//dstDataV.odim.gain = 1.0;
	dstDataV.data.fill(dstDataV.odim.undetect);
	//initDst(srcData, dstDataV);

	//@ dstDataU.updateTree();
	//@ dstDataV.updateTree();

	//@? dstProduct.updateTree(odim);

	mout.debug() << '\t' << dstDataU.data.getGeometry() << mout.endl;
	mout.debug() << '\t' << dstDataV.data.getGeometry() << mout.endl;


	double d,x,y;
	size_t index = 0;
	int j2;
	for (int j=w.ray1; j<w.ray2; ++j){
		j2 = (j+srcData.odim.nrays)%srcData.odim.nrays;
		for (int i = w.bin1; i<w.bin2; ++i){
			d = srcData.data.get<double>(i, j2);
			if ((d != srcData.odim.undetect) && (d != srcData.odim.nodata)){
				// mout.warn() << "data d: " << (double)d << mout.endl;
				srcData.odim.mapDopplerSpeed(d, x, y);
				dstDataU.data.put(index, x);
				dstDataV.data.put(index, y);
			}
			else {
				dstDataU.data.put(index, 0);
				dstDataV.data.put(index, 0);
			}
			// mout.warn() << '\t' << index << mout.endl;
			++index;
		}
	}

}

}  // rack::
