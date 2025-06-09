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

#include <drain/image/ImageFile.h>
#include <cmath>

#include <drain/util/FunctorBank.h>
#include <drain/util/Fuzzy.h>
#include <drain/util/Geo.h>

#include <drain/imageops/SlidingWindowOp.h>


#include "radar/Geometry.h"
#include "radar/Doppler.h"
#include "radar/DopplerInversionWindow.h"

#include "DopplerInversionOp.h"





namespace rack {


void DopplerWindOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	const Data<PolarSrc> & srcData = srcSweep.getFirstData(); // VRAD or VRADH

	mout.note("src dataSet: " , srcSweep );

	mout.warn("src data quantity=" , srcData.odim.quantity , ':' , srcData.data );

	if (srcData.data.isEmpty()){
		// Actually this should be in higher level
		mout.warn("data empty" );
		return;
	}

	//mout.warn("allocating data AMVU" );
	PlainData<PolarDst> & dstDataU   = dstProduct.getData("AMVU");
	// mout.warn("allocating data AMVV" );
	PlainData<PolarDst> & dstDataV   = dstProduct.getData("AMVV");
	// dstDataV.data.clear();
	PlainData<PolarDst> & dstQuality = dstProduct.getQualityData();
	//dstQuality.data.clear();

	dstDataU.odim.type = odim.type;
	ProductBase::applyODIM(dstDataU.odim, odim, true);
	ProductBase::completeEncoding(dstDataU.odim, targetEncoding);
	// dstDataU.initialize(dstDataU.odim.type, srcData.odim.getGeometry());
	dstDataU.data.setType(dstDataU.odim.type);
	setGeometry(srcData.odim, dstDataU);
	// mout.debug("dstDataU.odim" , EncodingODIM(dstDataU.odim) );

	dstDataV.odim.type = odim.type;
	ProductBase::applyODIM(dstDataV.odim, odim, true);
	ProductBase::completeEncoding(dstDataV.odim, targetEncoding);
	// dstDataV.initialize(dstDataV.odim.type, srcData.odim.getGeometry());
	dstDataV.data.setType(dstDataV.odim.type);
	setGeometry(srcData.odim, dstDataV);
	// dstDataV.initialize(type, geometry)

	getQuantityMap().setQuantityDefaults(dstQuality, "QIND");
	setGeometry(srcData.odim, dstQuality);

	// dstDataU.odim.prodpar = getParameters().getKeys();
	// dstDataV.odim.prodpar = getParameters().getKeys();

	mout.warn("update");
	dstDataU.odim.updateLenient(srcData.odim); // date, time, etc
	dstDataV.odim.updateLenient(srcData.odim); // date, time, etc

	mout.warn("U:    " , dstDataU );
	mout.warn("V:    " , dstDataV );

	/*
	mout.warn("scr:  " , srcData );
	mout.warn("QIND: " , dstQuality );
	//mout.warn("VRADC" , dstDataVRAD );
	*/


	const DopplerInversionConfig conf(widthM, heightD);


	if (srcData.hasQuality()){
		DopplerInversionWindowWeighted window(conf, dstDataU.odim);
		mout.warn("WEIGHTED, not used yet" );
	}
	else {

	}

	mout.warn("Create window");
	DopplerInversionWindow window(conf, dstDataU.odim);

	window.conf.updatePixelSize(srcData.odim);
	// window.resetAtEdges = true;
	// window.signCos = +1; //(testSigns & 1) ? +1 : -1;
	// window.signSin = -1;(testSigns & 2) ? +1 : -1;

	//window.setSize(width, height);
	window.setSrcFrame(srcData.data);
	window.setDstFrame(dstDataU.data);
	window.setDstFrame2(dstDataV.data);
	window.setDstFrameWeight(dstQuality.data);

	window.functorSetup = altitudeWeight;

	// MAIN OPERATION !
	window.run();

	/*
	dstDataU.odim.prodpar = getParameters().getKeys();

	mout.warn("update");
	dstDataU.odim.updateLenient(srcData.odim); // date, time, etc
	dstDataV.odim.updateLenient(srcData.odim); // date, time, etc
	*/

	mout.warn("update2");
	dstDataU.data.properties.importCastableMap(dstDataU.odim);
	mout.attention("dataU: ", dstDataU.data.properties);

	mout.warn("update3");
	dstDataV.data.properties.importCastableMap(dstDataV.odim);
	mout.attention("dataV: ", dstDataV.data.properties);

	// Copy VRAD
	PlainData<PolarDst> & dstDataVRAD   = dstProduct.getData("VRAD"); // or odim.quantity safer? VRADH?
	mout.warn("copying orig " , srcData.odim.quantity , "->" , "VRAD" );
	dstDataVRAD.copyEncoding(srcData);
	dstDataVRAD.setGeometry(srcData.data.getGeometry());
	dstDataVRAD.data.copyDeep(srcData.data);

	/// FUTURE EXTENSION (VVPslots > 0)
	if (VVP && false){

		mout.info("computing VVP " );

		PlainData<PolarDst> & dstDataHGHT = dstProduct.getData("HGHT"); // altitude
		PlainData<PolarDst> & dstDataVVPU = dstProduct.getData("VVPU"); // wind profile
		PlainData<PolarDst> & dstDataVVPV = dstProduct.getData("VVPV"); // wind profile
		PlainData<PolarDst> & dstDataVVPQ = dstProduct.getData("VVPQ"); // wind profile quality

		const QuantityMap & qm = getQuantityMap();
		qm.setQuantityDefaults(dstDataHGHT, "HGHT", "S");
		qm.setQuantityDefaults(dstDataVVPU, "AMVU", "S");
		qm.setQuantityDefaults(dstDataVVPV, "AMVV", "S");
		qm.setQuantityDefaults(dstDataVVPQ, "QIND", "C");

		const size_t cols = srcData.data.getWidth();
		const size_t rows = srcData.data.getHeight();
		dstDataVVPU.setGeometry(1, cols); // yes transpose
		dstDataVVPV.setGeometry(1, cols);
		dstDataVVPQ.setGeometry(1, cols);
		dstDataHGHT.setGeometry(1, cols);

		double u, v, q;
		double uSum, vSum, qSum;
		unsigned int counter;
		double beta, h;
		const double eta = srcData.odim.elangle * (M_PI/180.0);
		size_t address;
		for (size_t i = 0; i < cols; ++i) {
			beta = srcData.odim.getGroundAngle(i);
			h = 0.001 * Geometry::heightFromEtaBeta(eta, beta);
			//mout.note() << "col:" << i << " h=" << h << " = ";
			dstDataHGHT.data.put(i, dstDataHGHT.odim.scaleInverse(h));
			//mout << dstDataHGHT.odim.scaleForward(dstDataHGHT.data.get<double>(i)) << mout.endl;
			uSum = vSum = qSum = 0.0;
			counter = 0;
			for (size_t j = 0; j < rows; ++j) {
				address = dstQuality.data.address(i,j);
				q = dstQuality.odim.scaleForward(dstQuality.data.get<double>(address));
				if (q > 0.01){
					u = dstDataU.data.get<double>(address);
					v = dstDataV.data.get<double>(address);
					if ((u != dstDataU.odim.undetect) && (u != dstDataU.odim.nodata) && (v != dstDataV.odim.undetect) && (v != dstDataV.odim.nodata)){
						u = dstDataU.odim.scaleForward(u);
						v = dstDataV.odim.scaleForward(v);
						uSum += u;
						vSum += v;
						qSum += q;
						++counter;
					}
				}
			}
			if (qSum > 0.01){
				dstDataVVPU.data.put(i, dstDataVVPU.odim.scaleInverse(uSum/qSum));
				dstDataVVPV.data.put(i, dstDataVVPU.odim.scaleInverse(vSum/qSum));
				dstDataVVPQ.data.put(i, dstDataVVPU.odim.scaleInverse(qSum/static_cast<double>(counter)));
			}
			else {
				dstDataVVPU.data.put(i, dstDataVVPU.odim.nodata);
				dstDataVVPV.data.put(i, dstDataVVPV.odim.nodata);
				dstDataVVPQ.data.put(i, dstDataVVPU.odim.scaleInverse(0));
			}
		}
		// mout.warn("computing VVP ended " );
		//@ dstDataVVPU.updateTree();
		//@ dstDataVVPV.updateTree();
		//@ dstDataVVPQ.updateTree();
		//@ dstDataHGHT.updateTree();
	}

	/// Run also
	/*
	if (odim.NI < 0.0){
		mout.warn("deviation, " , srcData );
		SlidingWindowOp<RadarWindowDopplerDev<PolarODIM> > vradDevOp; // op not needed
		vradDevOp.setSize(width, height);
		vradDevOp.window.setSize(width, height);
		vradDevOp.window.countThreshold = (width*height)/5;  // require 20% of valid samples
		//const double pos = 5.4321/vradSrc.odim.NI;
		//vradDevOp.window.functor.set( 0.8*pos, 1.2*pos, 255.0 );
		vradDevOp.window.functor.scale = 0.01;
		vradDevOp.window.functor.offset = 0.0;
		vradDevOp.window.odimSrc = srcData.odim;
		vradDevOp.filter(srcData.data, dstQuality.data);
	}
	 */

	//drain::image::File::write(dst,"DopplerInversionOp.png");
	mout.debug3(window.odimSrc);


}




} // ::rack
