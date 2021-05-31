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

#include "data/Data.h"
#include "data/DataSelector.h"
#include "data/PolarODIM.h"
#include "data/QuantityMap.h"
//#include "drain/imageops/SlidingWindowHistogramOp.h"
//#include "drain/imageops/SlidingWindowOp.h"
//#include "drain/util/Fuzzy.h"
//#include "drain/util/LookUp.h"
#include "drain/image/Geometry.h"
#include "drain/image/Image.h"
#include "drain/image/ImageChannel.h"
#include "drain/image/Window.h"
#include "drain/imageops/SlidingWindowOp.h"
#include "product/ProductOp.h"
#include "radar/Doppler.h"
#include "drain/util/Fuzzy.h"
#include "drain/util/Log.h"
#include "drain/util/SmartMap.h"
#include "drain/util/Type.h"
#include "drain/util/TypeUtils.h"
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

		parameters.link("width", this->conf.widthM = widthM, "metres");
		parameters.link("height", this->conf.heightD = heightD, "deg");
		//parameters.append(conf, false); // TODO: Window::conf parameters ?
		parameters.link("threshold", this->conf.contributionThreshold = 0.5, "percentage");
		parameters.link("compensate", this->conf.invertPolar = false, "cart/polar[0|1]");
		//parameters.link("relativeScale", this->conf.relativeScale = false, "0|1");


		dataSelector.count = 1;
		allowedEncoding.clear();
		allowedEncoding.link("type", odim.type);
		allowedEncoding.link("gain", odim.scaling.scale);

	};

	virtual
	void setEncoding(const ODIM & inputODIM, PlainData<PolarDst> & dst) const;


	virtual
	double getTypicalMin(const PolarODIM & srcODIM) const = 0;

	virtual
	double getTypicalMax(const PolarODIM & srcODIM) const = 0;

	/// Convert window dimensions from metres and radians to image pixels.
	/*
	void setPixelConf(typename W::conf_t & pixelConf, const PolarODIM & inputODIM) const {
		this->conf.setPixelConf(pixelConf, inputODIM);
	}
	*/

};



template <class W>
void DopplerWindowOp<W>::setEncoding(const ODIM & inputODIM, PlainData<PolarDst> & dst) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	dst.odim.quantity = odim.quantity;

	drain::ReferenceMap typeRef;
	typeRef.link("type", dst.odim.type = odim.type);
	typeRef.updateValues(encodingRequest);
	dst.data.setType(dst.odim.type);

	if (odim.scaling.scale != 0.0){ // NOTE: now dst.odim.scaling.scale at least default (1.0)
		mout.warn() << "Init with ODIM: " << EncodingODIM(odim)  << mout.endl;
		//ProductOp::
		applyODIM(dst.odim, odim);
	}
	else {
		// const double max = (this->conf.relativeScale) ? 1.0 : inputODIM.NI;
		if (drain::Type::call<drain::typeIsSmallInt>(dst.data.getType())){
			dst.setPhysicalRange(getTypicalMin(inputODIM), getTypicalMax(inputODIM));
			//mout.note() << EncodingODIM(inputODIM) << mout.endl;
			mout.debug()  << "small int: " << EncodingODIM(dst.odim)  << mout.endl;
			mout.debug2() << "small int: " << dst.data  << mout.endl;
			// dstData.data.setScaling(dstData.odim.scaling.scale, dstData.odim.scaling.offset);
		}
		else {
			if (drain::Type::call<drain::typeIsInteger>(dst.data.getType()))
				mout.warn() << "large int" << mout.endl;
		}
		dst.odim.distinguishNodata(); // change undetect
	}

	//ProductBase::applyODIM(dst.odim, inputODIM, true);  // New. Use defaults if still unset
	ProductBase::completeEncoding(dst.odim, encodingRequest);

	dst.data.setScaling(dst.odim.scaling); // needed?
	//dst.data.setScaling(dst.odim.scaling.scale, dst.odim.scaling.offset);
	mout.debug() << "final dst: " << dst.data  << mout.endl;
}


template <class W>
void DopplerWindowOp<W>::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	const drain::RegExp quantityRe(dataSelector.quantity); // "VRADH?";

	const Data<PolarSrc> & vradSrc = srcSweep.getData(quantityRe);

	if (vradSrc.data.isEmpty()){
		mout.warn() << "VRAD missing" <<  mout.endl;
		return;
	}

	if (vradSrc.odim.getNyquist() == 0) {
		mout.warn() << "VRAD unusable, vradSrc.odim.getNyquist() == 0" <<  mout.endl;
		return;
	}

	Data<PolarDst> & dstData = dstProduct.getData(odim.quantity); // quality data?

	dstData.odim.updateLenient(vradSrc.odim);
	//dstData.odim.NI = vradSrc.odim.NI;
	initDst(vradSrc.odim, dstData);

	dstData.data.fill(dstData.odim.undetect); // NEW 2019/11

	processData(vradSrc, dstData);
	//mout.warn() << "after:" << dstData.data.getScaling() << mout.endl;


	//@ dstProduct.updateTree(dstData.odim);


}

/*
template <class W>
void DopplerWindowOp<W>::setPixelConf(typename W::conf_t & pixelConf, const PolarODIM & inputODIM) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	// pixelConf = this->conf;  PROBLEM: ftor prevents op=
	pixelConf.widthM  = this->conf.widthM;
	pixelConf.heightD = this->conf.heightD;
	pixelConf.updatePixelSize(inputODIM);
	pixelConf.invertPolar   = this->conf.invertPolar;
	pixelConf.contributionThreshold  = this->conf.contributionThreshold;
	pixelConf.relativeScale = this->conf.relativeScale;

	if (pixelConf.frame.width == 0){
		mout.note() << this->conf.frame.width  << mout.endl;
		mout.note() << this->conf.widthM << mout.endl;
		mout.note() << *this << mout.endl;
		mout.warn() << "Requested width (" << pixelConf.widthM <<  " meters) smaller than rscale ("<< inputODIM.rscale <<"), setting window width=1 " << mout.endl;
		pixelConf.frame.width = 1;
	}

	if (pixelConf.frame.height == 0){
		mout.warn() << "Requested height (" << pixelConf.heightD <<  " degrees) smaller than 360/nrays ("<< (360.0/inputODIM.geometry.height) <<"), setting window height=1 " << mout.endl;
		pixelConf.frame.height = 1;
	}

}
*/

// NOTE: window.write() may skip ftor! ?
template <class W>
void DopplerWindowOp<W>::processData(const Data<PolarSrc> & vradSrc, Data<PolarDst> & dstData) const {

		drain::Logger mout(__FUNCTION__, __FILE__);

		//DopplerDevWindow w;
		//w.initialize();

		typename W::conf_t pixelConf;
		this->conf.setPixelConf(pixelConf, vradSrc.odim);
		//setPixelConf(pixelConf, vradSrc.odim);

		SlidingWindowOp<W> op(pixelConf);
		mout.debug() << op << mout.endl;
		mout.special() << "provided functor: " <<  op.conf.getFunctorName() << '|' << op.conf.functorParameters << mout.endl;
		mout.debug() << "pixelConf.contributionThreshold " << pixelConf.contributionThreshold << mout.endl;
		mout.debug() << "op.conf.contributionThreshold " << op.conf.contributionThreshold << mout.endl;
		//dstData.data.setGeometry(vradSrc.data.getGeometry()); // setDst() handles
		//op.process(vradSrc.data, dstData.data);
		//op.traverseChannel(vradSrc.data.getChannel(0), dstData.data.getChannel(0));
		op.traverseChannel(vradSrc.data, dstData.data);

		dstData.odim.prodpar = this->parameters.getValues();
		//@~ dstData.updateTree();

}

/// Simple op not producing quality field.
/*
class DopplerAvgOp : public DopplerWindowOp<DopplerAverageWindow> {
public:

	**
	 *   \param width  - radial extent of window, in metres
	 *   \param height - azimuthal extent of window, in degrees
	 *
	DopplerAvgOp(int width = 1500, double height = 3.0) :
		DopplerWindowOp<DopplerAverageWindow>(__FUNCTION__, "Smoothens Doppler field", width, height) {
		parameters.link("relativeScale", this->conf.relativeScale = false, "0|1");
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
*/

class DopplerAvgOp : public DopplerWindowOp<DopplerAverageWindow2> {
public:

	/**
	 *   \param width  - radial extent of window, in metres
	 *   \param height - azimuthal extent of window, in degrees
	 */
	DopplerAvgOp(int width = 1500, double height = 3.0) :
		DopplerWindowOp<DopplerAverageWindow2>(__FUNCTION__, "Smoothens Doppler field, providing quality", width, height) {
		parameters.link("relativeScale", this->conf.relativeScale = false, "false|true");
		odim.quantity = "VRAD"; // VRAD_C (corrected)?
	};

	virtual ~DopplerAvgOp(){};

	virtual inline
	void processData(const Data<PolarSrc> & vradSrc, Data<PolarDst> & dstData) const {

		drain::Logger mout(__FUNCTION__, __FILE__);
		drain::FuzzyBell2<double> deviationQuality(1.0, 0.125); // 50m/s
		DopplerAverageWindow2::conf_t pixelConf(deviationQuality);
		// setPixelConf(pixelConf, vradSrc.odim);
		this->conf.setPixelConf(pixelConf, vradSrc.odim);

		mout.debug()  << "radarConf: " << this->conf.widthM << 'x' << this->conf.heightD << mout.endl;
		mout.debug()  << "pixelConf: " << pixelConf.frame.width << 'x' << this->conf.frame.height << mout.endl;

		SlidingWindowOp<DopplerAverageWindow2> op(pixelConf);
		mout.debug() << op << mout.endl;
		mout.warn() << op << mout.endl;
		mout.special() << "provided functor: " <<  op.conf.getFunctorName() << '|' << op.conf.functorParameters << mout.endl;
		//mout.debug()  << "provided functor: " << op.conf.ftor << mout.endl;

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



class DopplerEccentricityOp : public DopplerWindowOp<DopplerEccentricityWindow> {
public:

	/**
	 *   \param width  - radial extent of window, in metres
	 *   \param height - azimuthal extent of window, in degrees
	 */
	DopplerEccentricityOp(int widthM = 1500, double heightD = 3.0) :
		DopplerWindowOp<DopplerEccentricityWindow>(__FUNCTION__, "Magnitude of mean unit circle mapped Doppler speeds", widthM, heightD) {

		//parameters.link("relativeScale", this->conf.relativeScale = true, "true|false");

		//this->conf.relativeScale = true;
		odim.quantity = "VRAD_DEV"; // VRAD_C ?
		odim.scaling.scale   = 1.0/200.0;
		odim.scaling.offset = -odim.scaling.scale;
		//odim.scaling.scale   = 1.0/200.0;
		odim.type = "C";
		odim.nodata = 255; // TODO
	};

	virtual ~DopplerEccentricityOp(){};

protected:

	virtual inline
	double getTypicalMin(const PolarODIM & srcODIM) const {
		return 0.0;
	}

	virtual inline
	double getTypicalMax(const PolarODIM & srcODIM) const {
		return +1.0;
		//return (this->conf.relativeScale) ? +1.0 : +srcODIM.getNyquist();
	}


};




}


#endif /* DOPPLERDevOP_H_ */
