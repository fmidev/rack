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

#include <drain/util/Cloner.h>

#include <drain/image/Image.h>
#include <drain/image/Window.h>
#include <drain/imageops/ImageOp.h>

#include "radar/Analysis.h"
#include "DetectorOp.h"

namespace rack {

/*
 class LocalFunctorBank {

	template <class T>
	static
	drain::Cloner<drain::UnaryFunctor,T> & getCloner(){
		static drain::Cloner<drain::UnaryFunctor,T> cloner; // = Static::get<Cloner<T,D>, bank_id>();
		return cloner;
	}

};
*/

///
/*
 *  \see drain::Cloner
 *  \see drain::FunctorBank
 */
class LocalFunctorBank {

public:

	template <class T>
	T & clone(){
		return getCloner<T>().getClonedSrc();
	}


protected:

	template <class T>
	drain::Cloner<drain::Fuzzifier<double>,T> & getCloner(){
		static drain::Cloner<drain::Fuzzifier<double>,T> cloner;
		// cloner.drop(1);
		return cloner;
	}

};

/// Base class for BirdOp and InsectOp.
/**
 *
 */
class FuzzyDetectorOp: public DetectorOp {

protected:

	inline
	FuzzyDetectorOp(const std::string & name, const std::string & description, const std::string & classCode) :
		DetectorOp(name, description, classCode)  {
		// dataSelector.setQuantities("DBZ:VRAD:RHOHV:ZDR");
		//dataSelector.setQuantities("DBZH:VRADH:RHOHV:ZDR");
		dataSelector.setQuantities("");
	};


	inline
	FuzzyDetectorOp(const FuzzyDetectorOp & op) : DetectorOp(op) {
		this->parameters.copyStruct(op.getParameters(), op, *this);
	};

	virtual inline
	~FuzzyDetectorOp(){};

	drain::Range<double> dbzParam = {-5.0,+5.0}; // Peak(span) or threshold(area), hence called generally "parameter".
	double vradDevThreshold = 2.0;
	//double rhoHVthreshold = 0.95;
	drain::Range<double> rhoHVthreshold = {0.85,0.95};
	double zdrAbsThreshold = 2.0;
	drain::image::WindowConfig windowConf;
	// RadarWindowConfig windowConf;
	double gammaAdjustment = 1.0; // neutral value, post-processing

	virtual inline
	const QuantitySelector & getSelectorDBZ() const {
		return selectorEmpty; // return selectorDBZ;
	};

	virtual inline
	const QuantitySelector & getSelectorVRAD() const {
		return selectorEmpty; // return selectorVRAD;
	};

	virtual inline
	const QuantitySelector & getSelectorZDR() const {
		return selectorEmpty; // return selectorZDR;
	};

	virtual inline
	const QuantitySelector & getSelectorRHOHV() const {
		return selectorEmpty;  //return selectorRHOHV;
	};

	virtual
	void runDetection(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & dstAux) const;


protected:

	static const QuantitySelector selectorEmpty;
	static const QuantitySelector selectorDBZ;
	static const QuantitySelector selectorVRAD;
	static const QuantitySelector selectorZDR;
	static const QuantitySelector selectorRHOHV;

	static drain::FuzzyIdentity<double> dummy;

public:

	//virtual
	//const RadarFunctorBase & getDBZFuzzifier(const ODIM & srcODIM) const = 0;
	//const drain::Fuzzifier<double> & getDBZFuzzifier() const = 0;

	/*
	virtual inline
	RadarFunctorBaseOp & getDBZFuzzifier() const {
		return dummyFunctorOp;
	}

	virtual inline
	RadarFunctorBaseOp & getVRADFuzzifier() const {
		return dummyFunctorOp;
	}
	*/

	virtual inline
	drain::Fuzzifier<double> & getFuzzifierDBZ(LocalFunctorBank & bank) const {
		return dummy;
	}

	virtual inline
	drain::Fuzzifier<double> & getFuzzifierVRAD(LocalFunctorBank & bank) const {
		return dummy;
	}

	virtual inline
	drain::Fuzzifier<double> & getFuzzifierZDR(LocalFunctorBank & bank) const {
		return dummy; // or 1.0
	}

	virtual inline
	drain::Fuzzifier<double> & getFuzzifierRHOHV(LocalFunctorBank & bank) const {
		return dummy;
	}


	/*
	virtual inline
	RadarFunctorBaseOp & getZDRFuzzifier() const {
		return dummyFunctorOp;
	}

	virtual inline
	RadarFunctorBaseOp & getRHOHVFuzzifier() const {
		return dummyFunctorOp;
	}
	*/


	// virtual
	// void computeFuzzy(RadarFunctorBaseOp & fuzzifier, const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;  // = 0;

	virtual
	void computeFuzzy(const drain::Fuzzifier<double> & fuzzifier, const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;  // = 0;

	virtual
	void computeFuzzyVRAD(const drain::Fuzzifier<double> & fuzzifier, const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;  // = 0;

	/*
	virtual
	void computeFuzzyDBZ(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;  // = 0;

	virtual
	void computeFuzzyVRAD(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const; // = 0;

	virtual
	void computeFuzzyZDR(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const;  // = 0;

	virtual
	void computeFuzzyRHOHV(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProduct) const; // = 0;
	*/

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

	mutable
	RadarFunctorOp<drain::IdentityFunctor> dummyFunctorOp;
};


}

#endif

// Rack
