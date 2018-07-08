/**

    Copyright 2014-   Markus Peura & Joonas Karjalainen  Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

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
