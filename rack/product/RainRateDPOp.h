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
#ifndef RAINRATE2_H_
#define RAINRATE2_H_

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
	RainRateDPOp(double freezingLevel=10.0, double freezingLevelThickness=0.2, const std::string & dataThresholdParams="0.85:50.0:20.0:0.2:0.1", const std::string & dbzParams = "200.0:1.6:2000.0:2.0", const std::string & zzdrParams = "0.0122:0.820:-2.28", const std::string & kdpParams = "21.0:0.70", const std::string & kdpzdrParams = "29.7:0.89:-0.927") :
		PolarProductOp("RainRateDP","Estimates on precipitation rate [mm/h] from dual-pol parameters."){ // Optional postprocessing: morphological closing.
		//const std::string & dbzParams = "22.7:0.802:2000.0:2.0"
		dataSelector.path = "data[0-9]+/?$";
		//dataSelector.quantity = "^DBZH$";
		dataSelector.quantity = "^(RHOHV|KDP|DBZH|ZDR|QIND)$";
		dataSelector.count = 1;

		parameters.reference("freezingLevel", this->freezingLevel = freezingLevel, "km");
		parameters.reference("freezingLevelThickness", this->freezingLevelThickness = freezingLevelThickness, "km");
		parameters.reference("dataThresholdParams", this->dataThreshodParams = dataThresholdParams, "Data limits used in decision tree when calculating rain rate. RHOHV meteorological target:DBZ hail/ice:DBZ heavy rain: KDP heavy rain: ZDR");
		parameters.reference("dbzParams", this->dbzParams = dbzParams, "Coefficients in ...Marshall-Palmer");  //Marshall-Palmer
		parameters.reference("kdpParams", this->kdpParams = kdpParams, "a, b in R = a * KDP^b");
		parameters.reference("zzdrParams", this->zzdrParams = zzdrParams, "a, b and c in R = a * Z^b * ZDR^c");
		parameters.reference("kdpzdrParams", this->kdpzdrParams = kdpzdrParams, "a, b and c in R = a * KDP^0.89 * ZDR^c");

		odim.product = "SURF";
		// quantityMap.setQuantityDefaults(odim, "RATE", "S");
		/*
		odim.quantity = "RATE";
		odim.type = "S";
		odim.setTypeDefaults();
		odim.gain = 0.0001;
		odim.offset = 0.0;
		 */

		// Later these...
		// allowedEncoding.dereference("gain");
		allowedEncoding.clear();
		allowedEncoding.reference("type", odim.type);
		allowedEncoding.reference("gain", odim.gain);
		allowedEncoding.reference("offset", odim.offset);
		//allowedEncoding.reference("freeze", odim.freeze);
		/*
		allowedEncoding.reference("type", odim.type, "S");
		allowedEncoding.reference("gain", odim.gain);
		allowedEncoding.reference("offset", odim.offset);
		*/

		/*
		allowedEncoding.reference("rscale", odim.rscale);
		allowedEncoding.reference("nrays", odim.nrays);
		allowedEncoding.reference("nbins", odim.nbins);
		*/

	};




	virtual
	inline
	~RainRateDPOp(){};

	virtual
	//void processSweep(const SweepSrc & src, ProductDst & dst) const;
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const;

	double freezingLevel;
	double freezingLevelThickness;
	std::string dbzParams;
	std::string zzdrParams;
	std::string kdpParams;
	std::string kdpzdrParams;
	std::string dataThreshodParams;


protected:



};


}

#endif

// Rack
