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
#include "DopplerNoiseOp.h"

#include "drain/util/FunctorPack.h"
#include "drain/util/Fuzzy.h"

#include "drain/imageops/FunctorOp.h"
#include "drain/imageops/SlidingWindowHistogramOp.h"

//#include "hi5/Hi5Write.h"
/*#include "data/ODIM.h"
#include "data/Data.h"
#include "radar/Geometry.h"
#include "radar/Analysis.h"
*/
//#include "radar/Doppler.h"


using namespace drain;
using namespace drain::image;

namespace rack {


void DopplerNoiseOp::processDataSet(const DataSet<PolarSrc> & sweepSrc, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

	drain::Logger mout(__FUNCTION__, __FILE__);
	mout.debug3() << "start" <<  mout.endl; //

	//const double MAX = dstData.odim.scaleInverse(1.0); // dstData.data.getMax<double>(); //dstData.odim.scaleInverse(1);

	//static drain::RegExp regExpVRAD("^VRAD[H]?$");
	const Data<PolarSrc> &  vradSrc = sweepSrc.getFirstData();

	mout.special() << "Data: " << vradSrc <<  mout.endl;

	if (vradSrc.data.isEmpty()){
		mout.warn() << "VRAD missing, skipping..." <<  mout.endl;
		return;
	}

	const double NI = vradSrc.odim.getNyquist(LOG_ERR);

	if (NI == 0) {
		mout.note() << "vradSrc.odim (encoding): " << EncodingODIM(vradSrc.odim) << mout.endl;
		mout.warn() << "vradSrc.odim.NI==0, and could not derive NI from encoding" <<  mout.endl;
		return;
	}

	if (speedDevThreshold > NI) {
		mout.warn() << "speedDevThreshold=" << speedDevThreshold << " exceeds input NI="  << NI << mout.endl;
	}
	else if (speedDevThreshold > (0.8*NI)) {
		mout.warn() << "speedDevThreshold=" << speedDevThreshold << " close to input NI=" << NI << mout.endl;
	}
	else {
		mout.info() << "vradSrc NI=" << NI <<  mout.endl;
	}

	FuzzyStep<double> fuzzyStep; //(0.5);
	const double pos = speedDevThreshold; ///vradSrc.odim.NI; // TODO: consider relative value directly as parameter NO! => alarm if over +/- 0.5

	if (pos > 0.0)
		fuzzyStep.set( 0.5*pos, 1.5*pos); // scales to [0.0,1.0]
	else
		fuzzyStep.set( 1.5*(-pos), 0.5*(-pos)); // scales to [0.0,1.0]

	//DopplerDevWindow::conf_t pixelConf(fuzzyStep, conf.widthM, conf.heightD, 0.5, true, false); // require 5% samples
	// DopplerDevWindow::conf_t pixelConf(conf.widthM, conf.heightD, 0.5, true, false); // require 5% samples
	// pixelConf.updatePixelSize(vradSrc.odim);

	mout.debug() << "vradSrc: " << ODIM(vradSrc.odim)  <<  mout.endl;
	mout.debug() << "vradSrc props:" << vradSrc.data.getProperties() <<  mout.endl;
	mout.debug() << "functor: " << fuzzyStep  <<  mout.endl;

	//FuzzyBell<double> fuzzyBell(0.0, speedDevThreshold, 1.0, 0); //(0.5

	// DopplerDevWindow::conf_t pixelConf(fuzzyBell);
	// DopplerDevWindow::conf_t pixelConf;
	DopplerDevWindow::conf_t pixelConf(fuzzyStep);

	this->conf.setPixelConf(pixelConf, vradSrc.odim);

	mout.special()  << pixelConf.functorParameters <<  mout.endl;
	//pixelConf.getFunctor();

	SlidingWindowOp<DopplerDevWindow> op(pixelConf);

	mout.debug()  << "VRAD op   " << op <<  mout.endl;
	//mout.special()  << "window size: " << op.conf.frame <<  mout.endl;
	//mout.special() << "provided functor: " <<  op.conf.getFunctorName() << '|' << op.conf.functorParameters << mout.endl;

	dstData.setPhysicalRange(0.0, 1.0);
	op.traverseChannel(vradSrc.data, dstData.data);
	//dstData.odim.prodpar = feature;


	DataTools::updateInternalAttributes(dstData.getTree());
}


}

// Rack
