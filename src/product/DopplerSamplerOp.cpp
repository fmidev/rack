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

#include "DopplerSamplerOp.h"

namespace rack {


void DopplerSamplerOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	//const Data<PolarSrc> & srcData = srcSweep.getData("VRAD");
	const Data<PolarSrc> & srcData = srcSweep.getFirstData();  // VRAD or VRADH

	if (srcData.data.isEmpty()){
		mout.warn("data empty" );
		return;
	}
	//setEncoding(srcData.odim, dstData.odim);


	w.adjustIndices(srcData.odim);
	if (w.rayRange.max < w.rayRange.min){
		w.rayRange.max += 360;
	}

	mout.note("odim       : " , srcData.odim );
	mout.warn("range      : " , w.distanceRange );
	mout.warn("bin indices: " , w.binRange );
	mout.warn("azm        : " , w.azmRange   );
	mout.warn("ray indices: " , w.rayRange );

	mout.warn("azm.first: " , w.azmRange.min , " => ray.min="  ,  srcData.odim.getRayIndex(w.azmRange.min) );

	size_t count = (w.rayRange.max - w.rayRange.min) * (w.binRange.max - w.binRange.min);
	//mout.warn("size " , count );

	PlainData<PolarDst> & dstDataU = dstProduct.getData("X");
	PlainData<PolarDst> & dstDataV = dstProduct.getData("Y");

	//const QuantityMap & qm = getQuantityMap();
	//qm.setTypeDefaults(dstDataU, "d"); //typeid(double));
	//dstDataU.setTypeDefaults("d"); //typeid(double));
	dstDataU.setEncoding(typeid(double));
	//
	dstDataU.data.setGeometry(count, 1);
	dstDataU.odim.quantity = "X"; // ???
	//dstDataU.odim.scaling.scale = 1.0;
	dstDataU.data.fill(dstDataU.odim.undetect);
	//initDst(srcData, dstDataU);
	//
	//qm.setTypeDefaults(dstDataV, "d"); //typeid(double));
	//dstDataV.setTypeDefaults("d"); //typeid(double));
	dstDataV.setEncoding(typeid(double));
	dstDataV.data.setGeometry(count, 1);
	dstDataV.odim.quantity = "Y";
	//dstDataV.odim.scaling.scale = 1.0;
	dstDataV.data.fill(dstDataV.odim.undetect);
	//initDst(srcData, dstDataV);

	//@ dstDataU.updateTree();
	//@ dstDataV.updateTree();

	//@? dstProduct.updateTree(odim);

	mout.debug('\t' , dstDataU.data.getGeometry() );
	mout.debug('\t' , dstDataV.data.getGeometry() );


	double d,x,y;
	size_t index = 0;
	int j2;
	for (int j=w.rayRange.min; j<w.rayRange.max; ++j){
		j2 = (j+srcData.odim.area.height) % srcData.odim.area.height;
		for (int i = w.binRange.min; i<w.binRange.max; ++i){
			d = srcData.data.get<double>(i, j2);
			//if ((d != srcData.odim.undetect) && (d != srcData.odim.nodata)){
			if (srcData.odim.isValue(d)){
				// mout.warn("data d: " , (double)d );
				srcData.odim.mapDopplerSpeed(d, x, y);
				dstDataU.data.put(index, x);
				dstDataV.data.put(index, y);
			}
			else {
				dstDataU.data.put(index, 0);
				dstDataV.data.put(index, 0);
			}
			// mout.warn('\t' , index );
			++index;
		}
	}

}


void DopplerDiffPlotterOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	const Data<PolarSrc > & srcData = srcSweep.getData("VRAD");

	if (srcData.data.isEmpty()){
		mout.warn("data empty" );
		return;
	}
	// setEncoding(srcData.odim, dstData.odim);
	// DopplerInversionWindow dw;

	w.adjustIndices(srcData.odim);
	if (w.rayRange.max < w.rayRange.min){
		w.rayRange.max += 360;
	}
	mout.warn("rays: " , w.rayRange.min , '-' , w.rayRange.max );

	const double NI = srcData.odim.getNyquist();

	mout.warn("NI: " , NI );

	// mout.warn("ray=" , w.ray1 , ',' , w.ray2 , ", bins=" , w.bin1 , ',' , w.bin2 );

	size_t count = (w.rayRange.max - w.rayRange.min) * (w.binRange.max - w.binRange.min);
	//mout.warn("size " , count );

	PlainData<PolarDst> & dstDataU = dstProduct.getData("AZM"); // deg
	dstDataU.setEncoding(typeid(double));
	dstDataU.data.setGeometry(count, 1);
	dstDataU.odim.quantity = "AZM"; // ???
	dstDataU.data.fill(dstDataU.odim.undetect);

	PlainData<PolarDst> & dstData1 = dstProduct.getData("VRAD_NORM");
	dstData1.setEncoding(srcData.odim.type);
	dstData1.odim.setRange(-1.0, 1.0);
	//dstData1.data.setScaling(dstData1.odim.scaling.scale, dstData1.odim.scaling.offset);
	dstData1.data.setScaling(dstData1.odim.scaling);
	dstData1.data.setGeometry(count, 1);
	dstData1.odim.quantity = "VRAD_NORM";
	dstData1.data.fill(dstData1.odim.undetect);

	PlainData<PolarDst> & dstData2 = dstProduct.getData("VRAD_DIFF");
	dstData2.setEncoding(typeid(double));
	dstData2.data.setGeometry(count, 1);
	dstData2.odim.quantity = "VRAD_DIFF";
	//dstData2.odim.setRange();
	dstData2.data.fill(dstData2.odim.undetect);


	mout.debug('\t' , dstDataU.data.getGeometry() );
	mout.debug('\t' , dstData2.data.getGeometry() );
	mout.debug('\t' , dstData1.data.getGeometry() );


	double azm, v1,v2,vDiff; // ,x,y;

	int j1;
	int j2;

	size_t index = 0;
	for (int j=w.rayRange.min; j<w.rayRange.max; ++j){

		azm = srcData.odim.getBeamWidth() * static_cast<double>(j);
		j1 = (j-1 + srcData.odim.area.height) % srcData.odim.area.height;
		j2 = (j+1 + srcData.odim.area.height) % srcData.odim.area.height;

		for (int i = w.binRange.min; i<w.binRange.max; ++i){

			v1 = srcData.data.get<double>(i, j1);
			v2 = srcData.data.get<double>(i, j2);

			if (srcData.odim.deriveDifference(v1, v2, vDiff)) {
				// mout.warn("data d: " , (double)d );

				dstDataU.data.put(index, azm*drain::RAD2DEG);

				v1 = srcData.data.get<double>(i,j);
				if (srcData.odim.isValue(v1)){
					v1 = srcData.odim.scaleForward(v1) / NI; // phys
					dstData1.data.put(index, dstData1.odim.scaleInverse(v1));
				}
				else
					dstData1.data.put(index, dstData1.odim.nodata);
				//dstData1.data.putScaled(index, 1, srcData.data.getScaled(i,j) / NI);  // srcData.data.getScaled(i,j)); // simply copy

				vDiff = vDiff/2.0;// span=2 between j1 and j2
				dstData2.data.put(index, 1.0*vDiff);
			}
			else {
				dstDataU.data.put(index, dstDataU.odim.nodata);
				dstData1.data.put(index, dstData1.odim.nodata);
				dstData2.data.put(index, dstData2.odim.nodata);
			}
			// mout.warn('\t' , index );
			++index;
		}
	}

}



}  // rack::

// Rack
