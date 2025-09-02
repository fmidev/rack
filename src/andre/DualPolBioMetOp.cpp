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
#include <drain/image/ImageChannel.h>
#include <drain/image/ImageLike.h>
#include <drain/image/Window.h>
#include <drain/imageops/CopyOp.h>
#include <drain/imageops/FunctorOp.h>
// RAISED
#include <drain/util/FunctorPack.h>
#include <drain/util/Fuzzy.h>
#include <drain/imageops/SlidingWindowOp.h>

#include "data/DataSelector.h"
#include "data/DataTools.h"
#include "data/PolarODIM.h"
#include "data/QuantityMap.h"

//#include <drain/util/RegExp.h>

#include "radar/Analysis.h"
#include "radar/Doppler.h"
#include "DualPolBioMetOp.h"


namespace rack {



/** Insect and BirdOp methods are shown pairwise to help comparing them.
 *
 */
void BirdOp::init(double dbzPeak, double vradDevMin, double rhoHVmax, double zdrAbsMin, double windowWidth, double windowHeight) {

	// parameters.link("dbzPeak",      this->dbzParam.tuple(dbzPeak-5.0, dbzPeak+5.0) ,  "Typical reflectivity (DBZH)") = true;
	parameters.link("dbzPeak",      this->dbzParam.tuple(dbzPeak-10.0, dbzPeak+10.0),  "Typical reflectivity (dBZ)").fillArray = true;
	parameters.link("vradDevMin",   this->vradDevThreshold = vradDevMin, "Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)");
	parameters.link("rhoHVmax",     this->rhoHVthreshold.tuple(rhoHVmax, 0.9*rhoHVmax),  "Fuzzy threshold of maximum rhoHV value");
	parameters.link("zdrAbsMin",    this->zdrAbsThreshold = zdrAbsMin,  "Fuzzy threshold of absolute ZDR");
	parameters.link("window",       this->windowConf.frame.tuple(windowWidth, windowHeight),  "beam-directional(m), azimuthal(deg)"); //, "[d]");
	parameters.link("gamma",  		this->gammaAdjustment,  "Contrast adjustment, dark=0.0 < 1.0 < brighter ");

}

void InsectOp::init(double dbzMax, double vradDevMax, double rhoHVmax, double zdrAbsMin, double windowWidth, double windowHeight) {

	//parameters.link("dbzPeak",      this->dbzPeak = dbzPeak,  "Max reflectivity, +/-5dBZ");
	parameters.link("dbzMax",       this->dbzParam.tuple(dbzMax-5.0, dbzMax+5.0),  "Max reflectivity").fillArray = true;
	parameters.link("vradDevMax",   this->vradDevThreshold = vradDevMax, "Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)");
	parameters.link("rhoHVmax",     this->rhoHVthreshold.tuple(rhoHVmax, 0.8*rhoHVmax),  "Fuzzy threshold of maximum rhoHV value");
	parameters.link("zdrAbsMin",    this->zdrAbsThreshold = zdrAbsMin,  "Fuzzy threshold of absolute ZDR");
	parameters.link("window",       this->windowConf.frame.tuple(windowWidth, windowHeight),  "beam-directional(m), azimuthal(deg)"); //, "[d]");
	parameters.link("gamma",  		this->gammaAdjustment,  "Contrast adjustment, dark=0.0 < 1.0 < brighter ");

}



}

// Rack
