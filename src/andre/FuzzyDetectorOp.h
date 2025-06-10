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

#include <drain/image/Image.h>
#include <drain/image/Window.h>
#include <drain/imageops/ImageOp.h>


#include "andre/DetectorOp.h"

namespace rack {


/// Base class for BirdOp and InsectOp.
/**
 *
 */
class FuzzyDetectorOp: public DetectorOp {

protected:

	inline
	FuzzyDetectorOp(const std::string & name, const std::string & description, const std::string & classCode, bool vrad_flip) :
		DetectorOp(name, description, classCode)  {
		dataSelector.setQuantities("DBZH:VRAD:VRADH:RHOHV:ZDR");
	};


	inline
	FuzzyDetectorOp(const FuzzyDetectorOp & op) : DetectorOp(op) {
		this->parameters.copyStruct(op.getParameters(), op, *this);
	};

	virtual inline
	~FuzzyDetectorOp(){};


	double dbzParam = 0.0;  // Peak or threshold location (hence "param").
	double vradDevThreshold = 2.0;
	double rhoHVthreshold = 0.95;
	double zdrAbsThreshold = 2.0;
	drain::image::WindowConfig windowConf;
	// RadarWindowConfig windowConf;
	double gammaAdjustment = 2.0; // neutral value, post-processing


	virtual
	void runDetection(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & dstAux) const;


protected:

	// void init(double dbzPeak, double vradDevMax, double rhoHVmax, double zdrDevMin, double windowWidth, double windowHeight);
	//void init(double dbzPeak, double vradDevMax, double rhoHVmax, double zdrDevMin, double windowWidth, double windowHeight);


	virtual
	void computeFuzzyDBZ(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const = 0;

	virtual
	void computeFuzzyVRAD(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const = 0;

	virtual
	void computeFuzzyZDR(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const = 0;

	virtual
	void computeFuzzyRHOHV(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const = 0;

	/// Convenience function for "accumulating" detection results.
	/**
	 *   \param tmp - image for latest result, in a sequence of operations
	 *   \param dstData - actual result
	 *   \param dstProductAux -
	 *   Image & tmp,
	 *   const std::string & feature,
	 */
	void applyOperator(const ImageOp & op, const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const;

	/// Returns a 256-element map of Gamma corrected values, scaled by 256.
	static
	void getGammaLookUpTable(double p, std::vector<unsigned char> & lookUpTable);

};


}

#endif

// Rack
