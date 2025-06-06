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

#include <andre/BirdOp.h>
#include <drain/Log.h>
#include "data/DataSelector.h"
#include "data/DataTools.h"
#include "data/PolarODIM.h"
#include "data/QuantityMap.h"
#include "drain/image/ImageChannel.h"
#include "drain/image/ImageLike.h"
#include "drain/image/Window.h"
#include "drain/imageops/CopyOp.h"
#include "drain/imageops/FunctorOp.h"

// RAISED
// #include "drain/imageops/SlidingWindowOp.h"
// #include "radar/Analysis.h"
// #include "radar/Doppler.h"
#include "drain/util/FunctorPack.h"
// #include "drain/util/Fuzzy.h"
//#include "drain/util/RegExp.h"

#include "BirdOp.h"


namespace rack {

using namespace drain;
using namespace drain::image;



void FuzzyDualPolOp::init(double dbzPeak, double vradDevMin, double rhoHVmax, double zdrAbsMin, double windowWidth, double windowHeight) {

	parameters.link("dbzPeak",      this->dbzPeak = dbzPeak,  "Typical reflectivity (DBZH)");
	parameters.link(VRAD_FLIP ? "vradDevMax":"vradDevMin", this->vradDevRange.tuple(0.0, vradDevMin), "Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)");
	parameters.link("rhoHVmax",     this->rhoHVRange.tuple(0.9*rhoHVmax, rhoHVmax),  "Fuzzy threshold of maximum rhoHV value");
	parameters.link("zdrAbsMin",    this->zdrAbsMin = zdrAbsMin,  "Fuzzy threshold of absolute ZDR");
	parameters.link("windowWidth",  this->window.frame.width = windowWidth,  "window width, beam-directional (m)"); //, "[m]");
	parameters.link("windowHeight", this->window.frame.height = windowHeight,  "window width, azimuthal (deg)"); //, "[d]");
	// parameters.link("wradMin", this->wradMin, wradMin, "Minimum normalized deviation of within-bin Doppler speed deviation (WRAD)");

}




/* Rename
 *
 * Image & tmp,
 */
void FuzzyDualPolOp::applyOperator(const ImageOp & op,  const std::string & feature, const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

	drain::Logger mout(__FUNCTION__, getName() + "::"+feature);

	mout.debug("running " , feature , '/' , op.getName() );
	// mout.debug("ZDR_NONZERO" , op.functor );

	drain::StringBuilder<'_'> featureQuantity("FUZZY", srcData.odim.quantity, op.getName(), op.getParameters().getValues());

	//mout.attention("FUZZY...", feature, '-', op.getName(), '-', srcData.odim.quantity);
	mout.attention("quantity: ", featureQuantity);

	// OLD: (conditional-store)
	// static const std::string response_tmp("FUZZY_LAST");
	// const std::string & response = outputDataVerbosity.isSet(INTERMEDIATE) ? feature : response_tmp;

	// NEW
	PlainData<PolarDst> & dstFeature = dstProductAux.getQualityData(featureQuantity.str());


	if (dstFeature.data.isEmpty()){
		mout.attention("Computing feature [", featureQuantity, "] ");
		// mout.attention(DRAIN_LOG_VAR(dstFeature.odim.quantity));
		// dstFeature.odim.quantity = feature; // this causes weird duplicates - ?
		const QuantityMap & qm = getQuantityMap();
		qm.setQuantityDefaults(dstFeature, "PROB");
		dstFeature.setGeometry(srcData.data.getGeometry());
		// dstFeature.setPhysicalRange(0.0, 1.0); // needed?
		op.traverseChannel(srcData.data, dstFeature.data);
	}
	else {
		mout.ok<LOG_NOTICE>("Feature [", featureQuantity, "] exists already, not recomputing");
	}

	// OLD:
	// op.traverseChannel(srcData.data, dstFeature.data);

	if (!outputDataVerbosity){  // .isSet(DEBUG||INTERMEDIATE)
		dstFeature.setExcluded(); // Do not save in HDF5.
	}

	if (dstData.odim.prodpar.empty()){
		// COPY
		mout.attention("COPY:", srcData.data.getGeometry(), " feat:", dstFeature.data.getGeometry());
		dstData.setGeometry(srcData.data.getGeometry());
		mout.attention("COP2:", dstData.data.getGeometry());
		drain::image::CopyOp().process(dstFeature.data, dstData.data);
		dstData.odim.prodpar = feature;
		mout.attention("COP3:", dstData.data.getGeometry());
	}
	else {
		mout.attention("UPDATE");
		dstData.data.getChannel(0).setPhysicalRange({0,1}, true);
		dstFeature.data.getChannel(0).setPhysicalRange({0,1}, true);
		BinaryFunctorOp<drain::MultiplicationFunctor>().traverseChannel(dstData.data, dstFeature.data, dstData.data);
		// File::write(dstData.data, feature+".png");
		dstData.odim.prodpar += ',';
		dstData.odim.prodpar += feature;
		//dstData.odim.prodpar = drain::StringBuilder<','>(dstData.odim.prodpar, feature);
	}

	//mout.attention(feature, " dstData: ", dstData);

	/*
	/// Save directly to target (dstData), if this is the first applied detector
	if (NEW){
		mout.debug2("creating dst image" );
		//dstData.setPhysicalRange(0.0, 1.0);
		dstData.setPhysicalRange(0.0, 1.0);
		op.traverseChannel(src.data, dstData.data);
		dstData.odim.prodpar = feature;
		tmp.setGeometry(dstData.data.getGeometry());
	}
	else {
		mout.debug2("tmp exists => accumulating detection" );
		op.process(src.data, tmp);
		//op.traverseChannel(src.data.getChannel(0), tmp.getChannel(0));
		mout.debug2("updating dst image" );
		dstData.data.getChannel(0).setPhysicalRange({0,1}, true);
		tmp.getChannel(0).setPhysicalRange({0,1}, true);
		BinaryFunctorOp<drain::MultiplicationFunctor>().traverseChannel(dstData.data, tmp, dstData.data);
		// File::write(dstData.data, feature+".png");
		dstData.odim.prodpar += ',';
		dstData.odim.prodpar += feature;
	}

	/// Debugging: save intermediate images.
	// mout.special(outputDataVerbosity);

	//if (outputDataVerbosity >= 1){
	if (outputDataVerbosity){ // bool (isSet()

		mout.special<LOG_DEBUG>("saving [", feature, "]");
		PlainData<PolarDst> & dstFeature = dstProductAux.getQualityData(feature);  // consider direct instead of copy?
		dstFeature.odim.quantity = feature;
		const QuantityMap & qm = getQuantityMap();
		qm.setQuantityDefaults(dstFeature, "PROB");
		if (NEW)
			drain::image::CopyOp().process(dstData.data, dstFeature.data);
		else
			drain::image::CopyOp().process(tmp, dstFeature.data);
		//@ dstFeature.updateTree();
	}
	*/

}

void FuzzyDualPolOp::computeFuzzyDBZ(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {

	RadarFunctorOp<FuzzyBell<double> > dbzFuzzifier;
	dbzFuzzifier.odimSrc = srcData.odim;
	dbzFuzzifier.functor.set(dbzPeak, +5.0);
	applyOperator(dbzFuzzifier, "FUZZY_DBZH_LOW", srcData, dstData, dstProduct);

};

void FuzzyDualPolOp::computeFuzzyVRAD(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {

	FuzzyStep<double> fuzzyStep; //(0.5);
	//const double pos = vradDevMin; ///vradSrc.odim.NI; // TODO: consider relative value directly as parameter NO! => alarm if over +/- 0.5

	if (!VRAD_FLIP)
		fuzzyStep.set(vradDevRange.min, vradDevRange.max);
	else
		fuzzyStep.set(vradDevRange.max, vradDevRange.min);

	DopplerDevWindow::conf_t conf(fuzzyStep, window.frame.width, window.frame.height, 0.05, true, false); // require 5% samples
	conf.updatePixelSize(srcData.odim);
	SlidingWindowOp<DopplerDevWindow> vradDevOp(conf);

	//mout.warn("fuzzy step: " , fuzzyStep  );
	/*
	mout.debug2("VRAD op   " , vradDevOp );
	mout.debug(vradDevOp.conf.frame.width  , 'x' , vradDevOp.conf.frame.height );
	//mout.debug(vradDevOp.conf.ftor );
	mout.special("ftor params: " , vradDevOp.conf.functorParameters );
	mout.debug("vradSrc NI=" , vradSrc.odim.getNyquist() );
	mout.debug2("vradSrc props:" , vradSrc.data.getProperties() );
	*/
	//  tmp,
	applyOperator(vradDevOp, "FUZZY_VRADH_DEV", srcData, dstData, dstProduct);


};

void FuzzyDualPolOp::computeFuzzyZDR(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	RadarFunctorOp<FuzzyTriangle<double> > zdrFuzzifier;
	zdrFuzzifier.odimSrc = srcData.odim;
	zdrFuzzifier.functor.set(+zdrAbsMin, 0.0, -zdrAbsMin); // INVERSE //, -1.0, 1.0);
	// zdrFuzzifier.functor.set(0.5, 2.0, 255);
	// mout.debug("ZDR_NONZERO" , zdrFuzzifier.functor );
	// zdrFuzzifier.getParameters()
	applyOperator(zdrFuzzifier,  "FUZZY_ZDR_NONZERO", srcData, dstData, dstProduct);
};

void FuzzyDualPolOp::computeFuzzyRHOHV(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	RadarFunctorOp<FuzzyStep<double> > rhohvFuzzifier;
	rhohvFuzzifier.odimSrc = srcData.odim;
	//rhohvFuzzifier.functor.set(rhoHVmax+(1.0-rhoHVmax)/2.0, rhoHVmax);
	rhohvFuzzifier.functor.set(rhoHVRange.max, rhoHVRange.min);
	// mout.debug("RHOHV_LOW" , rhohvFuzzifier.functor );
	applyOperator(rhohvFuzzifier, "FUZZY_RHOHV_LOW", srcData, dstData, dstProduct);
};


// processDataSet
void FuzzyDualPolOp::runDetection(const DataSet<PolarSrc> & sweepSrc, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.debug3("start"); //

	/*
	 *  Notice: all the detection fields should be of quantity:
	 *  	FUZZY_${quantity}_
	 */


	// Image tmp(typeid(unsigned char));
	// tmp.setPhysicalRange(0.0, 1.0, true);

	const QuantityMap & qm = getQuantityMap();

	/// Reduction coefficient to compensate missing measurement data
	double overallScale = 1.0;

	const Data<PolarSrc> & srcDataDBZ = sweepSrc.getData(qm.DBZ.keySelector); // VolumeOpNew::
	const bool DBZ = !srcDataDBZ.data.isEmpty();  // or: || dbzParams.empty() ?
	if (!DBZ){
		mout.warn("DBZ data missing, selector: ", qm.DBZ.keySelector);
		overallScale *= 0.75;
	}
	else {
		computeFuzzyDBZ(srcDataDBZ, dstData, dstProductAux);
	}

	const Data<PolarSrc> &  srcDataVRAD = sweepSrc.getData(qm.VRAD.keySelector); // VolumeOpNew::
	const bool VRAD = !srcDataVRAD.data.isEmpty();
	const double NI = srcDataVRAD.odim.getNyquist();
	if (!VRAD){
		mout.warn("VRAD missing, skipping..." );
		overallScale *= 0.5;
	}
	else if (NI == 0) { //  if (vradSrc.odim.NI == 0) {
		mout.note("vradSrc.odim (encoding): " , EncodingODIM(srcDataVRAD.odim) );
		mout.warn("vradSrc.odim.NI==0, and could not derive NI from encoding" );
		mout.warn("skipping VRAD..." );
		overallScale *= 0.5;
	}
	else if (vradDevRange.min > NI) {
			mout.warn("vradDev range (" , vradDevRange , ") exceeds NI of input: " , NI ); // semi-fatal
			mout.warn("skipping VRAD..." );
			overallScale *= 0.5;
	}
	else {
		if (vradDevRange.max > NI) {
			mout.warn("threshold end point of vradDev (" , vradDevRange , ") exceeds NI of input: " , NI );
		}
		computeFuzzyVRAD(srcDataVRAD, dstData, dstProductAux);
	}



	const Data<PolarSrc> &  srcDataRHOHV = sweepSrc.getData(qm.RHOHV.keySelector); // VolumeOpNew::
	const bool RHOHV = !srcDataRHOHV.data.isEmpty();
	if (!RHOHV){
		overallScale *= 0.5;
		mout.warn("RHOHV missing, selector: ", qm.RHOHV.keySelector);
	}
	else {
		computeFuzzyRHOHV(srcDataRHOHV, dstData, dstProductAux);
	}

	const Data<PolarSrc> &  srcDataZDR = sweepSrc.getData(qm.ZDR.keySelector); // VolumeOpNew::
	const bool ZDR = !srcDataZDR.data.isEmpty();
	if (!ZDR){
		overallScale *= 0.75;
		mout.warn("ZDR missing" );
	}
	else {
		// mout.debug2(zdrSrc.odim );
		computeFuzzyZDR(srcDataZDR, dstData, dstProductAux);
	}

	mout.debug("Overall scale " , overallScale );
	mout.success(" -> dstData: " , dstData );

	if (dstData.data.isEmpty()){
		mout.error("dstData still empty! ? Could not find input data; quantity=", dataSelector.getQuantity());
	}
	else {
		if (overallScale < 1.0){
			mout.warn("Input(s) missing, rescaling with overall scale " , overallScale );
			dstData.data.getScaling().scale *= overallScale;
			dstData.data.getScaling().offset *= overallScale;
			dstData.odim.scaling.setScaling(dstData.data.getScaling());
		}
		//FunctorOp<FuzzyBell<double,double> > fuzzyBright(0.0,-8.0, dstData.odim.scaleInverse(overallScale));
		//FuzzyBellOp fuzzyBright(0.0,-0.032, overallScale);
		//UnaryFunctorOp<ScalingFunctor<double> > fuzzyBright;
		//fuzzyBright.functor.set(0.0, -overallScale/2.0);
		//fuzzyBright.functor.set(0.0,-0.032, overallScale);
		// UnaryFunctorOp<FuzzyStep<double> > fuzzyBright;
		// fuzzyBright.functor.set(overallScale, 0.0);
		//fuzzyBright.process(dstData.data, dstData.data);
	}
	writeHow(dstData);
	//DataTools::updateInternalAttributes(dstData.getTree()); // needed?
	mout.success(" -> dstData: " , dstData );

	/*
	const Data<PolarSrc> &  wradSrc = sweepSrc.getData("WRAD"); // VolumeOpNew::
	const bool WRAD = !wradSrc.data.isEmpty();
	if (!WRAD)
		mout.warn("WRAD missing" );
	else {

		RadarDataFuzzifier<FuzzyStep<double,double> > wradFuzzifier;
		wradFuzzifier.odimSrc = wradSrc.odim;
		wradFuzzifier.functor.set(wradMin*0.75*wradSrc.odim.NI, wradMin*1.25*wradSrc.odim.NI, 255);
		applyOperator(wradFuzzifier, tmp, "WRAD_HIGH", wradSrc, dstData, dstProductAux);

		/// NOT NEEDED?
		SlidingWindowOpT<RadarWindowStdDev<FuzzyStep<double,double> > > wradDevFuzzifier;
		const int w = static_cast<int>(window.frame.width/wradSrc.odim.rscale);
		const int h = static_cast<double>(wradSrc.odim.geometry.height) * window.frame.height/360.0;
		wradDevFuzzifier.window.setSize(w, h);
		wradDevFuzzifier.window.functor.set( 0.5, 0.95, 255.0);
		wradDevFuzzifier.window.odimSrc = wradSrc.odim;
		applyOperator(wradDevFuzzifier, tmp, "WRAD_DEV", wradSrc, dstData, dstProductAux);

	}
	*/

}




// kludge
/*
void BirdOp::init(double dbzPeak, double vradDevMin, double rhoHVmax, double zdrAbsMin, double windowWidth, double windowHeight) {

	parameters.link("dbzPeak",      this->dbzPeak = dbzPeak,  "Typical reflectivity (DBZH)");
	parameters.link("vradDevMin",   this->vradDevRange.tuple(0.0, vradDevMin), "Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)");
	parameters.link("rhoHVmax",     this->rhoHVRange.tuple(0.9*rhoHVmax, rhoHVmax),  "Fuzzy threshold of maximum rhoHV value");
	parameters.link("zdrAbsMin",    this->zdrAbsMin = zdrAbsMin,  "Fuzzy threshold of absolute ZDR");
	parameters.link("windowWidth",  this->window.frame.width = windowWidth,  "window width, beam-directional (m)"); //, "[m]");
	parameters.link("windowHeight", this->window.frame.height = windowHeight,  "window width, azimuthal (deg)"); //, "[d]");

	// parameters.link("wradMin", this->wradMin, wradMin, "Minimum normalized deviation of within-bin Doppler speed deviation (WRAD)");

}
*/

/*
void InsectOp::init(double dbzPeak, double vradDevMax, double rhoHVmax, double zdrAbsMin, double windowWidth, double windowHeight) {

	// THIS IS INVERTED (wrt. BIRD)
	//VRAD_FLIP=true;

	parameters.link("dbzPeak",      this->dbzPeak = dbzPeak, "Typical reflectivity (DBZH)");
	parameters.link("vradDevMax",   this->vradDevRange.tuple(0, vradDevMax), "Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)");
	parameters.link("rhoHVmax",     this->rhoHVRange.tuple(0.9*rhoHVmax, rhoHVmax), "Fuzzy threshold of maximum rhoHV value");
	parameters.link("zdrAbsMin",    this->zdrAbsMin = zdrAbsMin, "Fuzzy threshold of absolute ZDR");
	parameters.link("windowWidth",  this->window.frame.width = windowWidth, "window width, beam-directional (m)"); //, "[m]");
	parameters.link("windowHeight", this->window.frame.height = windowHeight, "window width, azimuthal (deg)"); //, "[d]");

}
*/

}

// Rack
