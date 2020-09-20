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
#include <data/DataSelector.h>
#include <data/DataTools.h>
#include <data/PolarODIM.h>
#include <data/QuantityMap.h>
#include "drain/image/ImageChannel.h"
#include "drain/image/ImageLike.h"
#include "drain/image/Window.h"
#include "drain/imageops/CopyOp.h"
#include "drain/imageops/FunctorOp.h"
#include "drain/imageops/SlidingWindowOp.h"
#include <radar/Analysis.h>
#include <radar/Doppler.h>
#include "drain/util/FunctorPack.h"
#include "drain/util/Fuzzy.h"
#include "drain/util/Log.h"
#include "drain/util/RegExp.h"


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
void BiometeorOp::applyOperator(const ImageOp & op, Image & tmp, const std::string & feature, const Data<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

	drain::Logger mout(__FUNCTION__, getName() + "::"+feature);

	mout.debug() << "running " << feature << '/' << op.getName() << mout.endl;

	const bool NEW = dstData.odim.prodpar.empty();  // or tmp.empty()

	//Channel & channel = dstData.data;
	//channel.properties.updateFromMap(dstData.data.getProperties());

	/// Save directly to target (dstData), if this is the first applied detector
	if (NEW){
		mout.debug(1) << "creating dst image" << mout.endl;
		//dstData.setPhysicalRange(0.0, 1.0);
		dstData.setPhysicalRange(0.0, 1.0);
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

void BiometeorOp::processDataSet(const DataSet<PolarSrc> & sweepSrc, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

	drain::Logger mout(__FUNCTION__, __FILE__);
	mout.debug(2) << "start" <<  mout.endl; //

	//mout.error() << dstData <<  mout.endl; //

	//dstData.setPhysicalRange(0.0, 1.0);

	Image tmp(typeid(unsigned char));
	tmp.setPhysicalScale(0.0, 1.0);
	//tmp.adoptScaling(dstData.data);
	// 2018 ? tmp.scaling.setLimits(dstData.data.getMin<double>(), dstData.data.getMax<double>());

	// const double MAX = dstData.data.getEncoding().getTypeMax<double>(); //dstData.odim.scaleInverse(1);

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
	else if (vradDev.min > NI) {
			mout.warn() << "vradDev range (" << vradDev << ") exceeds NI of input: " << NI << mout.endl; // semi-fatal
			mout.warn() << "skipping VRAD..." <<  mout.endl;
			overallScale *= 0.5;
	}
	else {
		if (vradDev.max > NI) {
			mout.warn() << "threshold end point of vradDev (" << vradDev << ") exceeds NI of input: " << NI << mout.endl;
		}

		FuzzyStep<double> fuzzyStep; //(0.5);
		//const double pos = vradDevMin; ///vradSrc.odim.NI; // TODO: consider relative value directly as parameter NO! => alarm if over +/- 0.5

		if (!VRAD_FLIP)
			fuzzyStep.set(vradDev.min, vradDev.max);
		else
			fuzzyStep.set(vradDev.max, vradDev.min);

		/*
		if (vradDev.min <= vradDev.max)
			fuzzyStep.set(vradDev.min, vradDev.max, MAX);
			//fuzzyStep.set( 0.8*pos, 1.2*pos, MAX); // 255.0 ); ?? 2019/11
		else
			fuzzyStep.set(vradDev.max, vradDev.min, MAX);
			//fuzzyStep.set( 1.2*(-pos), 0.8*(-pos), MAX );
		 */
		// RadarWindowConfig:
		// int widthM=1500, double heightD=3.0, double contributionThreshold = 0.5, bool invertPolar=false, bool relativeScale=false
		DopplerDevWindow::conf_t conf(fuzzyStep, window.width, window.height, 0.05, true, false); // require 5% samples
		conf.updatePixelSize(vradSrc.odim);
		SlidingWindowOp<DopplerDevWindow> vradDevOp(conf);

		//mout.warn() << "fuzzy step: " << fuzzyStep  <<  mout.endl;
		mout.debug(1) << "VRAD op   " << vradDevOp <<  mout.endl;
		mout.debug()  << vradDevOp.conf.width  << 'x' << vradDevOp.conf.height <<  mout.endl;
		mout.debug()  << vradDevOp.conf.ftor <<  mout.endl;
		mout.debug()  << "vradSrc NI=" << vradSrc.odim.getNyquist() <<  mout.endl;
		mout.debug(1) << "vradSrc props:" << vradSrc.data.getProperties() <<  mout.endl;

		/*
		dstData.data.setOptimalScale(0.0, 1.0);
		dstData.odim.scale   = dstData.data.getScaling().scale;
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
		const int w = static_cast<int>(window.width/wradSrc.odim.rscale);
		const int h = static_cast<double>(wradSrc.odim.geometry.height) * window.height/360.0;
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
		//rhohvFuzzifier.functor.set(rhoHVmax+(1.0-rhoHVmax)/2.0, rhoHVmax);
		rhohvFuzzifier.functor.set(rhoHV.max, rhoHV.min);
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

		// tmp.setPhysicalScale(0.0, 1.0);

		//RadarDataFuzzifier<FuzzyStep<double,double> > zdrFuzzifier;
		RadarFunctorOp<FuzzyTriangle<double> > zdrFuzzifier;
		zdrFuzzifier.odimSrc = zdrSrc.odim;
		zdrFuzzifier.functor.set(+zdrAbsMin, 0.0, -zdrAbsMin); // INVERSE //, -1.0, 1.0);
		//zdrFuzzifier.functor.set(0.5, 2.0, 255);
		mout.debug() << "ZDR_NONZERO" << zdrFuzzifier.functor << mout.endl;
		applyOperator(zdrFuzzifier, tmp, "ZDR_NONZERO", zdrSrc, dstData, dstProductAux);
		//applyOperator(zdrFuzzifier, tmp, "ZDR_HIGH", zdrSrc, dstData, dstProductAux);

		//mout.warn() << "ZDR_NONZERO" << zdrFuzzifier.functor << mout.endl;
		//mout.warn() << "ZDR_NONZERO" << tmp << mout.endl;

		// File::write(tmp, "ZDR_NONZERO.png");

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

	writeHow(dstData);
	DataTools::updateInternalAttributes(dstData.getTree()); // needed?

}


// kludge
void BirdOp::init(double dbzPeak, double vradDevMin, double rhoHVmax, double zdrAbsMin, double windowWidth, double windowHeight) {

	parameters.reference("dbzPeak", this->dbzPeak = dbzPeak, "Typical reflectivity (DBZH)");

	parameters.reference("vradDevMin", this->vradDev.vect, "Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)");
	this->vradDev.min = 0; //0.9 * vradDevMin;
	this->vradDev.max = 1.0 * vradDevMin;

	// parameters.reference("wradMin", this->wradMin, wradMin, "Minimum normalized deviation of within-bin Doppler speed deviation (WRAD)");
	parameters.reference("rhoHVmax", this->rhoHV.vect, "Fuzzy threshold of maximum rhoHV value");
	this->rhoHV.min = 0.9 * rhoHVmax;
	this->rhoHV.max = rhoHVmax;
	//parameters.reference("rhoHVmax", this->rhoHVmax = rhoHVmax, "Maximum rhoHV value (fuzzy)");

	parameters.reference("zdrAbsMin", this->zdrAbsMin = zdrAbsMin, "Fuzzy threshold of absolute ZDR");

	parameters.reference("windowWidth", this->window.width = windowWidth, "window width, beam-directional (m)"); //, "[m]");
	parameters.reference("windowHeight", this->window.height = windowHeight, "window width, azimuthal (deg)"); //, "[d]");

}

void InsectOp::init(double dbzPeak, double vradDevMax, double rhoHVmax, double zdrAbsMin, double windowWidth, double windowHeight) {

	parameters.reference("dbzPeak", this->dbzPeak = dbzPeak, "Typical reflectivity (DBZH)");

	// THIS IS INVERTED (wrt. BIRD)
	VRAD_FLIP=true;
	parameters.reference("vradDevMax", this->vradDev.vect, "Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)");
	this->vradDev.min = 0; //0.9 * vradDevMax;
	this->vradDev.max = 1.0 * vradDevMax;

	// parameters.reference("wradMin", this->wradMin, wradMin, "Minimum normalized deviation of within-bin Doppler speed deviation (WRAD)");
	parameters.reference("rhoHVmax", this->rhoHV.vect, "Fuzzy threshold of maximum rhoHV value");
	this->rhoHV.min = 0.9 * rhoHVmax;
	this->rhoHV.max = rhoHVmax;
	//parameters.reference("rhoHVmax", this->rhoHVmax = rhoHVmax, "Maximum rhoHV value (fuzzy)");

	parameters.reference("zdrAbsMin", this->zdrAbsMin = zdrAbsMin, "Fuzzy threshold of absolute ZDR");

	parameters.reference("windowWidth", this->window.width = windowWidth, "window width, beam-directional (m)"); //, "[m]");
	parameters.reference("windowHeight", this->window.height = windowHeight, "window width, azimuthal (deg)"); //, "[d]");

}


}

// Rack
