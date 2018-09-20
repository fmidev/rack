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

#include "DopplerNoiseOp.h"

#include <drain/util/FunctorPack.h>
#include <drain/util/Fuzzy.h>

#include <drain/image/File.h>
#include <drain/imageops/FunctorOp.h>
#include <drain/imageops/SlidingWindowHistogramOp.h>

//#include "hi5/Hi5Write.h"

#include "data/ODIM.h"
#include "data/Data.h"
#include "radar/Geometry.h"
#include "radar/Analysis.h"
#include "radar/Doppler.h"


using namespace drain;
using namespace drain::image;

namespace rack {

/*
void DopplerNoiseOp::applyOperator(const ImageOp & op, const std::string & feature, const Data<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {
	Image & tmp = dstProductAux.getQualityData(feature);
}
*/

/* Rename
 *
 *
 */
/*
void DopplerNoiseOp::applyOperator(const ImageOp & op, Image & tmp, const std::string & feature, const Data<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

	drain::Logger mout(getName() + "::"+__FUNCTION__, feature);

	mout.debug() << "running " << feature << '/' << op.getName() << mout.endl;

	const bool NEW = dstData.odim.prodpar.empty();  // or tmp.empty()

	//Channel & channel = dstData.data;
	//channel.properties.updateFromMap(dstData.data.getProperties());

	/// Save directly to target (dstData), if this is the first applied detector
	if (NEW){
		mout.debug(1) << "creating dst image" << mout.endl;
		//dstData.setPhysicalRange(0.0, 1.0);
		op.traverseChannel(src.data, dstData.data);
		dstData.odim.prodpar = feature;
		tmp.copyShallow(dstData.data);
		// tmp.initialize(dstData.data.getType(), dstData.data.getGeometry());
		// tmp.adoptScaling(dstData.data);
	}
	else {
		mout.debug(1) << "tmp exists => accumulating detection" << mout.endl;
		op.process(src.data, tmp);
		//op.traverseChannel(src.data.getChannel(0), tmp.getChannel(0));
		mout.debug(1) << "updating dst image" << mout.endl;
		BinaryFunctorOp<MultiplicationFunctor>().traverseChannel(dstData.data, tmp, dstData.data);
		// File::write(dstData.data, feature+".png");
		dstData.odim.prodpar += ',';
		dstData.odim.prodpar += feature;
	}

	/// Debugging: save intermediate images.
	if (outputDataVerbosity >= 1){
		PlainData<PolarDst> & dstFeature = dstProductAux.getQualityData(feature);  // consider direct instead of copy?
		const QuantityMap & qm = getQuantityMap();
		//dstFeature.odim.setQuantityDefaults("PROB");
		dstFeature.odim.quantity = feature;
		qm.setQuantityDefaults(dstFeature, "PROB");
		if (NEW)
			drain::image::CopyOp().process(dstData.data, dstFeature.data);
		else
			drain::image::CopyOp().process(tmp, dstFeature.data);
		//@ dstFeature.updateTree();
	}

}
*/

void DopplerNoiseOp::processDataSet(const DataSet<PolarSrc> & sweepSrc, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug(2) << "start" <<  mout.endl; //

	const double MAX = dstData.data.getMax<double>(); //dstData.odim.scaleInverse(1);

	//static drain::RegExp regExpVRAD("^VRAD[H]?$");
	const Data<PolarSrc> &  vradSrc = sweepSrc.getFirstData(); // VolumeOpNew::

	if (vradSrc.data.isEmpty()){
		mout.warn() << "VRAD missing, skipping..." <<  mout.endl;
		return;
	}

	const double NI = vradSrc.odim.getNyquist(true);

	if (NI == 0) {
		mout.note() << "vradSrc.odim (encoding): " << EncodingODIM(vradSrc.odim) << mout.endl;
		mout.warn() << "vradSrc.odim.NI==0, and could not derive NI from encoding" <<  mout.endl;
		return;
	}

	FuzzyStep<double> fuzzyStep; //(0.5);
	const double pos = vradDevMin; ///vradSrc.odim.NI; // TODO: consider relative value directly as parameter NO! => alarm if over +/- 0.5

	if (pos > 0.0)
		fuzzyStep.set( 0.8*pos, 1.2*pos, 255.0 );
	else
		fuzzyStep.set( 1.2*(-pos), 0.8*(-pos), MAX );

	DopplerDevWindow::conf_t conf(fuzzyStep, windowWidth, windowHeight, 0.05, true, false); // require 5% samples
	conf.updatePixelSize(vradSrc.odim);
	SlidingWindowOp<DopplerDevWindow> vradDevOp(conf);

	mout.warn() << "fuzzy step: " << fuzzyStep  <<  mout.endl;
	mout.debug(1)  << "VRAD op   " << vradDevOp <<  mout.endl;
	mout.debug()  << vradDevOp.conf.width  << 'x' << vradDevOp.conf.height <<  mout.endl;
	mout.debug()  << vradDevOp.conf.ftor <<  mout.endl;
	mout.debug()  << "vradSrc NI=" << vradSrc.odim.getNyquist() <<  mout.endl;
	mout.debug(1) << "vradSrc props:" << vradSrc.data.getProperties() <<  mout.endl;

	dstData.setPhysicalRange(0.0, 1.0);
	/*
		dstData.data.setOptimalScale(0.0, 1.0);
		dstData.odim.gain   = dstData.data.getScaling().scale;
		dstData.odim.offset = dstData.data.getScaling().offset;
	 */
	vradDevOp.traverseChannel(vradSrc.data, dstData.data);
	//dstData.odim.prodpar = feature;

	//applyOperator(vradDevOp, tmp, "VRAD_DEV", vradSrc, dstData, dstProductAux);
	//mout.debug() << dstData.data <<  mout.endl;
	//mout.debug() << EncodingODIM(dstData.odim) <<  mout.endl;


	DataTools::updateAttributes(dstData.tree);
}


}

// Rack
