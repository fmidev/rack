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

#include <drain/Log.h>
#include <drain/util/Fuzzy.h>
#include <drain/util/FunctorPack.h>
#include <drain/util/Output.h>
#include <drain/image/ImageChannel.h>
#include <drain/image/ImageLike.h>
#include <drain/image/Window.h>
#include <drain/imageops/CopyOp.h>
#include <drain/imageops/FunctorOp.h>
#include <drain/imageops/SlidingWindowOp.h>

#include "data/DataSelector.h"
// #include "data/DataTools.h"
// #include "data/PolarODIM.h"
#include "data/QuantityMap.h"
#include "radar/Doppler.h"

#include "FuzzyDetectorOp.h"




namespace rack {

const QuantitySelector FuzzyDetectorOp::selectorEmpty;
const QuantitySelector FuzzyDetectorOp::selectorDBZ("DBZH", "DBZ", "DBZV", "DBTH", "DBTV");
const QuantitySelector FuzzyDetectorOp::selectorVRAD("VRADH", "VRAD", "VRADV");
const QuantitySelector FuzzyDetectorOp::selectorZDR("ZDR");
const QuantitySelector FuzzyDetectorOp::selectorRHOHV("RHOHV");

drain::FuzzyIdentity<double> FuzzyDetectorOp::dummy;

class MyOp: public RadarFunctorBase, public drain::image::ImageOp {

public:

	const drain::UnaryFunctor & ftor;

	// Kind of a wrapper
	MyOp(const drain::UnaryFunctor & ftor) : drain::image::ImageOp(ftor.getName(), ftor.getDescription()), ftor(ftor),
			value(ftor.getParameters().getValues()) {
			//value(drain::sprinter(ftor.getParameters().getValues()).str()) {
		parameters.link("value", value);
	}

	virtual inline
    void traverseChannel(const Channel & src, Channel &dst) const override {
		apply(src, dst, ftor, true);
	};

	std::string value;

	// op.traverseChannel(srcData.data, dstFeature.data);
};


void FuzzyDetectorOp::computeFuzzy(const drain::Fuzzifier<double> & fuzzifier, const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	// RadarFunctorBaseOp & fuzzifier = getDBZFuzzifier();
	// fuzzifier.odimSrc.updateFromCastableMap(srcData.odim);
	//RadarFunctorBaseOp & f;

	MyOp op(fuzzifier);
	op.odimSrc = srcData.odim;

	applyOperator(op, srcData, dstData, dstProduct);

	//drain::FunctorBank & functorBank = drain::getFunctorBank();

};

void FuzzyDetectorOp::computeFuzzyVRAD(const drain::Fuzzifier<double> & fuzzifier, const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {

	DopplerDevWindow::conf_t conf(fuzzifier, windowConf.frame.width, windowConf.frame.height, 0.05, true, false); // require 5% samples
	conf.updatePixelSize(srcData.odim);
	drain::image::SlidingWindowOp<DopplerDevWindow> vradDevOp(conf);
	applyOperator(vradDevOp, srcData, dstData, dstProduct);

};


/*
void FuzzyDetectorOp::computeFuzzyDBZ(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	RadarFunctorBaseOp & fuzzifier = getDBZFuzzifier();
	fuzzifier.odimSrc.updateFromCastableMap(srcData.odim);
	applyOperator(fuzzifier, srcData, dstData, dstProduct);
};
*/

void FuzzyDetectorOp::getGammaLookUpTable(double p, std::vector<unsigned char> & lookUpTable){
	const size_t N = 256;
	const double scale = 1.0/255.0;
	lookUpTable.resize(N);
	const drain::GammaFunctor functor(p);
	for (size_t i=0; i<N; ++i){
		lookUpTable[i] = static_cast<unsigned char>(255.0 * functor(scale * static_cast<double>(i)));
	}
}




/* Rename
 *
 * Image & tmp,
 * const std::string & feature,
 */
void FuzzyDetectorOp::applyOperator(const ImageOp & op,  const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

	drain::Logger mout(__FUNCTION__, getName()); //  + "::"+feature

	const drain::StringBuilder<'_'> featureQuantityBrief("FUZZY", srcData.odim.quantity);
	const drain::StringBuilder<'_'> featureMethod(op.getName(), op.getParameters().getValues());
	const drain::StringBuilder<'_'> featureQuantity(featureQuantityBrief, featureMethod);

	//mout.attention("FUZZY...", feature, '-', op.getName(), '-', srcData.odim.quantity);
	mout.info("quantity: ", featureQuantity);

	if (mout.isDebug(2)){
		drain::Output output(featureQuantityBrief.str() + ".txt");
		const QuantityMap & qm = getQuantityMap();
		const Quantity & quantity = qm.get(srcData.odim.quantity);

		double f;
		const double coeff = 1.0/256.0;
		for (int i = 0; i<256; ++i){
			f = quantity.physicalRange.min + static_cast<double>(i)* coeff*quantity.physicalRange.span();
			output << i << '\t' << f;
		}
	}

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
		dstFeature.getHow()["method"] = featureMethod.str();
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
		//mout.attention("COPY:", srcData.data.getGeometry(), " feat:", dstFeature.data.getGeometry());
		dstData.setGeometry(srcData.data.getGeometry());
		// mout.attention("COP2:", dstData.data.getGeometry());
		drain::image::CopyOp().process(dstFeature.data, dstData.data);
		dstData.odim.prodpar = featureQuantityBrief;
		// mout.attention("COP3:", dstData.data.getGeometry());
	}
	else {
		// mout.attention("UPDATE");
		dstData.data.getChannel(0).setPhysicalRange({0,1}, true);
		dstFeature.data.getChannel(0).setPhysicalRange({0,1}, true);
		BinaryFunctorOp<drain::MultiplicationFunctor>().traverseChannel(dstData.data, dstFeature.data, dstData.data);
		// File::write(dstData.data, feature+".png");
		dstData.odim.prodpar += ',';
		dstData.odim.prodpar += featureQuantityBrief;
		//dstData.odim.prodpar = drain::StringBuilder<','>(dstData.odim.prodpar, feature);
	}


}



/*
 *  TODO: default quality for each missing input data
 */
void FuzzyDetectorOp::runDetection(const DataSet<PolarSrc> & sweepSrc, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.debug3("start"); //

	/*
	 *  All the detection fields will have quantity:
	 *  	FUZZY_${quantity}_
	 */

	if (getDataSelector().getQuantitySelector()){
		mout.debug("expected ", getDataSelector().getQuantitySelector().size(), " select:", getDataSelector().getQuantitySelector());
		mout.debug("received ", sweepSrc);
	}

	/// Reduction coefficient to compensate missing measurement data
	double overallScale = 1.0;

	// Todo: local destruction
	LocalFunctorBank functorBank;

	const QuantitySelector & dbzSelect = getSelectorDBZ();
	if (dbzSelect){
		const Data<PolarSrc> & srcDataDBZ = sweepSrc.getData(dbzSelect); // VolumeOpNew::
		if (srcDataDBZ.data){
			computeFuzzy(getFuzzifierDBZ(functorBank), srcDataDBZ, dstData, dstProductAux);
		}
		else {
			mout.warn("DBZ data missing, selector: ", dbzSelect);
			mout.warn("Sweep info: ", sweepSrc);
			overallScale *= 0.75;
		}
	}

	const QuantitySelector & selectVRAD = getSelectorVRAD();
	if (selectVRAD){
		const Data<PolarSrc> &  srcDataVRAD = sweepSrc.getData(selectVRAD); // VolumeOpNew::
		const double NI = srcDataVRAD.odim.getNyquist();
		if (!srcDataVRAD.data){
			mout.warn("VRAD data missing, selector: ", selectVRAD); // qm.VRAD.keySelector);
			mout.warn("Sweep info: ", sweepSrc);
			// mout.warn("VRAD missing, skipping..." );
			overallScale *= 0.5;
		}
		else if (NI == 0) { //  if (vradSrc.odim.NI == 0) {
			mout.note("vradSrc.odim (encoding): " , EncodingODIM(srcDataVRAD.odim) );
			mout.warn("vradSrc.odim.NI==0, and could not derive NI from encoding" );
			mout.warn("skipping VRAD..." );
			overallScale *= 0.5;
		}
		else if (vradDevThreshold > NI) {
			//else if (vradDevRange.min > NI) {
				//mout.warn("vradDev range (" , vradDevRange , ") exceeds NI of input: " , NI ); // semi-fatal
			mout.warn("vradDev threshold (" , vradDevThreshold , ") exceeds NI of input: " , NI ); // semi-fatal
			mout.warn("skipping VRAD..." );
			overallScale *= 0.5;
		}
		else {
			/*
			if (vradDevRange.max > NI) {
				mout.warn("threshold end point of vradDev (" , vradDevRange , ") exceeds NI of input: " , NI );
			}
			*/
			computeFuzzyVRAD(getFuzzifierVRAD(functorBank), srcDataVRAD, dstData, dstProductAux);
		}
	}
	else {
		mout.debug("This detector does not use VRAD");
	}


	const QuantitySelector & selectRHOHV = getSelectorRHOHV();
	if (selectRHOHV){
		const Data<PolarSrc> &  srcDataRHOHV = sweepSrc.getData(selectRHOHV); // VolumeOpNew::
		if (srcDataRHOHV.data){
			computeFuzzy(getFuzzifierRHOHV(functorBank), srcDataRHOHV, dstData, dstProductAux);
			// computeFuzzyRHOHV(srcDataRHOHV, dstData, dstProductAux);
		}
		else {
			overallScale *= 0.5;
			mout.warn("RHOHV missing, selector: ", selectRHOHV);
		}
	}
	else {
		mout.debug("This detector does not use RHOHV");
	}

	const QuantitySelector & selectZDR = getSelectorZDR();
	if (selectZDR){
		const Data<PolarSrc> &  srcDataZDR = sweepSrc.getData(selectZDR); // VolumeOpNew::

		if (srcDataZDR.data){
			computeFuzzy(getFuzzifierZDR(functorBank), srcDataZDR, dstData, dstProductAux);
		}
		else {
			overallScale *= 0.75;
			mout.warn("ZDR missing" );
			// mout.debug2(zdrSrc.odim );
			// computeFuzzyZDR(srcDataZDR, dstData, dstProductAux);
		}
	}
	else {
		mout.debug("This detector does not use ZDR");
	}

	mout.debug("Overall scale " , overallScale );
	mout.success(" -> dstData: " , dstData );

	if (dstData.data){
		if (overallScale < 1.0){ // TODO: rescale this to match with the number of input quantities; aNonMet
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
	else {
		mout.error("dstData still empty! ? Could not find input data; quantity=", dataSelector.getQuantity());
	}

	if (gammaAdjustment != 1.0){
		mout.special("applying gamma adjustment (p=", gammaAdjustment,")");
		std::vector<unsigned char> lookUp;
		getGammaLookUpTable(gammaAdjustment, lookUp); // todo: odim scaling included? Tiny difference...
		/*
		for (size_t i=0; i<256; ++i){
			mout.special(" look: ", i, " => ", (float)lookUp[i]);
		}
		*/

		//double d;
		for (drain::image::Image::iterator it = dstData.data.begin(); it!= dstData.data.end(); ++it){
			if (dstData.odim.isValue(static_cast<double>(*it))){
				*it = lookUp[*it];
			}
		}

		dstData.getHow()["gammaAdjustment"] = gammaAdjustment;

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

}

// Rack
