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

#ifndef POLAR_SLIDINGWINDOWOP_H_
#define POLAR_SLIDINGWINDOWOP_H_

#include "drain/imageops/SlidingWindowOp.h"

#include "../radar/Analysis.h" // temp
#include "PolarProductOp.h"

namespace rack {


template <class W>
class PolarSlidingWindowOp : public PolarProductOp {

public:


	typename W::conf_t conf;

	PolarSlidingWindowOp(const std::string & name = __FUNCTION__, const std::string &description = "") : PolarProductOp(name,description) {

		parameters.reference("width", this->conf.widthM = 1500, "metres");
		parameters.reference("height", this->conf.heightD = 3.0, "deg");
		parameters.reference("threshold", this->conf.contributionThreshold = 0.5, "percentage");
		parameters.reference("invertPolar", this->conf.invertPolar = false, "cart/polar");
		// parameters.append(conf.getParameters());
	};


	virtual
	void processData(const Data<src_t > & srcData, Data<dst_t > & dstData) const {
		drain::Logger mout(__FUNCTION__, __FILE__);
		if (srcData.hasQuality()){
			mout.warn() << "quality found, weighted operation" << mout.endl;
			processDataWeighted(srcData, dstData);
		}
		else {
			mout.warn() << "no quality, unweighted operation" << mout.endl;
			mout.warn() << dstData.data.getScaling() << mout.endl;
			processPlainData(srcData, dstData);
		}
	};

	virtual
	void processPlainData(const PlainData<src_t > & srcData, PlainData<dst_t > & dstData) const {

		drain::Logger mout(__FUNCTION__, __FILE__);
		//mout.warn() << "not implemented" << mout.endl;
		typename W::conf_t pixelConf;
		this->setPixelConf(srcData.odim, pixelConf); // what about other parameters?

		mout.warn() << "srcData.odim: " << srcData.odim << mout.endl;

		SlidingWindowOp<W> op(pixelConf);
		mout.warn() << op << mout.endl;
		mout.debug() << "provided functor: " << op.conf.ftor << mout.endl;
		mout.debug() << "pixelConf.contributionThreshold " << pixelConf.contributionThreshold << mout.endl;
		mout.debug() << "op.conf.contributionThreshold " << op.conf.contributionThreshold << mout.endl;
		//dstData.data.setGeometry(vradSrc.data.getGeometry()); // setDst() handles
		//op.process(vradSrc.data, dstData.data);
		//op.traverseChannel(vradSrc.data.getChannel(0), dstData.data.getChannel(0));
		//srcData.data.properties.updateFromMap(srcData.odim);
		op.process(srcData.data, dstData.data);
		// op.traverseChannel(srcData.data, dstData.data);

		dstData.odim.prodpar = this->parameters.getValues();

	};

	/// Quality-weighted prosessing of data
	virtual
	void processDataWeighted(const Data<src_t > & srcData, Data<dst_t > & dstData) const {
		drain::Logger mout(__FUNCTION__, __FILE__);
		mout.warn() << "not implemented, calling for plain data" << mout.endl;

		processPlainData(srcData, dstData);

	};



protected:

	/// Convert azimuthal and radial quantities to pixels
	void setPixelConf(const PolarODIM & odim, typename W::conf_t & pixelConf) const;



};


template <class W>
void PolarSlidingWindowOp<W>::setPixelConf(const PolarODIM & odim, typename W::conf_t & pixelConf) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

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
		mout.warn() << "Requested height (" << pixelConf.heightD <<  " degrees) smaller than 360/nrays ("<< (360.0/odim.geometry.height) <<"), setting window height=1 " << mout.endl;
		pixelConf.height = 1;
	}


}

class PolarSlidingAvgOp : public PolarSlidingWindowOp<RadarWindowAvg<RadarWindowConfig> > {
public:

	inline
	PolarSlidingAvgOp() : PolarSlidingWindowOp< RadarWindowAvg<RadarWindowConfig> >(__FUNCTION__, "descr"){
	};

};


}

#endif /* POLARSLIDINGDOPPLERWINDOW_H_ */
