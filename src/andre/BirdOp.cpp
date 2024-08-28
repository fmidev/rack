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
#include "drain/imageops/SlidingWindowOp.h"
#include "radar/Analysis.h"
#include "radar/Doppler.h"
#include "drain/util/FunctorPack.h"
#include "drain/util/Fuzzy.h"
//#include "drain/util/RegExp.h"


using namespace drain;
using namespace drain::image;

namespace rack {


/*
void BirdOp::applyOperator(const ImageOp & op, const std::string & feature, const Data<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {
	Image & tmp = dstProductAux.getQualityData(feature);
}
*/

/* Rename
 *
 *
 */
void GliderOp::applyOperator(const ImageOp & op, Image & tmp, const std::string & feature, const Data<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

	drain::Logger mout(__FUNCTION__, getName() + "::"+feature);

	mout.debug("running " , feature , '/' , op.getName() );

	const bool NEW = dstData.odim.prodpar.empty();  // or tmp.empty()

	//Channel & channel = dstData.data;
	//channel.properties.updateFromMap(dstData.data.getProperties());
	//mout.success(feature , " -> dstData: " , dstData );


	/// Save directly to target (dstData), if this is the first applied detector
	if (NEW){
		mout.debug2("creating dst image" );
		//dstData.setPhysicalRange(0.0, 1.0);
		dstData.setPhysicalRange(0.0, 1.0);
		op.traverseChannel(src.data, dstData.data);
		dstData.odim.prodpar = feature;
		//tmp.copyShallow(dstData.data);
		tmp.setGeometry(dstData.data.getGeometry());
		// mout.success("dstData: " , dstData );
		// tmp.initialize(dstData.data.getType(), dstData.data.getGeometry());
		// tmp.adoptScaling(dstData.data);
	}
	else {
		mout.debug2("tmp exists => accumulating detection" );
		op.process(src.data, tmp);
		//op.traverseChannel(src.data.getChannel(0), tmp.getChannel(0));
		mout.debug2("updating dst image" );
		dstData.data.getChannel(0).setPhysicalRange({0,1}, true);
		tmp.getChannel(0).setPhysicalRange({0,1}, true);
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

// processDataSet
void GliderOp::runDetection(const DataSet<PolarSrc> & sweepSrc, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.debug3("start"); //

	//mout.error(dstData ); //

	//dstData.setPhysicalRange(0.0, 1.0);

	Image tmp(typeid(unsigned char));
	tmp.setPhysicalRange(0.0, 1.0, true);
	//tmp.setOptimalScale();
	//tmp.adoptScaling(dstData.data);
	// 2018 ? tmp.scaling.setLimits(dstData.data.getMin<double>(), dstData.data.getMax<double>());

	// const double MAX = dstData.data.getEncoding().getTypeMax<double>(); //dstData.odim.scaleInverse(1);

	/// Reduction coefficient to compensate missing measurement data
	double overallScale = 1.0;

	const Data<PolarSrc> & dbzSrc = sweepSrc.getData("DBZH"); // VolumeOpNew::
	const bool DBZ = !dbzSrc.data.isEmpty();  // or: || dbzParams.empty() ?
	if (!DBZ){
		mout.warn("DBZH missing" );
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
		mout.debug("DBZ_LOW" , dbzFuzzifier.functor );

		applyOperator(dbzFuzzifier, tmp, "DBZ_LOW", dbzSrc, dstData, dstProductAux);

	}

	//static drain::RegExp regExpVRAD("^VRAD[H]?$");
	static const KeySelector selectVRAD("^VRAD[H]?$");
	const Data<PolarSrc> &  vradSrc = sweepSrc.getData(selectVRAD); // VolumeOpNew::
	const bool VRAD = !vradSrc.data.isEmpty();
	const double NI = vradSrc.odim.getNyquist();
	if (!VRAD){
		mout.warn("VRAD missing, skipping..." );
		overallScale *= 0.5;
	}
	else if (NI == 0) { //  if (vradSrc.odim.NI == 0) {
		mout.note("vradSrc.odim (encoding): " , EncodingODIM(vradSrc.odim) );
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

		FuzzyStep<double> fuzzyStep; //(0.5);
		//const double pos = vradDevMin; ///vradSrc.odim.NI; // TODO: consider relative value directly as parameter NO! => alarm if over +/- 0.5

		if (!VRAD_FLIP)
			fuzzyStep.set(vradDevRange.min, vradDevRange.max);
		else
			fuzzyStep.set(vradDevRange.max, vradDevRange.min);

		/*
		if (vradDev.first <= vradDev.second)
			fuzzyStep.set(vradDev.first, vradDev.second, MAX);
			//fuzzyStep.set( 0.8*pos, 1.2*pos, MAX); // 255.0 ); ?? 2019/11
		else
			fuzzyStep.set(vradDev.second, vradDev.first, MAX);
			//fuzzyStep.set( 1.2*(-pos), 0.8*(-pos), MAX );
		 */
		// RadarWindowConfig:
		// int widthM=1500, double heightD=3.0, double contributionThreshold = 0.5, bool invertPolar=false, bool relativeScale=false
		DopplerDevWindow::conf_t conf(fuzzyStep, window.frame.width, window.frame.height, 0.05, true, false); // require 5% samples
		conf.updatePixelSize(vradSrc.odim);
		SlidingWindowOp<DopplerDevWindow> vradDevOp(conf);

		//mout.warn("fuzzy step: " , fuzzyStep  );
		mout.debug2("VRAD op   " , vradDevOp );
		mout.debug(vradDevOp.conf.frame.width  , 'x' , vradDevOp.conf.frame.height );
		//mout.debug(vradDevOp.conf.ftor );
		mout.special("ftor params: " , vradDevOp.conf.functorParameters );
		mout.debug("vradSrc NI=" , vradSrc.odim.getNyquist() );
		mout.debug2("vradSrc props:" , vradSrc.data.getProperties() );

		/*
		dstData.data.setOptimalScale(0.0, 1.0);
		dstData.odim.scale   = dstData.data.getScaling().scale;
		dstData.odim.offset = dstData.data.getScaling().offset;
		*/

		applyOperator(vradDevOp, tmp, "VRAD_DEV", vradSrc, dstData, dstProductAux);
		//mout.debug(dstData.data );
		//mout.debug(EncodingODIM(dstData.odim) );

	}

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


	const Data<PolarSrc> &  rhohvSrc = sweepSrc.getData("RHOHV"); // VolumeOpNew::
	const bool RHOHV = !rhohvSrc.data.isEmpty();
	if (!RHOHV){
		overallScale *= 0.5;
		mout.warn("RHOHV missing" );
	}
	else {

		//RadarFunctorOp<FuzzyStep<double> > rhohvFuzzifier;
		RadarFunctorOp<FuzzyStep<double> > rhohvFuzzifier;
		rhohvFuzzifier.odimSrc = rhohvSrc.odim;
		//rhohvFuzzifier.functor.set(rhoHVmax+(1.0-rhoHVmax)/2.0, rhoHVmax);
		rhohvFuzzifier.functor.set(rhoHVRange.max, rhoHVRange.min);
		mout.debug("RHOHV_LOW" , rhohvFuzzifier.functor );
		applyOperator(rhohvFuzzifier, tmp, "RHOHV_LOW", rhohvSrc, dstData, dstProductAux);

	}

	const Data<PolarSrc> &  zdrSrc = sweepSrc.getData("ZDR"); // VolumeOpNew::
	const bool ZDR = !zdrSrc.data.isEmpty();
	if (!ZDR){
		overallScale *= 0.75;
		mout.warn("ZDR missing" );
	}
	else {

		mout.debug2(zdrSrc.odim );

		// tmp.setPhysicalScale(0.0, 1.0);

		//RadarDataFuzzifier<FuzzyStep<double,double> > zdrFuzzifier;
		RadarFunctorOp<FuzzyTriangle<double> > zdrFuzzifier;
		zdrFuzzifier.odimSrc = zdrSrc.odim;
		zdrFuzzifier.functor.set(+zdrAbsMin, 0.0, -zdrAbsMin); // INVERSE //, -1.0, 1.0);
		//zdrFuzzifier.functor.set(0.5, 2.0, 255);
		mout.debug("ZDR_NONZERO" , zdrFuzzifier.functor );
		applyOperator(zdrFuzzifier, tmp, "ZDR_NONZERO", zdrSrc, dstData, dstProductAux);
		//applyOperator(zdrFuzzifier, tmp, "ZDR_HIGH", zdrSrc, dstData, dstProductAux);

		//mout.warn("ZDR_NONZERO" , zdrFuzzifier.functor );
		//mout.warn("ZDR_NONZERO" , tmp );

		// File::write(tmp, "ZDR_NONZERO.png");

	}

	mout.debug("Overall scale " , overallScale );
	mout.success(" -> dstData: " , dstData );

	if (dstData.data.isEmpty()){
		mout.error("could not find input data; quantity=", dataSelector.getQuantity());
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
}


// kludge
void BirdOp::init(double dbzPeak, double vradDevMin, double rhoHVmax, double zdrAbsMin, double windowWidth, double windowHeight) {

	parameters.link("dbzPeak", this->dbzPeak = dbzPeak, "Typical reflectivity (DBZH)");

	parameters.link("vradDevMin", this->vradDevRange.tuple(), "Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)");
	this->vradDevRange.min = 0; //0.9 * vradDevMin;
	this->vradDevRange.max = 1.0 * vradDevMin;

	// parameters.link("wradMin", this->wradMin, wradMin, "Minimum normalized deviation of within-bin Doppler speed deviation (WRAD)");
	parameters.link("rhoHVmax", this->rhoHVRange.tuple(), "Fuzzy threshold of maximum rhoHV value");
	this->rhoHVRange.min = 0.9 * rhoHVmax;
	this->rhoHVRange.max = rhoHVmax;
	//parameters.link("rhoHVmax", this->rhoHVmax = rhoHVmax, "Maximum rhoHV value (fuzzy)");

	parameters.link("zdrAbsMin", this->zdrAbsMin = zdrAbsMin, "Fuzzy threshold of absolute ZDR");

	parameters.link("windowWidth", this->window.frame.width = windowWidth, "window width, beam-directional (m)"); //, "[m]");
	parameters.link("windowHeight", this->window.frame.height = windowHeight, "window width, azimuthal (deg)"); //, "[d]");

}

void InsectOp::init(double dbzPeak, double vradDevMax, double rhoHVmax, double zdrAbsMin, double windowWidth, double windowHeight) {

	parameters.link("dbzPeak", this->dbzPeak = dbzPeak, "Typical reflectivity (DBZH)");

	// THIS IS INVERTED (wrt. BIRD)
	VRAD_FLIP=true;
	parameters.link("vradDevMax", this->vradDevRange.tuple(), "Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)");
	this->vradDevRange.min = 0; //0.9 * vradDevMax;
	this->vradDevRange.max = 1.0 * vradDevMax;

	// parameters.link("wradMin", this->wradMin, wradMin, "Minimum normalized deviation of within-bin Doppler speed deviation (WRAD)");
	parameters.link("rhoHVmax", this->rhoHVRange.tuple(), "Fuzzy threshold of maximum rhoHV value");
	this->rhoHVRange.min = 0.9 * rhoHVmax;
	this->rhoHVRange.max = rhoHVmax;
	//parameters.link("rhoHVmax", this->rhoHVmax = rhoHVmax, "Maximum rhoHV value (fuzzy)");

	parameters.link("zdrAbsMin", this->zdrAbsMin = zdrAbsMin, "Fuzzy threshold of absolute ZDR");

	parameters.link("windowWidth", this->window.frame.width = windowWidth, "window width, beam-directional (m)"); //, "[m]");
	parameters.link("windowHeight", this->window.frame.height = windowHeight, "window width, azimuthal (deg)"); //, "[d]");

}


}

// Rack
