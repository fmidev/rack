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
	// Algorithm is based on the study made by Brandon Hickman from The University Of Helsinki
#ifndef RAINRATE_DP_OP_H_
#define RAINRATE_DP_OP_H_

//#include "VolumeOpNew.h"
#include "PolarProductOp.h"


using namespace drain::image;

namespace rack {

/// Uses DBZ, ZDR and KDP to derive rain rate. Under construction.
/**
 *RATE_KDP = sign(KDP) * kdp_aa * (|KDP| ** kdp_bb).

  where

    kdp_aa = 40.6
    kdp_bb = 0.866
 *
 */



class RainRateDPOp: public PolarProductOp {

public:

	//RainRateDPOp(double a = 22.7, double b = 0.802) :
	RainRateDPOp() :
		PolarProductOp("RainRateDP","Precip. rate [mm/h] from dual-pol using fuzzy thresholds. Alg. by Brandon Hickman"),
		dbzRange(30.0, 50.0), kdpRange(0.25, 0.35), zdrRange(0.15, 0.25)
		{

		dataSelector.setQuantities("^(DBZH|RHOHV|KDP|ZDR|QIND)$");
		dataSelector.setMaxCount(1);

		odim.product = "SURF";
		odim.quantity = "RATE";
		odim.type = "S";

		parameters.link("dbz", dbzRange.tuple(), "heavy:hail").fillArray = true;
		parameters.link("zdr", zdrRange.tuple(), "heavy").fillArray = true;
		parameters.link("rhohv", rhohv = 0.85, "met");
		parameters.link("kdp", kdpRange.tuple(), "heavy").fillArray = true;

		// quantityMap.setQuantityDefaults(odim, "RATE", "S");
		/*
		odim.quantity = "RATE";
		odim.type = "S";
		odim.setTypeDefaults();
		odim.scaling.scale = 0.0001;
		odim.scaling.offset = 0.0;
		 */

		// Later these...
		// allowedEncoding.delink("gain");
		/*
		allowedEncoding.clear();
		allowedEncoding.link("type", odim.type);
		allowedEncoding.link("gain", odim.scaling.scale);
		allowedEncoding.link("offset", odim.scaling.offset);
		//allowedEncoding.link("freeze", odim.freeze);
		allowedEncoding.link("type", odim.type, "S");
		allowedEncoding.link("gain", odim.scaling.scale);
		allowedEncoding.link("offset", odim.scaling.offset);
		*/

		/*
		allowedEncoding.link("rscale", odim.rscale);
		allowedEncoding.link("nrays", odim.geometry.height);
		allowedEncoding.link("nbins", odim.geometry.width);
		*/

	};




	virtual
	inline
	~RainRateDPOp(){};

	virtual
	void processDataSetOLD(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const;

	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const;


	drain::Range<double> dbzRange;
	drain::Range<double> kdpRange;
	drain::Range<double> zdrRange;
	double rhohv;

protected:

	void computeFuzzyMembership(
			const PlainData<PolarSrc> & srcData, const drain::Fuzzifier<double> & fuzzyFctor,
			PlainData<PolarDst> & dstData) const;

	void computeProduct(
			const PlainData<PolarSrc> & srcData, const drain::Fuzzifier<double> & fuzzyFctor,
			const SingleParamPrecip & rateFnc, DataSet<PolarDst> & dstProduct) const;

	void computeProduct2(
			const PlainData<PolarSrc> & srcData,  const drain::Fuzzifier<double> & fuzzyFctor,
			const PlainData<PolarSrc> & srcData2, const drain::Fuzzifier<double> & fuzzyFctor2,
			const DoubleParamPrecip & rateFnc, DataSet<PolarDst> & dstProduct) const;


};


}

#endif

// Rack
