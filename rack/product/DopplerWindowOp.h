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


#ifndef DOPPLER_WIN_OP_H_
#define DOPPLER_WIN_OP_H_

//#include "PolarSlidingWindowOp.h"

#include <data/Data.h>
#include <data/DataSelector.h>
#include <data/PolarODIM.h>
#include <data/QuantityMap.h>
//#include <drain/imageops/SlidingWindowHistogramOp.h>
//#include <drain/imageops/SlidingWindowOp.h>
//#include <drain/util/Fuzzy.h>
//#include <drain/util/LookUp.h>
#include <image/Geometry.h>
#include <image/Image.h>
#include <image/ImageChannel.h>
#include <image/Window.h>
#include <imageops/SlidingWindowOp.h>
#include <product/ProductOp.h>
#include <radar/Doppler.h>
#include <util/Fuzzy.h>
#include <util/Log.h>
#include <util/SmartMap.h>
#include <util/Type.h>
#include <util/TypeUtils.h>
#include "DopplerOp.h"
//#include <cmath>
#include <string>


namespace rack {



/// Base class for Doppler average and variance
/**
 *  \tparam W - a class derived from drain::DopplerWindowConfig
 */
template <class W>
class DopplerWindowOp : public DopplerOp {

public:


	typename W::conf_t conf;




	virtual ~DopplerWindowOp(){};


	// Outputs uField and vField both, so dst dataSET needed
	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const ;
	// virtual void processImage(const PolarODIM & odim, const Image &src, Image &dst) const;

	virtual
	void processData(const Data<PolarSrc> & vradSrc, Data<PolarDst> & dstData) const;

protected:

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
		parameters.reference("compensate", this->conf.invertPolar = false, "cart/polar[0|1]");
		//parameters.reference("relativeScale", this->conf.relativeScale = false, "0|1");


		dataSelector.count = 1;
		allowedEncoding.clear();
		allowedEncoding.reference("type", odim.type);
		allowedEncoding.reference("gain", odim.gain);

	};

	virtual
	void setEncoding(const ODIM & inputODIM, PlainData<PolarDst> & dst) const;


	virtual
	double getTypicalMin(const PolarODIM & srcODIM) const = 0;

	virtual
	double getTypicalMax(const PolarODIM & srcODIM) const = 0;

	/// Convert window dimensions from metres and radians to image pixels.
	void setPixelConf(typename W::conf_t & pixelConf, const PolarODIM & odim) const;

};



template <class W>
void DopplerWindowOp<W>::setEncoding(const ODIM & inputODIM, PlainData<PolarDst> & dst) const {

	drain::Logger mout(name, __FUNCTION__);

	dst.odim.quantity = odim.quantity;

	drain::ReferenceMap typeRef;
	typeRef.reference("type", dst.odim.type = odim.type);
	typeRef.updateValues(encodingRequest);
	dst.data.setType(dst.odim.type);

	if (odim.gain != 0.0){ // NOTE: now dst.odim.gain at least default (1.0)
		mout.warn() << "Init with use ODIM: " << EncodingODIM(odim)  << mout.endl;
		//ProductOp::
		applyODIM(dst.odim, odim);
	}
	else {
		// const double max = (this->conf.relativeScale) ? 1.0 : inputODIM.NI;
		if (drain::Type::call<drain::typeIsSmallInt>(dst.data.getType())){
			dst.setPhysicalRange(getTypicalMin(inputODIM), getTypicalMax(inputODIM));
			//mout.note() << EncodingODIM(inputODIM) << mout.endl;
			mout.debug()  << "small int: " << EncodingODIM(dst.odim)  << mout.endl;
			mout.debug(1) << "small int: " << dst.data  << mout.endl;
			// dstData.data.setScaling(dstData.odim.gain, dstData.odim.offset);
		}
		else {
			if (drain::Type::call<drain::typeIsInteger>(dst.data.getType()))
				mout.warn() << "large int" << mout.endl;
		}
	}

	//ProductBase::applyODIM(dst.odim, inputODIM, true);  // New. Use defaults if still unset
	ProductBase::handleEncodingRequest(dst.odim, encodingRequest);

	dst.data.setScaling(dst.odim.gain, dst.odim.offset);
	mout.debug() << "final dst: " << dst.data  << mout.endl;
}


template <class W>
void DopplerWindowOp<W>::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(name, __FUNCTION__);

	const Data<PolarSrc> & vradSrc = srcSweep.getData("VRAD"); // relax, allow user to modify?

	if (vradSrc.data.isEmpty()){
		mout.warn() << "VRAD missing" <<  mout.endl;
		return;
	}

	if (vradSrc.odim.getNyquist() == 0) {
		mout.warn() << "VRAD unusable, vradSrc.odim.getNyquist() == 0" <<  mout.endl;
		return;
	}

	Data<PolarDst> & dstData = dstProduct.getData(odim.quantity); // quality data?

	dstData.odim.update(vradSrc.odim);
	//dstData.odim.NI = vradSrc.odim.NI;
	initDst(vradSrc.odim, dstData);

	processData(vradSrc, dstData);
	//mout.warn() << "after:" << dstData.data.getScaling() << mout.endl;


	//@ dstProduct.updateTree(dstData.odim);


}

template <class W>
void DopplerWindowOp<W>::setPixelConf(typename W::conf_t & pixelConf, const PolarODIM & odim) const {

	drain::Logger mout(name, __FUNCTION__);

	// pixelConf = this->conf;  PROBLEM: ftor prevents op=
	pixelConf.widthM  = this->conf.widthM;
	pixelConf.heightD = this->conf.heightD;
	pixelConf.invertPolar   = this->conf.invertPolar;
	pixelConf.contributionThreshold  = this->conf.contributionThreshold;
	pixelConf.relativeScale = this->conf.relativeScale;


	pixelConf.updatePixelSize(odim);

	if (pixelConf.width == 0){
		mout.note() << this->conf.width  << mout.endl;
		mout.note() << this->conf.widthM << mout.endl;
		mout.note() << *this << mout.endl;
		mout.warn() << "Requested width (" << pixelConf.widthM <<  " meters) smaller than rscale ("<< odim.rscale <<"), setting window width=1 " << mout.endl;
		pixelConf.width = 1;
	}

	if (pixelConf.height == 0){
		mout.warn() << "Requested height (" << pixelConf.heightD <<  " degrees) smaller than 360/nrays ("<< (360.0/odim.nrays) <<"), setting window height=1 " << mout.endl;
		pixelConf.height = 1;
	}


}

// NOTE: window.write() may skip ftor! ?
template <class W>
void DopplerWindowOp<W>::processData(const Data<PolarSrc> & vradSrc, Data<PolarDst> & dstData) const {

		drain::Logger mout(name, __FUNCTION__);

		//DopplerDevWindow w;
		//w.initialize();

		typename W::conf_t pixelConf;
		setPixelConf(pixelConf, vradSrc.odim);

		SlidingWindowOp<W> op(pixelConf);
		mout.debug() << op << mout.endl;
		mout.debug() << "provided functor: " << op.conf.ftor << mout.endl;
		mout.debug() << "pixelConf.contributionThreshold " << pixelConf.contributionThreshold << mout.endl;
		mout.debug() << "op.conf.contributionThreshold " << op.conf.contributionThreshold << mout.endl;
		//dstData.data.setGeometry(vradSrc.data.getGeometry()); // setDst() handles
		//op.process(vradSrc.data, dstData.data);
		//op.traverseChannel(vradSrc.data.getChannel(0), dstData.data.getChannel(0));
		op.traverseChannel(vradSrc.data, dstData.data);

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
	double getTypicalMin(const PolarODIM & srcODIM) const {
		return (this->conf.relativeScale) ? -1.0 : -srcODIM.getNyquist();
	}

	virtual inline
	double getTypicalMax(const PolarODIM & srcODIM) const {
		return (this->conf.relativeScale) ? +1.0 : +srcODIM.getNyquist();
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
	double getTypicalMin(const PolarODIM & srcODIM) const {
		return 0.0;
	}

	virtual inline
	double getTypicalMax(const PolarODIM & srcODIM) const {
		return (this->conf.relativeScale) ? +1.0 : +srcODIM.getNyquist();
	}


};


class DopplerAvg2Op : public DopplerWindowOp<DopplerAverageWindow2> {
public:

	/**
	 *   \param width  - radial extent of window, in metres
	 *   \param height - azimuthal extent of window, in degrees
	 */
	DopplerAvg2Op(int width = 1500, double height = 3.0) :
		DopplerWindowOp<DopplerAverageWindow2>(__FUNCTION__, "Smoothens Doppler field", width, height) {
		parameters.reference("relativeScale", this->conf.relativeScale = false, "0|1");
		odim.quantity = "VRAD"; // VRAD_C (corrected)?
	};

	virtual ~DopplerAvg2Op(){};

	virtual inline
	void processData(const Data<PolarSrc> & vradSrc, Data<PolarDst> & dstData) const {

		drain::Logger mout(name, __FUNCTION__);
		drain::FuzzyBell2<double> deviationQuality(1.0, 0.125); // 50m/s
		DopplerAverageWindow2::conf_t pixelConf(deviationQuality);
		setPixelConf(pixelConf, vradSrc.odim);

		mout.debug()  << "radarConf: " << this->conf.widthM << 'x' << this->conf.heightD << mout.endl;
		mout.debug()  << "pixelConf: " << pixelConf.width << 'x' << this->conf.height << mout.endl;

		SlidingWindowOp<DopplerAverageWindow2> op(pixelConf);
		mout.debug() << op << mout.endl;
		mout.debug()  << "provided functor: " << op.conf.ftor << mout.endl;

		const drain::image::Geometry & g = vradSrc.data.getGeometry();

		drain::image::Image dummy(g.getWidth(), g.getHeight());

		PlainData<PolarDst> & dstQuality =  dstData.getQualityData("QIND");
		getQuantityMap().setQuantityDefaults(dstQuality, "QIND");
		dstQuality.setGeometry(g.getWidth(), g.getHeight());

		op.traverseChannel(vradSrc.data, dummy,  dstData.data, dstQuality.data);

		dstData.odim.prodpar = this->parameters.getValues();
	}

protected:

	virtual inline
	double getTypicalMin(const PolarODIM & srcODIM) const {
		return (this->conf.relativeScale) ? -1.0 : -srcODIM.getNyquist();
	}

	virtual inline
	double getTypicalMax(const PolarODIM & srcODIM) const {
		return (this->conf.relativeScale) ? +1.0 : +srcODIM.getNyquist();
	}

};



}


#endif /* DOPPLERDevOP_H_ */
