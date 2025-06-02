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
#ifndef BIRD_H_
#define BIRD_H_

#include <andre/DetectorOp.h>
#include "data/Data.h"
#include "drain/image/Image.h"
#include "drain/image/Window.h"
#include "drain/imageops/ImageOp.h"
#include <string>

using namespace drain::image;

namespace rack {


///
/**

 *
 */
class GliderOp: public DetectorOp {

protected:

	inline
	GliderOp(const std::string & name, const std::string & description, const std::string & classCode) :
		DetectorOp(name, description, classCode), dbzPeak(+5),  VRAD_FLIP(false), zdrAbsMin(+2.0)  {
		//dataSelector.setQuantityRegExp("^(DBZH|VRAD|VRADH|RHOHV|ZDR)$");
		dataSelector.setQuantities("DBZH:VRAD:VRADH:RHOHV:ZDR");
		// dataSelector.setMaxCount(1);
	};


	inline
	GliderOp(const GliderOp & op) : DetectorOp(op), dbzPeak(0.0), VRAD_FLIP(false), zdrAbsMin(+2.0) {
		this->parameters.copyStruct(op.getParameters(), op, *this);
	};

	virtual inline
	~GliderOp(){};


	double dbzPeak;

	bool VRAD_FLIP;
	drain::Range<double> vradDevRange;

	//double wradMin;

	drain::Range<double> rhoHVRange;

	double zdrAbsMin;

	drain::image::WindowConfig window;
	//double windowWidth;
	//double windowHeight;


	virtual
	//void processDataSet(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & dstAux) const;
	void runDetection(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & dstAux) const;


protected:


	/// Inits common for BIRD and INSECT
	// kludge
	//void init(double dbzPeak = -5.0, double vradDevMin = 5.0, double rhoHVmax = 0.7, double zdrDevMin = 2.0, double windowWidth = 2500, double windowHeight = 5.0);

	/// Convenience function for "accumulating" detection results.
	/**
	 *   \param tmp - image for latest result, in a sequence of operations
	 *   \param dstData - actual result
	 *   \param dstProductAux -
	 */
	void applyOperator(const ImageOp & op, Image & tmp, const std::string & feature, const Data<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const;


};

///
/**

 *
 */
class BirdOp: public GliderOp {

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

		GliderOp(__FUNCTION__, "Estimates bird probability from DBZH, VRAD, RhoHV and ZDR.", "nonmet.biol.bird"){ // Optional postprocessing: morphological closing.

		init(dbzPeak, vradDevMin, rhoHVmax, zdrAbsMin, windowWidth, windowHeight);

	};

	inline
	BirdOp(const BirdOp & op) : GliderOp(op) {
		this->parameters.copyStruct(op.getParameters(), op, *this);
	};

	// virtual 	inline	~BirdOp(){};

protected:

	///
	void init(double dbzPeak = -5.0, double vradDevMin = 5.0, double rhoHVmax = 0.7, double zdrDevMin = 2.0, double windowWidth = 2500, double windowHeight = 5.0);

};


class InsectOp: public GliderOp {

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
		GliderOp(__FUNCTION__, "Estimates probability from DBZH, VRAD, RhoHV and ZDR.", "nonmet.biol.insect"){
		init(dbzPeak, vradDevMax, rhoHVmax, zdrAbsMin, windowWidth, windowHeight);
		// this->vradDev.max = 0.9 *vradDevMax;
		// this->vradDev.min = 1.1 *vradDevMax;
	};

	InsectOp(const InsectOp & op) : GliderOp(op) {
		this->parameters.copyStruct(op.getParameters(), op, *this);
	}

protected:

	void init(double dbzPeak = -5.0, double vradDevMax = +5.0, double rhoHVmax = 0.7, double zdrDevMin = 2.0, double windowWidth = 2500, double windowHeight = 5.0);

};


}

#endif

// Rack
