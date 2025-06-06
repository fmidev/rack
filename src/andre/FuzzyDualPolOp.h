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
#ifndef RACK_FUZZY_DUALPOL_OP
#define RACK_FUZZY_DUALPOL_OP

#include <string>

#include "drain/image/Image.h"
#include "drain/image/Window.h"
#include "drain/imageops/ImageOp.h"

// RAISED
#include "drain/imageops/SlidingWindowOp.h"

//#include "data/Data.h"
#include "andre/DetectorOp.h"
// #include "drain/util/FunctorPack.h"
// #include "drain/util/Fuzzy.h"

// RAISED
#include "radar/Analysis.h"
#include "radar/Doppler.h"

namespace rack {

using namespace drain::image;

///
/**

 *
 */
class FuzzyDualPolOp: public DetectorOp {

protected:

	inline
	FuzzyDualPolOp(const std::string & name, const std::string & description, const std::string & classCode, bool vrad_flip) :
		DetectorOp(name, description, classCode), dbzPeak(+5), VRAD_FLIP(vrad_flip), zdrAbsMin(+2.0)  {
		dataSelector.setQuantities("DBZH:VRAD:VRADH:RHOHV:ZDR");
	};


	inline
	FuzzyDualPolOp(const FuzzyDualPolOp & op) : DetectorOp(op), dbzPeak(0.0), VRAD_FLIP(op.VRAD_FLIP), zdrAbsMin(+2.0) {
		this->parameters.copyStruct(op.getParameters(), op, *this);
	};

	virtual inline
	~FuzzyDualPolOp(){};


	double dbzPeak;
	const bool VRAD_FLIP;
	drain::Range<double> vradDevRange;
	drain::Range<double> rhoHVRange;
	double zdrAbsMin;

	drain::image::WindowConfig window;


	virtual
	//void processDataSet(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & dstAux) const;
	void runDetection(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & dstAux) const;


protected:

	void init(double dbzPeak, double vradDevMax, double rhoHVmax, double zdrDevMin, double windowWidth, double windowHeight);

	virtual
	void computeFuzzyDBZ(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;

	virtual
	void computeFuzzyVRAD(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;

	virtual
	void computeFuzzyZDR(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;

	virtual
	void computeFuzzyRHOHV(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;

	/// Convenience function for "accumulating" detection results.
	/**
	 *   \param tmp - image for latest result, in a sequence of operations
	 *   \param dstData - actual result
	 *   \param dstProductAux -
	 *   Image & tmp,
	 */
	void applyOperator(const ImageOp & op, const std::string & feature, const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const;


};

///
/**

 *
 */
class BirdOp: public FuzzyDualPolOp {

public:

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
//	BirdOp(double dbzPeak = -5.0, double vradDevMin = 5.0, double rhoHVmax = 0.7, double zdrAbsMin = 2.0, double windowWidth = 2500, double windowHeight = 5.0) :
	BirdOp(double dbzPeak = 0.0, double vradDevMin = 3.0, double rhoHVmax = 0.8, double zdrAbsMin = 1.0, double windowWidth = 2500, double windowHeight = 5.0) :

		FuzzyDualPolOp(__FUNCTION__, "Estimates bird probability from DBZH, VRAD, RhoHV and ZDR.", "nonmet.biol.bird", false){ // Optional postprocessing: morphological closing.

		init(dbzPeak, vradDevMin, rhoHVmax, zdrAbsMin, windowWidth, windowHeight);

	};

	inline
	BirdOp(const BirdOp & op) : FuzzyDualPolOp(op) {
		this->parameters.copyStruct(op.getParameters(), op, *this);
	};

	virtual inline
	~BirdOp(){};


	/*
	virtual
	const ImageOp & getFuzzifierDBZ(const PolarODIM & odim) const {
		dbzFuzzifier.odimSrc = odim;
		dbzFuzzifier.functor.set(dbzPeak, +5.0);
		return dbzFuzzifier;
	}

	virtual
	const ImageOp & getFuzzifierVRAD(const PolarODIM & odim) const;

	virtual inline
	const ImageOp & getFuzzifierZDR(const PolarODIM & odim) const {
		dbzFuzzifier.odimSrc = odim;
		dbzFuzzifier.functor.set(dbzPeak, +5.0);
		return dbzFuzzifier;
	};

	virtual
	const ImageOp & getFuzzifierRHOHV(const PolarODIM & odim) const {
		dbzFuzzifier.odimSrc = odim;
		dbzFuzzifier.functor.set(dbzPeak, +5.0);
		return dbzFuzzifier;
	};


protected:

	mutable
	RadarFunctorOp<drain::FuzzyBell<double> > dbzFuzzifier;

	mutable
	drain::image::SlidingWindowOp<DopplerDevWindow> vradFuzzifier;
	//RadarFunctorOp<drain::FuzzyBell<double> > dbzFuzzifier;
	*/
};


class InsectOp: public FuzzyDualPolOp {

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
	InsectOp(double dbzPeak = -10.0, double vradDevMax = +5.0, double rhoHVmax = 0.7, double zdrAbsMin = 3.0, double windowWidth = 2500, double windowHeight = 5.0) :
		FuzzyDualPolOp(__FUNCTION__, "Estimates probability from DBZH, VRAD, RhoHV and ZDR.", "nonmet.biol.insect", true){
		init(dbzPeak, vradDevMax, rhoHVmax, zdrAbsMin, windowWidth, windowHeight);
	};

	InsectOp(const InsectOp & op) : FuzzyDualPolOp(op) {
		this->parameters.copyStruct(op.getParameters(), op, *this);
	}

	/*
	virtual
	const ImageOp & getFuzzifierDBZ(const PolarODIM & odim) const {
		dbzFuzzifier.odimSrc = odim;
		dbzFuzzifier.functor.set(dbzPeak, +5.0);
		return dbzFuzzifier;
	}

	virtual inline
	const ImageOp & getFuzzifierVRAD(const PolarODIM & odim) const {
		dbzFuzzifier.odimSrc = odim;
		dbzFuzzifier.functor.set(dbzPeak, +5.0);
		return dbzFuzzifier;
	};

	virtual inline
	const ImageOp & getFuzzifierZDR(const PolarODIM & odim) const {
		dbzFuzzifier.odimSrc = odim;
		dbzFuzzifier.functor.set(dbzPeak, +5.0);
		return dbzFuzzifier;
	};

	virtual
	const ImageOp & getFuzzifierRHOHV(const PolarODIM & odim) const {
		dbzFuzzifier.odimSrc = odim;
		dbzFuzzifier.functor.set(dbzPeak, +5.0);
		return dbzFuzzifier;
	};


protected:

	mutable
	RadarFunctorOp<drain::FuzzyBell<double> > dbzFuzzifier;
	//drain::FuzzyStep<double> fuzzyStepDBZ;

	mutable
	drain::image::SlidingWindowOp<DopplerDevWindow> vradFuzzifier;
	*/

};


}

#endif

// Rack
