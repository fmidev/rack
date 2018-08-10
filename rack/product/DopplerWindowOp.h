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

#include <drain/util/Fuzzy.h>
#include <drain/util/LookUp.h>

#include <drain/imageops/SlidingWindowOp.h>
#include <drain/imageops/SlidingWindowHistogramOp.h>
//#include <drain/util/FunctorBank.h>


//#include "Analysis.h"
#include "../radar/Doppler.h"
#include "DopplerOp.h"

#include <cmath>


namespace rack {




/**
 *  \tparam W - a class derived from drain::DopplerWindowConfig
 */
template <class W>
class DopplerWindowOp : public DopplerOp {

public:


	typename W::conf_t conf;


	/*
	double threshold; // absolute count of samples needed
	bool compensatePolar;
	bool relativeScale;  // true, if
	*/
	/**
	 *   \param width  - radial extent of window, in metres
	 *   \param height - azimuthal extent of window, in degrees
	 */
	DopplerWindowOp(const std::string & name, const std::string & description, int widthM, double heightD) :
			DopplerOp(name, description), conf() { //, threshold(0.5), compensatePolar(true), relativeScale(relativeScale) {

		parameters.reference("width", this->conf.widthM = widthM, "metres");
		parameters.reference("height", this->conf.heightD = heightD, "deg");
		//parameters.append(conf, false); // TODO: Window::conf parameters ?
		parameters.reference("threshold", this->conf.contributionThreshold = 0.5, "percentage");
		parameters.reference("compensate", this->conf.invertPolar = false, "cart/polar");
		//parameters.reference("relativeScale", this->conf.relativeScale = false, "0|1");

		//parameters.append(); // TODO: Window::conf parameters ?
		/*
		//parameters.reference("threshold", this->threshold = threshold, "percentage");
		//parameters.reference("compensate", this->compensatePolar = false, "cart/polar");
		*/

		dataSelector.count = 1;
		allowedEncoding.clear();
		allowedEncoding.reference("type", odim.type);
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


	// Outputs uField and vField both, so dst dataSET needed
	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const ;
	// virtual void processImage(const PolarODIM & odim, const Image &src, Image &dst) const;

	virtual
	void processData(const Data<PolarSrc> & vradSrc, Data<PolarDst> & dstData) const;

protected:

	virtual
	void setEncoding(const ODIM & inputODIM, PlainData<PolarDst> & dst) const;


	virtual
	double getPhysicalMin(const PolarODIM & srcODIM) const = 0;

	virtual
	double getPhysicalMax(const PolarODIM & srcODIM) const = 0;

	/*
	virtual inline
	double getPhysicalMin(const PolarODIM & srcODIM) const {
		return (this->conf.relativeScale) ? 1.0 : -srcODIM.NI;
	}

	virtual inline
	double getPhysicalMax(const PolarODIM & srcODIM) const {
		return (this->conf.relativeScale) ? 1.0 : +srcODIM.NI;
	}
	*/

};



template <class W>
void DopplerWindowOp<W>::setEncoding(const ODIM & inputODIM, PlainData<PolarDst> & dst) const {

	drain::Logger mout(name, __FUNCTION__);

	dst.odim.quantity = odim.quantity;

	if (odim.gain != 0.0){ // NOTE: now dst.odim.gain at least default (1.0)
		mout.warn() << "Init with use ODIM: " << EncodingODIM(odim)  << mout.endl;
		//ProductOp::
		applyODIM(dst.odim, odim);
	}
	else {
		// const double max = (this->conf.relativeScale) ? 1.0 : inputODIM.NI;
		if (drain::Type::call<drain::typeIsSmallInt>(dst.data.getType())){
			dst.setPhysicalRange(getPhysicalMin(inputODIM), getPhysicalMax(inputODIM));
			//mout.note() << EncodingODIM(inputODIM) << mout.endl;
			mout.warn() << "small int: " << EncodingODIM(dst.odim)  << mout.endl;
			mout.note() << "small int: " << dst.data  << mout.endl;
			// dstData.data.setScaling(dstData.odim.gain, dstData.odim.offset);
		}
		else {
			if (drain::Type::call<drain::typeIsInteger>(dst.data.getType()))
				mout.warn() << "large int" << mout.endl;
		}
	}

	ProductBase::applyODIM(dst.odim, inputODIM, true);  // New. Use defaults if still unset
	ProductBase::handleEncodingRequest(dst.odim, encodingRequest);


	dst.data.setScaling(dst.odim.gain, dst.odim.offset);
	mout.warn() << "final dst: " << dst.data  << mout.endl;
}


template <class W>
void DopplerWindowOp<W>::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(name, __FUNCTION__);

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

	dstData.odim.NI = vradSrc.odim.NI;
	initDst(vradSrc.odim, dstData);

	processData(vradSrc, dstData);
	//mout.warn() << "after:" << dstData.data.getScaling() << mout.endl;


	//@ dstProduct.updateTree(dstData.odim);


}

// NOTE: window.write() may skip ftor! ?
template <class W>
void DopplerWindowOp<W>::processData(const Data<PolarSrc> & vradSrc, Data<PolarDst> & dstData) const {

		drain::Logger mout(name, __FUNCTION__);

		// typename W::conf_t opConf(fuzzy, this->conf.widthM, this->conf.heightD, this->conf.contributionThreshold, this->conf.invertPolar); //*static_cast<double>(w*h));// require 20% of valid samples
		typename W::conf_t opConf(this->conf.widthM, this->conf.heightD,
				this->conf.contributionThreshold, this->conf.invertPolar, this->conf.relativeScale); //*static_cast<double>(w*h));// require 20% of valid samples
		opConf.updatePixelSize(vradSrc.odim);

		if (opConf.width == 0){
			mout.note() << this->conf.width  << mout.endl;
			mout.note() << this->conf.widthM << mout.endl;
			mout.note() << *this << mout.endl;
			mout.warn() << "Applied width (" << opConf.widthM <<  " meters) smaller than rscale ("<< vradSrc.odim.rscale <<"), setting window width=1 " << mout.endl;
			opConf.width = 1;
		}

		if (opConf.height == 0){
			mout.warn() << "Applied height (" << opConf.heightD <<  " degrees) smaller than 360/nrays ("<< (360.0/vradSrc.odim.nrays) <<"), setting window height=1 " << mout.endl;
			opConf.height = 1;
		}


		SlidingWindowOp<W> op(opConf);
		mout.debug() << op << mout.endl;
		mout.debug() << "provided functor: " << op.conf.ftor << mout.endl;
		//dstData.data.setGeometry(vradSrc.data.getGeometry()); // setDst() handles
		//op.process(vradSrc.data, dstData.data);
		op.traverseChannel(vradSrc.data.getChannel(0), dstData.data.getChannel(0));

		dstData.odim.prodpar = this->parameters.getValues();
		//@~ dstData.updateTree();

}


class DopplerAvgOp : public DopplerWindowOp<DopplerAverageWindow> {
public:

	/**
	 *   \param width  - radial extent of window, in metres
	 *   \param height - azimuthal extent of window, in degrees
	 */
	DopplerAvgOp(int width = 1500, double height = 3.0) :
		DopplerWindowOp<DopplerAverageWindow>(__FUNCTION__, "Smoothens Doppler field", width, height) {
		parameters.reference("relativeScale", this->conf.relativeScale = false, "0|1");
		odim.quantity = "VRAD"; // VRAD_C (corrected)?
	};

	virtual ~DopplerAvgOp(){};

protected:

	virtual inline
	double getPhysicalMin(const PolarODIM & srcODIM) const {
		return (this->conf.relativeScale) ? -1.0 : -srcODIM.NI;
	}

	virtual inline
	double getPhysicalMax(const PolarODIM & srcODIM) const {
		return (this->conf.relativeScale) ? +1.0 : +srcODIM.NI;
	}

};


class DopplerDevOp : public DopplerWindowOp<DopplerDevWindow> {
public:

	/**
	 *   \param width  - radial extent of window, in metres
	 *   \param height - azimuthal extent of window, in degrees
	 */
	DopplerDevOp(int widthM = 1500, double heightD = 3.0) :
		DopplerWindowOp<DopplerDevWindow>(__FUNCTION__, "Computes std.dev.[m/s] of Doppler field", widthM, heightD) {
		parameters.reference("relativeScale", this->conf.relativeScale = true, "0|1");

		//this->conf.relativeScale = true;
		odim.quantity = "VRAD_DEV"; // VRAD_C ?
		odim.offset = 0.0;
		odim.gain   = 0.0;
		odim.type = "C";
		odim.nodata = 255; // TODO
	};

	virtual ~DopplerDevOp(){};

protected:

	virtual inline
	double getPhysicalMin(const PolarODIM & srcODIM) const {
		return 0.0;
	}

	virtual inline
	double getPhysicalMax(const PolarODIM & srcODIM) const {
		return (this->conf.relativeScale) ? 1.0 : +srcODIM.NI;
	}


};


}


#endif /* DOPPLERDevOP_H_ */
