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

#include <drain/util/Fuzzy.h>
#include <drain/image/ImageFile.h>
#include <drain/imageops/SlidingWindowMedianOp.h>
#include "NonMetOp.h"
#include "FuzzyDetectorOp.h"


#include <drain/imageops/SlidingWindowOp.h>
#include "radar/Analysis.h"
#include "radar/Doppler.h"

namespace rack {


void NonMetOp::runDetector(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.attention(srcData.odim.quantity);
	// drain::FuzzyStepsoid<double, double> f(odimIn.scaleInverse(threshold), odimIn.scaleInverse(threshold + thresholdWidth) - odimIn.scaleInverse(threshold) ); BUG
	const unsigned int QMIN = dstProb.odim.scaleInverse(0.0);
	const unsigned int QMAX = dstProb.odim.scaleInverse(0.95);
	drain::FuzzyStep<double> fuzzyStep; //(threshold.max, threshold.min, QMAX);  // inverted
	if (threshold.min < threshold.max){
		fuzzyStep.set(threshold.max, threshold.min, QMAX);
	}
	else if (threshold.min == threshold.max){
			fuzzyStep.set(threshold.max, 0.9*threshold.max, QMAX);
	}
	else {
		mout.info("swapping min-max of ", threshold);
		fuzzyStep.set(threshold.min, threshold.max, QMAX);
	}

	//mout.special("fuzzy step:" , fuzzyStep );
	/*
	for (double d: {0.0, 0.1, 0.2, 0.5, 0.75, 1.0}){
		mout.special("fuzzy step:", d, " -> \t", fuzzyStep(d));
	}
	*/

	Image::const_iterator it = srcData.data.begin();
	Image::iterator dit = dstProb.data.begin();
	while (it != srcData.data.end()){
		if (srcData.odim.isValue(*it)){
			*dit = fuzzyStep(srcData.odim.scaleForward(*it));
		}
		else {
			*dit = 0.0;
		}
		++it; ++dit;
	}

	/// Median filtering imitating morphological closing
	//if ((windowWidth>0) && (windowHeight>0)){
	if (medianWindow.getArea() > 0.0){

		const CoordinatePolicy & coordPolicy = srcData.data.getCoordinatePolicy();
		dstProb.data.setCoordinatePolicy(coordPolicy);

		//Image tmp;
		//tmp.setCoordinatePolicy(coordPolicy);

		const int w = srcData.odim.getBeamBins(medianWindow.width);        // windowWidth / srcData.odim.rscale;
		const int h = srcData.odim.getAzimuthalBins(medianWindow.height);  // windowHeight * 360.0 / srcData.odim.geometry.height;

		SlidingWindowMedianOp median;
		median.setSize(w,h);
		median.conf.percentage = medianPos;
		mout.warn("median: " , median );
		//median.traverseChannel(dstProb.data.getChannel(0), dstProb.data.getChannel(0));
		//median.process(dstProb.data, tmp);
		//drain::image::FilePng::write(tmp, "mika.png");
		median.process(dstProb.data, dstProb.data);
		//drain::image::FilePng::write(dstProb.data, "sika.png");

		/*
		median.filter(dstProb.data, tmp);
		median.setSize(w*2,h*2);
		median.conf.percentage = 1.0 - medianPos;
		median.filter(tmp, dstProb.data);
		*/
	}

	/// Mask out median values spread to \c undetect regions
	it = srcData.data.begin();
	dit = dstProb.data.begin();
	while (it != srcData.data.end()){
		if (!srcData.odim.isValue(*it)){
			*dit = QMIN;
		}
		/*
		if (srcData.odim.isValue(*it)){
			*dit = (QMAX - static_cast<int>(*dit));
			// *dit = fuzzyStep(srcData.odim.scaleForward(*it));
			// *dit = 64 + fuzzyStep(srcData.odim.scaleForward(*it))/ 2;
		}
		else {
			*dit = 0.0;
		}
		*/
		++it; ++dit;
	}

}


NonMet2Op::NonMet2Op(const drain::UniTuple<double,2> & threshold, const drain::UniTuple<double,2> & medianWindow, double medianThreshold) :
				FuzzyDetectorOp(__FUNCTION__, "Estimates probability from DBZH, VRAD, RhoHV and ZDR.", "nonmet"){
	init(threshold, medianWindow, medianThreshold);
}

//void NonMet2Op::init(double dbzMax, double vradDevMax, double rhoHVmax, double zdrDevMin, double windowWidth, double windowHeight){
void NonMet2Op::init(const drain::UniTuple<double,2> & threshold, const drain::UniTuple<double,2> & medianWindow, double medianThreshold) {

	dataSelector.setQuantities("RHOHV");
	REQUIRE_STANDARD_DATA = false;
	UNIVERSAL = true;

	parameters.link("threshold", this->threshold.tuple() = threshold, "0...1[:0...1]");
	parameters.link("medianWindow", this->medianWindow.tuple() = medianWindow, "metres,degrees");
	parameters.link("medianPos", this->medianPos = medianPos, "0...1");
}

void NonMet2Op::computeFuzzyZDR(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {

}

void NonMet2Op::computeFuzzyRHOHV(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {

}



void ChaffOp::init(double dbzPeak, double vradDevMax, double rhoHVmax, double zdrAbsMin, double windowWidth, double windowHeight) {

	parameters.link("dbzPeak",     this->dbzParam = dbzPeak,  "Typical reflectivity (DBZH)");
	parameters.link("vradDevMax",  this->vradDevThreshold = vradDevMax, "Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)");
	parameters.link("rhoHVmax",    this->rhoHVthreshold = rhoHVmax,  "Fuzzy threshold of maximum rhoHV value");
	parameters.link("zdrAbsMin",   this->zdrAbsThreshold = zdrAbsMin,  "Fuzzy threshold of absolute ZDR");
	parameters.link("window",      this->windowConf.frame.tuple(windowWidth, windowHeight),  "beam-directional(m), azimuthal(deg)"); //, "[d]");
	parameters.link("gamma",       this->gammaAdjustment,  "Contrast adjustment, dark=0.0 < 1.0 < brighter ");

}


void ChaffOp::computeFuzzyDBZ(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	RadarFunctorOp<drain::FuzzyBell<double> > dbzFuzzifier;
	dbzFuzzifier.odimSrc = srcData.odim;
	// dbzFuzzifier.functor.set(dbzPeak, +25.0);
	if (dbzParam.empty()){ // start==end
		dbzFuzzifier.functor.set(dbzParam.min-10.0, dbzParam.max+10.0);
	}
	else {
		dbzFuzzifier.functor.set(dbzParam.min, dbzParam.max);
	}
	applyOperator(dbzFuzzifier, srcData, dstData, dstProduct);
};

void ChaffOp::computeFuzzyVRAD(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {

	drain::FuzzyStep<double> fuzzyStep;
	fuzzyStep.set(vradDevThreshold + 1.0, vradDevThreshold - 1.0); // Inverse; small deviation yields high response

	DopplerDevWindow::conf_t conf(fuzzyStep, windowConf.frame.width, windowConf.frame.height, 0.05, true, false); // require 5% samples
	conf.updatePixelSize(srcData.odim);

	SlidingWindowOp<DopplerDevWindow> vradDevOp(conf);
	applyOperator(vradDevOp, srcData, dstData, dstProduct);

};

void ChaffOp::computeFuzzyZDR(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	RadarFunctorOp<drain::FuzzyTriangle<double> > zdrFuzzifier;
	zdrFuzzifier.odimSrc = srcData.odim;
	zdrFuzzifier.functor.set(+zdrAbsThreshold, 0.0, -zdrAbsThreshold); // INVERSE //, -1.0, 1.0);
	applyOperator(zdrFuzzifier, srcData, dstData, dstProduct);
};



void ChaffOp::computeFuzzyRHOHV(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	RadarFunctorOp<drain::FuzzyStep<double> > rhohvFuzzifier;
	rhohvFuzzifier.odimSrc = srcData.odim;
	rhohvFuzzifier.functor.set(rhoHVthreshold, rhoHVthreshold * 0.8);
	applyOperator(rhohvFuzzifier, srcData, dstData, dstProduct);
};


} // Rack
