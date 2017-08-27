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
*//**

    Copyright 2001 - 2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)

    This file is part of AnoRack, a module of Rack for C++.

    AnoRack is not free software.

*/

#ifndef DOPPLER_WIN_OP_H_
#define DOPPLER_WIN_OP_H_

//#include "PolarSlidingWindowOp.h"

#include <drain/image/SlidingWindowOp.h>
#include <drain/image/SlidingWindowHistogramOp.h>
#include <drain/util/Fuzzy.h>
#include <drain/util/LookUp.h>
//#include <drain/util/FunctorBank.h>


//#include "Analysis.h"
#include "DopplerOp.h"

#include <cmath>


namespace rack {




/**
 *  \tparam W - a class derived from drain::DopplerWindowConfig
 */
template <class W>
class DopplerWindowOp : public DopplerOp {
public:

	int widthM;     // metres
	double heightD; // degrees
	double threshold; // absolute count of samples needed
	bool compensatePolar;

	/**
	 *   \param width  - radial extent of window, in metres
	 *   \param height - azimuthal extent of window, in degrees
	 */
	DopplerWindowOp(const std::string & name, const std::string & description, int widthM, double heightD) : DopplerOp(name, description), threshold(0.5), compensatePolar(true) {

		parameters.reference("width", this->widthM = widthM, "metres");
		parameters.reference("height", this->heightD = heightD, "deg");
		//parameters.reference("threshold", this->threshold = threshold, "percentage");
		//parameters.reference("compensate", this->compensatePolar = false, "cart/polar");

		dataSelector.count = 1;
		allowedEncoding.clear();
		allowedEncoding.reference("gain", odim.gain);

	};

	/*
	DopplerWindowOp(int width = 1500, double height = 3.0) : DopplerOp("DopplerXX", "Computes Doppler speed xxx.") {

		parameters.reference("width", this->widthM = width, "m");
		parameters.reference("height", this->heightD = height, "deg");
		parameters.reference("threshold", this->threshold = threshold, "percentage");
		//parameters.reference("compensate", this->compensatePolar = false, "cart/polar");

		dataSelector.count = 1;
		allowedEncoding.clear();
		allowedEncoding.reference("gain", odim.gain);

	};
	*/

	virtual ~DopplerWindowOp(){};


	// Outputs u and v both, so dst dataSET needed
	virtual
	void processDataSet(const DataSetSrc<PolarSrc> & srcSweep, DataSetDst<PolarDst> & dstProduct) const ;
	// virtual void filterImage(const PolarODIM & odim, const Image &src, Image &dst) const;

	virtual
	void processData(const Data<PolarSrc> & vradSrc, Data<PolarDst> & dstData) const;


};


template <class W>
void DopplerWindowOp<W>::processDataSet(const DataSetSrc<PolarSrc> & srcSweep, DataSetDst<PolarDst> & dstProduct) const {

	drain::MonitorSource mout(name, __FUNCTION__);

	const Data<PolarSrc> & vradSrc = srcSweep.getData("VRAD"); // relax, allow user to modify?

	if (vradSrc.data.isEmpty()){
		mout.warn() << "VRAD missing" <<  mout.endl;
		return;
	}

	if (vradSrc.odim.NI == 0) {
		mout.warn() << "VRAD unusable, vradSrc.odim.NI == 0" <<  mout.endl;
		return;
	}

	Data<PolarDst> & dstData = dstProduct.getData(odim.quantity); // quality data?

	initDst(vradSrc.odim, dstData);
	dstData.odim.NI = vradSrc.odim.NI;

	processData(vradSrc, dstData);

	dstProduct.updateTree(dstData.odim);


}

template <class W>
void DopplerWindowOp<W>::processData(const Data<PolarSrc> & vradSrc, Data<PolarDst> & dstData) const {

		drain::MonitorSource mout(name, __FUNCTION__);

		//const int w = vradSrc.odim.getBeamBins(width); static_cast<int>(width/vradSrc.odim.rscale);
		//const int h = vradSrc.odim.getAzimuthalBins(height); // static_cast<double>(vradSrc.odim.nrays * height)/360.0;

		//drain::FuzzyStep<double> fuzzy;
		drain::FuzzyBell<double> fuzzy;
		double pos = 0.5; //*vradSrc.odim.NI;
		//fuzzy.set( 0.8*pos, 1.2*pos, dstData.odim.scaleInverse(1.0) );  // ftor.setParameters("1.0,0.5");
		//fuzzy.set( 0.8*pos, 1.2*pos, dstData.data.getMax<double>()-1.0 );  // ftor.setParameters("1.0,0.5");
		fuzzy.set(0, -pos, dstData.data.getMax<double>()-1.0 );  // ftor.setParameters("1.0,0.5");

		typename W::config conf(vradSrc.odim, fuzzy, widthM, heightD, this->threshold); //*static_cast<double>(w*h));// require 20% of valid samples
		conf.invertPolar = this->compensatePolar;

		if (conf.width == 0){
			mout.warn() << "Applied width (" << widthM <<  " meters) smaller than rscale ("<< vradSrc.odim.rscale <<"), setting window width=1 " << mout.endl;
			conf.width = 1;
		}

		if (conf.height == 0){
			mout.warn() << "Applied height (" << heightD <<  " degrees) smaller than 360/nrays ("<< (360.0/vradSrc.odim.nrays) <<"), setting window height=1 " << mout.endl;
			conf.height = 1;
		}


		SlidingWindowOpT<W> op(conf);

		mout.debug() << op << mout.endl;

		op.filter(vradSrc.data, dstData.data);
		dstData.odim.prodpar = this->parameters.getValues();

		dstData.updateTree();

}


class DopplerAvgOp : public DopplerWindowOp<DopplerAverageWindow> {
public:

	/**
	 *   \param width  - radial extent of window, in metres
	 *   \param height - azimuthal extent of window, in degrees
	 */
	DopplerAvgOp(int width = 1500, double height = 3.0) :
		DopplerWindowOp<DopplerAverageWindow>(__FUNCTION__, "Smoothens Doppler field", width, height) {
		//this->parameters.reference("invertPolar", this->conf.invertPolar);
		parameters.reference("areaThreshold", this->threshold = 0.5, "percentage");
		parameters.reference("compensate", this->compensatePolar = true, "cart=0|polar=1");
		odim.quantity = "VRAD"; // VRAD_C (corrected)?
	};

	virtual ~DopplerAvgOp(){};



};


class DopplerDevOp : public DopplerWindowOp<DopplerDevWindow> {
public:

	/**
	 *   \param width  - radial extent of window, in metres
	 *   \param height - azimuthal extent of window, in degrees
	 */
	DopplerDevOp(int width = 1500, double height = 3.0) :
		DopplerWindowOp<DopplerDevWindow>(__FUNCTION__, "Computes std.dev.[m/s] of Doppler field", width, height) {
		parameters.reference("areaThreshold", this->threshold = 0.5, "percentage");
		parameters.reference("compensate", this->compensatePolar = true, "cart/polar");
		odim.quantity = "VRAD_DEV"; // VRAD_C ?
		odim.offset = 0.0;
		odim.gain   = 0.5;
		odim.type = "C";
		odim.nodata = 255; // TODO
	};

	virtual ~DopplerDevOp(){};



};


}


#endif /* DOPPLERDevOP_H_ */
