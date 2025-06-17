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
#ifndef RACK_GLIDER_OP
#define RACK_GLIDER_OP

#include <string>

#include <drain/image/Image.h>
#include <drain/image/Window.h>
#include <drain/imageops/ImageOp.h>


#include "FuzzyDetectorOp.h"

// RAISED
/*
#include <drain/imageops/SlidingWindowOp.h>
#include "radar/Analysis.h"
#include "radar/Doppler.h"

//#include "data/Data.h"
#include "andre/DetectorOp.h"
// #include <drain/util/FunctorPack.h>
// #include <drain/util/Fuzzy.h>
// RAISED
*/



namespace rack {

using namespace drain::image;


///
/**
 *
 */
class BirdOp: public FuzzyDetectorOp {

public:

	/**
	 *
	 *  \param dbzPeak
	 *  \param vradDevMin
	 *  \param rhoHVmax
	 *  \param zdrAbsMin
	 *  \param windowWidth  // Optional postprocessing: morphological closing.
	 *  \param windowHeight
	 *
	 */
//	BirdOp(double dbzPeak = -5.0, double vradDevMin = 5.0, double rhoHVmax = 0.7, double zdrAbsMin = 2.0, double windowWidth = 2500, double windowHeight = 5.0) :
	BirdOp(double dbzPeak = 0.0, double vradDevMin = 1.0, double rhoHVmax = 0.95, double zdrAbsMin = 1.0, double windowWidth = 2500, double windowHeight = 5.0) :

		FuzzyDetectorOp(__FUNCTION__, "Estimates bird probability from DBZH, VRAD, RhoHV and ZDR.", "nonmet.biol.bird"){

		init(dbzPeak, vradDevMin, rhoHVmax, zdrAbsMin, windowWidth, windowHeight);

	};

	inline
	BirdOp(const BirdOp & op) : FuzzyDetectorOp(op) {
		this->parameters.copyStruct(op.getParameters(), op, *this);
	};

	virtual inline
	~BirdOp(){};

	virtual
	void init(double dbzPeak, double vradDevMax, double rhoHVmax, double zdrDevMin, double windowWidth, double windowHeight);

	virtual
	void computeFuzzyDBZ(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;

	virtual
	void computeFuzzyVRAD(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;

	virtual
	void computeFuzzyZDR(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;

	virtual
	void computeFuzzyRHOHV(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;


};


class InsectOp: public FuzzyDetectorOp {

public:

	// BIRD: double dbzPeak = -5.0, double vradDevMin = 5.0, double rhoHVmax = 0.7, double zdrAbsMin = 2.0, double windowWidth = 2500, double windowHeight = 5.0

	/**
	 *
	 *  \param dbzPeak
	 *  \param vradDevMin
	 *  \param rhoHVmax
	 *  \param zdrAbsMin
	 *  \param windowWidth
	 *  \param windowHeight
	 *
	 */
	// InsectOp(double dbzPeak = -10.0, double vradDevMax = +5.0, double rhoHVmax = 0.7, double zdrAbsMin = 3.0, double windowWidth = 2500, double windowHeight = 5.0) :
	InsectOp(double dbzMax = 0.0, double vradDevMax = +5.0, double rhoHVmax = 0.95, double zdrAbsMin = 3.0, double windowWidth = 2500, double windowHeight = 5.0) :
		FuzzyDetectorOp(__FUNCTION__, "Probability of insects, based on DBZH, VRAD, RhoHV and ZDR.", "nonmet.biol.insect"){
		init(dbzMax, vradDevMax, rhoHVmax, zdrAbsMin, windowWidth, windowHeight);
	};

	InsectOp(const InsectOp & op) : FuzzyDetectorOp(op) {
		this->parameters.copyStruct(op.getParameters(), op, *this);
	}

	virtual
	void init(double dbzMax, double vradDevMax, double rhoHVmax, double zdrDevMin, double windowWidth, double windowHeight);

	virtual
	void computeFuzzyDBZ(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;

	virtual
	void computeFuzzyVRAD(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;

	virtual
	void computeFuzzyZDR(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;

	virtual
	void computeFuzzyRHOHV(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;


};


}

#endif

// Rack
