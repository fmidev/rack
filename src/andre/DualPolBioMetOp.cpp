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

#include "DualPolBioMetOp.h"


namespace rack {

using namespace drain;
using namespace drain::image;


/** Insect and BirdOp methods are shown pairwise to help comparing them.
 *
 */

void InsectOp::init(double dbzPeak, double vradDevMin, double rhoHVmax, double zdrAbsMin, double windowWidth, double windowHeight) {

	parameters.link("dbzPeak",      this->dbzPeak = dbzPeak,  "Typical reflectivity (DBZH)");
	parameters.link("vradDevMax",   this->vradDevRange.tuple(0.0, vradDevMin), "Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)");
	parameters.link("rhoHVmax",     this->rhoHVRange.tuple(0.9*rhoHVmax, rhoHVmax),  "Fuzzy threshold of maximum rhoHV value");
	parameters.link("zdrAbsMin",    this->zdrAbsMin = zdrAbsMin,  "Fuzzy threshold of absolute ZDR");
	parameters.link("windowWidth",  this->window.frame.width = windowWidth,  "window width, beam-directional (m)"); //, "[m]");
	parameters.link("windowHeight", this->window.frame.height = windowHeight,  "window width, azimuthal (deg)"); //, "[d]");
	// parameters.link("wradMin", this->wradMin, wradMin, "Minimum normalized deviation of within-bin Doppler speed deviation (WRAD)");

}

void BirdOp::init(double dbzPeak, double vradDevMin, double rhoHVmax, double zdrAbsMin, double windowWidth, double windowHeight) {

	parameters.link("dbzPeak",      this->dbzPeak = dbzPeak,  "Typical reflectivity (DBZH)");
	parameters.link("vradDevMin",   this->vradDevRange.tuple(0.0, vradDevMin), "Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)");
	parameters.link("rhoHVmax",     this->rhoHVRange.tuple(0.9*rhoHVmax, rhoHVmax),  "Fuzzy threshold of maximum rhoHV value");
	parameters.link("zdrAbsMin",    this->zdrAbsMin = zdrAbsMin,  "Fuzzy threshold of absolute ZDR");
	parameters.link("windowWidth",  this->window.frame.width = windowWidth,  "window width, beam-directional (m)"); //, "[m]");
	parameters.link("windowHeight", this->window.frame.height = windowHeight,  "window width, azimuthal (deg)"); //, "[d]");
	// parameters.link("wradMin", this->wradMin, wradMin, "Minimum normalized deviation of within-bin Doppler speed deviation (WRAD)");

}







void InsectOp::computeFuzzyDBZ(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	RadarFunctorOp<FuzzyBell<double> > dbzFuzzifier;
	dbzFuzzifier.odimSrc = srcData.odim;
	dbzFuzzifier.functor.set(dbzPeak, +5.0);
	applyOperator(dbzFuzzifier, "FUZZY_DBZH_LOW", srcData, dstData, dstProduct);
};

void BirdOp::computeFuzzyDBZ(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	RadarFunctorOp<FuzzyBell<double> > dbzFuzzifier;
	dbzFuzzifier.odimSrc = srcData.odim;
	dbzFuzzifier.functor.set(dbzPeak, +5.0);
	applyOperator(dbzFuzzifier, "FUZZY_DBZH_LOW", srcData, dstData, dstProduct);
};


void InsectOp::computeFuzzyVRAD(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {

	FuzzyStep<double> fuzzyStep; //(0.5);
	//const double pos = vradDevMin; ///vradSrc.odim.NI; // TODO: consider relative value directly as parameter NO! => alarm if over +/- 0.5

	/*if (!VRAD_FLIP)
		fuzzyStep.set(vradDevRange.min, vradDevRange.max);
	else
		fuzzyStep.set(vradDevRange.max, vradDevRange.min);
		*/
	fuzzyStep.set(vradDevRange.max, vradDevRange.min);

	DopplerDevWindow::conf_t conf(fuzzyStep, window.frame.width, window.frame.height, 0.05, true, false); // require 5% samples
	conf.updatePixelSize(srcData.odim);
	SlidingWindowOp<DopplerDevWindow> vradDevOp(conf);

	applyOperator(vradDevOp, "FUZZY_VRADH_DEV", srcData, dstData, dstProduct);
};

void BirdOp::computeFuzzyVRAD(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {

	FuzzyStep<double> fuzzyStep; //(0.5);

	/*
	if (!VRAD_FLIP)
		fuzzyStep.set(vradDevRange.min, vradDevRange.max);
	else
		fuzzyStep.set(vradDevRange.max, vradDevRange.min);
		*/
	fuzzyStep.set(vradDevRange.min, vradDevRange.max);

	DopplerDevWindow::conf_t conf(fuzzyStep, window.frame.width, window.frame.height, 0.05, true, false); // require 5% samples
	conf.updatePixelSize(srcData.odim);
	SlidingWindowOp<DopplerDevWindow> vradDevOp(conf);

	applyOperator(vradDevOp, "FUZZY_VRADH_DEV", srcData, dstData, dstProduct);

};

void InsectOp::computeFuzzyZDR(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	RadarFunctorOp<FuzzyTriangle<double> > zdrFuzzifier;
	zdrFuzzifier.odimSrc = srcData.odim;
	zdrFuzzifier.functor.set(+zdrAbsMin, 0.0, -zdrAbsMin); // INVERSE //, -1.0, 1.0);
	applyOperator(zdrFuzzifier,  "FUZZY_ZDR_NONZERO", srcData, dstData, dstProduct);
};

void BirdOp::computeFuzzyZDR(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	RadarFunctorOp<FuzzyTriangle<double> > zdrFuzzifier;
	zdrFuzzifier.odimSrc = srcData.odim;
	zdrFuzzifier.functor.set(+zdrAbsMin, 0.0, -zdrAbsMin); // INVERSE //, -1.0, 1.0);
	applyOperator(zdrFuzzifier,  "FUZZY_ZDR_NONZERO", srcData, dstData, dstProduct);
};



void InsectOp::computeFuzzyRHOHV(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	RadarFunctorOp<FuzzyStep<double> > rhohvFuzzifier;
	rhohvFuzzifier.odimSrc = srcData.odim;
	//rhohvFuzzifier.functor.set(rhoHVmax+(1.0-rhoHVmax)/2.0, rhoHVmax);
	rhohvFuzzifier.functor.set(rhoHVRange.max, rhoHVRange.min);
	// mout.debug("RHOHV_LOW" , rhohvFuzzifier.functor );
	applyOperator(rhohvFuzzifier, "FUZZY_RHOHV_LOW", srcData, dstData, dstProduct);
};

void BirdOp::computeFuzzyRHOHV(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const {
	RadarFunctorOp<FuzzyStep<double> > rhohvFuzzifier;
	rhohvFuzzifier.odimSrc = srcData.odim;
	//rhohvFuzzifier.functor.set(rhoHVmax+(1.0-rhoHVmax)/2.0, rhoHVmax);
	rhohvFuzzifier.functor.set(rhoHVRange.max, rhoHVRange.min);
	// mout.debug("RHOHV_LOW" , rhohvFuzzifier.functor );
	applyOperator(rhohvFuzzifier, "FUZZY_RHOHV_LOW", srcData, dstData, dstProduct);
};




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
