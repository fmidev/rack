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

#include <cmath>

#include <drain/util/FunctorBank.h>
#include <drain/util/Fuzzy.h>
#include <drain/util/Geo.h>

#include <drain/image/File.h>
#include <drain/imageops/SlidingWindowOp.h>


#include "radar/Geometry.h"
#include "radar/Doppler.h"
#include "radar/PolarSmoother.h"

#include "DopplerInversionWindow.h"

#include "DopplerInversionOp.h"





namespace rack {


void DopplerWindOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(name, __FUNCTION__);

	const Data<PolarSrc> & srcData = srcSweep.getFirstData(); // VRAD or VRADH

	mout.note() << "src dataSet: " << srcSweep << mout.endl;

	mout.warn() << "src data quantity=" << srcData.odim.quantity << ':' << srcData.data << mout.endl;

	if (srcData.data.isEmpty()){
		// Actually this should be in higher level
		mout.warn() << "data empty" << mout.endl;
		return;
	}

	//mout.warn() << "allocating data AMVU" << mout.endl;
	PlainData<PolarDst> & dstDataU   = dstProduct.getData("AMVU");
	/*
	 mout.note() << "AMVU tests" << mout.endl;
	mout.warn() << "AMVU: quantity=" << dstDataU.odim.quantity << mout.endl;
	mout.warn() << "AMVU: quantity=" << dstProduct.getData("AMVU").odim.quantity << mout.endl;

	typedef std::map<std::string, PlainData<PolarDst> > dmap;
	dmap m;
	m.insert(dmap::value_type("AMFY", PlainData<PolarDst>(dstProduct.getTree()["data98"], "AMFU")));
	mout.note() << "AMFY/AMFU: quantity=" << m.begin()->second.odim.quantity << mout.endl;

	PlainData<PolarDst> dstDataU2(dstProduct.getTree()["data99"],"AMVU");
	mout.warn() << "AMVU2: quantity=" << dstDataU2.odim.quantity << mout.endl;

	PlainData<PolarDst> dstDataU3(dstDataU2);
	mout.warn() << "AMVU3: quantity=" << dstDataU3.odim.quantity << mout.endl;
	*/

	//dstDataU.data.clear();
	mout.warn() << "allocating data AMVV" << mout.endl;
	PlainData<PolarDst> & dstDataV   = dstProduct.getData("AMVV");
	//dstDataV.data.clear();
	PlainData<PolarDst> & dstQuality = dstProduct.getQualityData();
	//dstQuality.data.clear();

	dstDataU.odim.type = odim.type;
	ProductBase::applyODIM(dstDataU.odim, odim, true);
	ProductBase::completeEncoding(dstDataU.odim, encodingRequest);
	dstDataU.data.setType(dstDataU.odim.type);
	setGeometry(srcData.odim, dstDataU);
	mout.debug() << "dstDataU.odim" << EncodingODIM(dstDataU.odim) << mout.endl;

	dstDataV.odim.type = odim.type;
	ProductBase::applyODIM(dstDataV.odim, odim, true);
	ProductBase::completeEncoding(dstDataV.odim, encodingRequest);
	dstDataV.data.setType(dstDataV.odim.type);
	setGeometry(srcData.odim, dstDataV);

	getQuantityMap().setQuantityDefaults(dstQuality, "QIND");
	setGeometry(srcData.odim, dstQuality);

	mout.warn() << "U:    " << dstDataU << mout.endl;
	mout.warn() << "V:    " << dstDataV << mout.endl;

	/*
	mout.warn() << "scr:  " << srcData << mout.endl;
	mout.warn() << "QIND: " << dstQuality << mout.endl;
	//mout.warn() << "VRADC" << dstDataVRAD << mout.endl;
	*/


	const DopplerInversionConfig conf(widthM, heightD);


	if (srcData.hasQuality()){
		DopplerInversionWindowWeighted window(conf, dstDataU.odim);
		mout.warn() << "WEIGHTED, not used yet" << mout.endl;
	}
	else {

	}

	DopplerInversionWindow window(conf, dstDataU.odim);

	window.conf.updatePixelSize(srcData.odim);
	//window.resetAtEdges = true;

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

	dstDataU.odim.prodpar = getParameters().getKeys();
	dstDataU.odim.updateLenient(srcData.odim); // date, time, etc
	dstDataU.data.properties.importMap(dstDataU.odim);
	dstDataV.data.properties.importMap(dstDataV.odim);


	// Copy VRAD
	PlainData<PolarDst> & dstDataVRAD   = dstProduct.getData("VRAD");
	//dstDataVRAD.odim.importMap(srcData.odim);
	dstDataVRAD.copyEncoding(srcData);
	dstDataVRAD.setGeometry(srcData.data.getGeometry());
	dstDataVRAD.data.copyDeep(srcData.data);

	/// FUTURE EXTENSION (VVPslots > 0)
	if (VVP && false){

		mout.info() << "computing VVP " << mout.endl;

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
		// mout.warn() << "computing VVP ended " << mout.endl;
		//@ dstDataVVPU.updateTree();
		//@ dstDataVVPV.updateTree();
		//@ dstDataVVPQ.updateTree();
		//@ dstDataHGHT.updateTree();
	}

	/// Run also
	/*
	if (odim.NI < 0.0){
		mout.warn() << "deviation, " << srcData << mout.endl;
		SlidingWindowOp<RadarWindowDopplerDev<PolarODIM> > vradDevOp; // op not needed
		vradDevOp.setSize(width, height);
		vradDevOp.window.setSize(width, height);
		vradDevOp.window.countThreshold = (width*height)/5;  // require 20% of valid samples
		//const double pos = 5.4321/vradSrc.odim.NI;
		//vradDevOp.window.functor.set( 0.8*pos, 1.2*pos, 255.0 );
		vradDevOp.window.functor.gain = 0.01;
		vradDevOp.window.functor.offset = 0.0;
		vradDevOp.window.odimSrc = srcData.odim;
		vradDevOp.filter(srcData.data, dstQuality.data);
	}
	 */

	//drain::image::File::write(dst,"DopplerInversionOp.png");
	mout.debug(3) << window.odimSrc << mout.endl;


}



void DopplerDiffOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	drain::Logger mout(getName(), __FUNCTION__);

	//double azm;
	int j1, j2;
	double v1, v2, diff;


	dstData.setEncoding(odim.type);
	dstData.odim.setRange(-dMax,+dMax);
	dstData.data.setScaling(dstData.odim.gain, dstData.odim.offset);

	const double minPhys = dstData.odim.getMin();
	const double maxPhys = dstData.odim.getMax();

	mout.warn() << "range " << minPhys << ',' << maxPhys << mout.endl;

	//PlainData<PolarDst> & dstQuality = dstData.getQualityData();
	//setGeometry(dstData.odim, dstQuality);

	for (size_t j=0; j<srcData.data.getHeight(); ++j){

			//azm = srcData.odim.getAzimuth(j);
			j1 = (j-1 + srcData.odim.nrays) % srcData.odim.nrays;
			j2 = (j+1 + srcData.odim.nrays) % srcData.odim.nrays;

			for (size_t i=0; i<srcData.data.getWidth(); ++i){

				v1 = srcData.data.get<double>(i, j1);
				v2 = srcData.data.get<double>(i, j2);

				if (srcData.odim.deriveDifference(v1, v2, diff)) {
					diff *= 0.5; // because j1,j2 have span +1-(-1) = 2
					//diff = dstData.odim.scaleInverse(diff);
					if ((diff>=minPhys) && (diff<=maxPhys))
						dstData.data.putScaled(i, j, diff);
					else
						dstData.data.put(i,j, dstData.odim.nodata);
				}
				else {
					dstData.data.put(i,j, dstData.odim.undetect);
				}
			}
		}

}



} // ::rack
