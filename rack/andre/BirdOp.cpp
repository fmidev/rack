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

#include "BirdOp.h"

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

// kludge
void BirdOp::init(double dbzPeak, double vradDevMin, double rhoHVmax, double zdrDevMin, double windowWidth, double windowHeight) {

	dataSelector.path = "data[0-9]+/?$";
	//dataSelector.quantity = "^(DBZH|VRAD|WRAD|RHOHV|ZDR)$";
	dataSelector.quantity = "^(DBZH|VRAD|VRADH|RHOHV|ZDR)$";
	dataSelector.count = 1;

	parameters.reference("dbzPeak", this->dbzPeak = dbzPeak, "Typical reflectivity (DBZH)");
	parameters.reference("vradDevMin", this->vradDevMin = vradDevMin, "Minimum of bin-to-bin Doppler speed (VRAD) deviation (m/s)");
	// parameters.reference("wradMin", this->wradMin, wradMin, "Minimum normalized deviation of within-bin Doppler speed deviation (WRAD)");
	parameters.reference("rhoHVmax", this->rhoHVmax = rhoHVmax, "Maximum rhoHV value (fuzzy)");
	parameters.reference("zdrDevMin", this->zdrDevMin = zdrDevMin, "Minimum std.deviation of ZDR (fuzzy)");

	parameters.reference("windowWidth", this->windowWidth = windowWidth, "window width, beam-directional (m)"); //, "[m]");
	parameters.reference("windowHeight", this->windowHeight = windowHeight, "window width, azimuthal (deg)"); //, "[d]");

}

/*
void BirdOp::applyOperator(const ImageOp & op, const std::string & feature, const Data<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {
	Image & tmp = dstProductAux.getQualityData(feature);
}
*/

/* Rename
 *
 *
 */
void BirdOp::applyOperator(const ImageOp & op, Image & tmp, const std::string & feature, const Data<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

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
	if (ProductOp::outputDataVerbosity >= 1){
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

void BirdOp::processDataSet(const DataSet<PolarSrc> & sweepSrc, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug(2) << "start" <<  mout.endl; //

	//mout.error() << dstData <<  mout.endl; //

	//dstData.setPhysicalRange(0.0, 1.0);

	Image tmp(typeid(unsigned char));
	//tmp.adoptScaling(dstData.data);
	// 2018 ? tmp.scaling.setLimits(dstData.data.getMin<double>(), dstData.data.getMax<double>());

	const double MAX = dstData.data.getMax<double>(); //dstData.odim.scaleInverse(1);

	/// Reduction coefficient to compensate missing measurement data
	double overallScale = 1.0;

	const Data<PolarSrc> & dbzSrc = sweepSrc.getData("DBZH"); // VolumeOpNew::
	const bool DBZ = !dbzSrc.data.isEmpty();  // or: || dbzParams.empty() ?
	if (!DBZ){
		mout.warn() << "DBZH missing" <<  mout.endl;
		overallScale *= 0.75;
	}
	else {

		//RadarDataFuzzifier<FuzzyStepsoid<double,double> > dbzFuzzifier;
		//dbzFuzzifier.functor.set(dbzPeak, -2.5, 255); // negative width, ie. decreasing func
		//RadarDataFuzzifier<FuzzyTriangle<double,double> > dbzFuzzifier;
		// dbzFuzzifier.functor.set(-32, dbzPeak, dbzPeak+10.0, 255.0);
		RadarFunctorOp<FuzzyBell<double> > dbzFuzzifier;
		dbzFuzzifier.odimSrc = dbzSrc.odim;
		dbzFuzzifier.functor.set(dbzPeak, +5.0);
		mout.debug() << "DBZ_LOW" << dbzFuzzifier.functor << mout.endl;

		applyOperator(dbzFuzzifier, tmp, "DBZ_LOW", dbzSrc, dstData, dstProductAux);

	}

	static drain::RegExp regExpVRAD("^VRAD[H]?$");
	const Data<PolarSrc> &  vradSrc = sweepSrc.getData(regExpVRAD); // VolumeOpNew::
	const bool VRAD = !vradSrc.data.isEmpty();
	const double NI = vradSrc.odim.getNyquist();
	if (!VRAD){
		mout.warn() << "VRAD missing, skipping..." <<  mout.endl;
		overallScale *= 0.5;
	}
	else if (NI == 0) { //  if (vradSrc.odim.NI == 0) {
		mout.note() << "vradSrc.odim (encoding): " << EncodingODIM(vradSrc.odim) << mout.endl;
		mout.warn() << "vradSrc.odim.NI==0, and could not derive NI from encoding" <<  mout.endl;
		mout.warn() << "skipping VRAD..." <<  mout.endl;
		overallScale *= 0.5;
	}
	else if (vradDevMin > NI) {
		mout.warn() << "vradDevMin (" << vradDevMin << ") exceeds NI of input: " << NI << mout.endl; // semi-fatal
		mout.warn() << "skipping VRAD..." <<  mout.endl;
		overallScale *= 0.5;
	}
	else {

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
		mout.debug()  << "vradSrc NI=" << vradSrc.odim.NI <<  mout.endl;
		mout.debug(1) << "vradSrc props:" << vradSrc.data.getProperties() <<  mout.endl;

		dstData.setPhysicalRange(0.0, 1.0);
		/*
		dstData.data.setOptimalScale(0.0, 1.0);
		dstData.odim.gain   = dstData.data.getScaling().scale;
		dstData.odim.offset = dstData.data.getScaling().offset;
		*/

		applyOperator(vradDevOp, tmp, "VRAD_DEV", vradSrc, dstData, dstProductAux);
		//mout.debug() << dstData.data <<  mout.endl;
		//mout.debug() << EncodingODIM(dstData.odim) <<  mout.endl;

	}

	/*
	const Data<PolarSrc> &  wradSrc = sweepSrc.getData("WRAD"); // VolumeOpNew::
	const bool WRAD = !wradSrc.data.isEmpty();
	if (!WRAD)
		mout.warn() << "WRAD missing" <<  mout.endl;
	else {

		RadarDataFuzzifier<FuzzyStep<double,double> > wradFuzzifier;
		wradFuzzifier.odimSrc = wradSrc.odim;
		wradFuzzifier.functor.set(wradMin*0.75*wradSrc.odim.NI, wradMin*1.25*wradSrc.odim.NI, 255);
		applyOperator(wradFuzzifier, tmp, "WRAD_HIGH", wradSrc, dstData, dstProductAux);

		/// NOT NEEDED?
		SlidingWindowOpT<RadarWindowStdDev<FuzzyStep<double,double> > > wradDevFuzzifier;
		const int w = static_cast<int>(windowWidth/wradSrc.odim.rscale);
		const int h = static_cast<double>(wradSrc.odim.nrays) * windowHeight/360.0;
		wradDevFuzzifier.window.setSize(w, h);
		wradDevFuzzifier.window.functor.set( 0.5, 0.95, 255.0);
		wradDevFuzzifier.window.odimSrc = wradSrc.odim;
		applyOperator(wradDevFuzzifier, tmp, "WRAD_DEV", wradSrc, dstData, dstProductAux);

	}
	 */


	const Data<PolarSrc> &  rhohvSrc = sweepSrc.getData("RHOHV"); // VolumeOpNew::
	const bool RHOHV = !rhohvSrc.data.isEmpty();
	if (!RHOHV){
		overallScale *= 0.5;
		mout.warn() << "RHOHV missing" <<  mout.endl;
	}
	else {

		//RadarFunctorOp<FuzzyStep<double> > rhohvFuzzifier;
		RadarFunctorOp<FuzzyStep<double> > rhohvFuzzifier;
		rhohvFuzzifier.odimSrc = rhohvSrc.odim;
		rhohvFuzzifier.functor.set(rhoHVmax+(1.0-rhoHVmax)/2.0, rhoHVmax);
		mout.debug() << "RHOHV_LOW" << rhohvFuzzifier.functor << mout.endl;
		applyOperator(rhohvFuzzifier, tmp, "RHOHV_LOW", rhohvSrc, dstData, dstProductAux);

	}

	const Data<PolarSrc> &  zdrSrc = sweepSrc.getData("ZDR"); // VolumeOpNew::
	const bool ZDR = !zdrSrc.data.isEmpty();
	if (!ZDR){
		overallScale *= 0.75;
		mout.warn() << "ZDR missing" <<  mout.endl;
	}
	else {

		mout.debug(1) << zdrSrc.odim << mout.endl;

		//RadarDataFuzzifier<FuzzyStep<double,double> > zdrFuzzifier;
		RadarFunctorOp<FuzzyTriangle<double> > zdrFuzzifier;
		zdrFuzzifier.odimSrc = zdrSrc.odim;
		zdrFuzzifier.functor.set(+zdrDevMin, 0.0, -zdrDevMin); // INVERSE //, -1.0, 1.0);
		//zdrFuzzifier.functor.set(0.5, 2.0, 255);
		mout.debug() << "ZDR_NONZERO" << zdrFuzzifier.functor << mout.endl;
		applyOperator(zdrFuzzifier, tmp, "ZDR_NONZERO", zdrSrc, dstData, dstProductAux);
		//applyOperator(zdrFuzzifier, tmp, "ZDR_HIGH", zdrSrc, dstData, dstProductAux);

		/*
		SlidingWindowOpT<RadarWindowStdDev<FuzzyStepsoid<double,double> > > stdDevOp;
		const int w = static_cast<int>(windowWidth/zdrSrc.odim.rscale);
		const int h = static_cast<double>(zdrSrc.odim.nrays) * windowHeight/360.0;
		stdDevOp.window.setSize(w, h);
		stdDevOp.window.functor.set( zdrDevMin, 0.25*zdrDevMin, 255.0);
		stdDevOp.window.odimSrc = zdrSrc.odim;
		applyOperator(stdDevOp, tmp, "ZDR_DEV", zdrSrc, dstData, dstProductAux);
		*/

	}

	if (dstData.data.isEmpty()){
		mout.error() << "could not find input data; quantity=" << dataSelector.quantity;
	}
	else {
		//FunctorOp<FuzzyBell<double,double> > fuzzyBright(0.0,-8.0, dstData.odim.scaleInverse(overallScale));
		//FuzzyBellOp fuzzyBright(0.0,-0.032, overallScale);
		UnaryFunctorOp<FuzzyBell<double> > fuzzyBright;
		fuzzyBright.functor.set(0.0,-0.032, overallScale);
		fuzzyBright.process(dstData.data, dstData.data);
	}
	DataTools::updateAttributes(dstData.tree);
}


}

// Rack
