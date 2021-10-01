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

#include "drain/util/Fuzzy.h"
#include "drain/util/FunctorPack.h"
//#include "drain/image/SlidingWindowMedianOp.h"
#include "drain/image/File.h"

//#include "hi5/Hi5Write.h"

#include "data/ODIM.h"
#include "data/Data.h"
#include "data/QuantityMap.h"

#include "radar/Geometry.h"
#include "radar/Analysis.h"
#include "main/rack.h"
#include "radar/Precipitation.h"

#include "RainRateDPOp.h"
#include "RainRateOp.h"


//#include "drain/image/SegmentAreaOp.h"
//#include "drain/image/MathOpPack.h"



//using namespace drain::image;

namespace rack {



void RainRateDPOp::processDataSet(const DataSet<PolarSrc> & sweepSrc, DataSet<PolarDst> & dstProduct) const {

	/// For monitoring cmd execution as text dump. Use --debug (or \c --verbose \c \<level\> to define verbosity.
	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.info() << "start" <<  mout.endl;

	// IDEA: FUZZY => PROB quality2/... so that finally class could be extracted?

	/// Retrieve dBZ data
	const PlainData<PolarSrc> & srcDBZH = sweepSrc.getData("DBZH");

	const bool DBZH  = !srcDBZH.data.isEmpty();

	if (!DBZH){
		mout.warn() << "critical DBZH data missing, giving up..." <<  mout.endl;
		return;
	}

	//const drain::image::AreaGeometry & geometry = srcDBZH.data.getGeometry();
	const QuantityMap & qmap = getQuantityMap();

	// Reserve main quantity (RATE) first!
	PlainData<PolarDst> & rateDst = dstProduct.getData(odim.quantity);
	//this->initDst(srcDBZH.odim, rateDst);
	//this->initDst(odim, rateDst);
	//rateDst.setEncoding(odim.type, encodingRequest);
	//rateDst.setEncoding(odim.type);
	qmap.setQuantityDefaults(rateDst, odim.quantity, odim.type); // type also?
	rateDst.setEncoding(odim.type, encodingRequest);
	rateDst.setGeometry(srcDBZH.data.getGeometry());
	rateDst.odim.rscale = srcDBZH.odim.rscale;

	//completeEncoding(rateDst.odim, encodingRequest);
	//rateDst.data.setScaling(rateDst.odim.scaling);
	//applyODIM(rateDst.odim, odim, true);
	//rateDst.data.setScaling(rateDst.odim.scaling.scale, rateDst.odim.scaling.offset); // clumsy

	//qmap.setQuantityDefaults(rateDst, odim.quantity, odim.type); // type also?
	mout.warn() << "dst data: " << rateDst <<  mout.endl;

	// RadarFunctorOp<drain::FuzzyStep<double> > step;
	// RadarFunctorOp<drain::FuzzyStepsoid<double> > stepsoid;

	// RHOHV
	const PlainData<PolarSrc> & srcRHOHV = sweepSrc.getData("RHOHV");
	const bool RHOHV = !srcRHOHV.data.isEmpty();


	// ZDR
	const PlainData<PolarSrc> & srcZDR = sweepSrc.getData("ZDR");
	const bool ZDR   = !srcZDR.data.isEmpty();
	if (!ZDR){
		mout.warn() << "ZDR missing" <<  mout.endl;
	}

	// KDP
	const PlainData<PolarSrc> & srcKDP = sweepSrc.getData("KDP");
	const bool KDP = !srcKDP.data.isEmpty();
	if (!KDP){
		mout.warn() << "KDP missing" <<  mout.endl;
	}

	// KDP x DBZH... needed?
	/*
	PlainData<PolarDst> & heavyKDPxDBZH = dstProduct.getData("KDPxDBZH_FUZZY");
	qmap.setQuantityDefaults(heavyKDPxDBZH, "PROB");
	heavyKDPxDBZH.setGeometry(geometry);
	drain::image::BinaryFunctorOp<MultiplicationFunctor> mul;
	mout.warn() << heavyDBZH.data     <<  mout.endl;
	mout.warn() << heavyKDP.data.getChannel(0)      <<  mout.endl;
	mout.warn() << heavyKDPxDBZH.data <<  mout.endl;
	mul.traverseChannel(heavyDBZH.data, heavyKDP.data.getChannel(0), heavyKDPxDBZH.data);
	*/


	//double rateZ      = 0.0;
	//double rateZZDR   = 0.0;
	double rateKDP    = 0.0;
	//double rateKDPZDR = 0.0;

	double rate;
	double rateRain = 0.0;
	double rateHeavy = 0.0;

	const double d = (dbzRange.max-dbzRange.min) / 2.0;
	drain::FuzzyStep<double> fuzzyDBZ(dbzRange.min-d, dbzRange.min+d);
	double weightHeavy;

	drain::FuzzyStep<double> fuzzyHail(dbzRange.max-d, dbzRange.max+d);
	double weightHail;

	drain::FuzzyStep<double> fuzzyZDR(zdrRange.min, zdrRange.max);
	double weightZDR;

	drain::FuzzyStep<double> fuzzyKDP(kdpRange.min, kdpRange.max);
	double weightKDP;

	/// Bin-wise checks
	bool dbzh=false;
	bool zdr=false;
	bool kdp=false;

	double valueDBZH = -32;
	double valueKDP  = 0.0;
	double valueZDR  = 0.0;

	mout.note() << "main loop" <<  mout.endl;

	typedef drain::image::Image::const_iterator iter_t;
	iter_t itDBZH      = srcDBZH.data.begin();
	iter_t itZDR       = srcZDR.data.begin();
	iter_t itKDP       = srcKDP.data.begin();
	for (drain::image::Image::iterator it = rateDst.data.begin(); it!=rateDst.data.end(); ++it){

		rate = rateDst.odim.nodata;

		// isValue==true, if value is not 'undetect' or 'nodata'
		if (DBZH)
			dbzh = srcDBZH.odim.isValue(*itDBZH);

		if (ZDR)
			zdr = srcZDR.odim.isValue(*itZDR);

		if (KDP)
			kdp = srcKDP.odim.isValue(*itKDP);


		if (kdp){
			valueKDP  = srcKDP.odim.scaleForward(*itKDP);
			weightKDP = fuzzyKDP(valueKDP);
			rateKDP = RainRateOp::precipKDP.rainRate(valueKDP);
		}

		if (zdr){
			valueZDR  = srcKDP.odim.scaleForward(*itZDR);
			weightZDR = fuzzyZDR(valueZDR);
		}


		if (dbzh){ // needed?

			valueDBZH = srcDBZH.odim.scaleForward(*itDBZH);

			if (kdp){

				/// Z & ZDR
				if (zdr)
					rateRain = (1.0 - weightZDR)*RainRateOp::precipZrain.rainRate(valueDBZH) + weightZDR*RainRateOp::precipZZDR.rainRate(valueDBZH, valueZDR);
				else
					rateRain = RainRateOp::precipZrain.rainRate(valueDBZH);

				/// KDP & ZDR
				if (zdr)
					rateHeavy = (1.0 - weightZDR)*RainRateOp::precipKDP.rainRate(valueKDP) + weightZDR*RainRateOp::precipKDPZDR.rainRate(valueKDP, valueZDR);
				else
					rateHeavy = rateKDP;

				weightHeavy = weightKDP*fuzzyDBZ(valueDBZH);

				rate = (1.0 - weightHeavy)*rateRain + weightHeavy*rateHeavy;

				// Main branch
				weightHail = fuzzyHail(valueDBZH);

				rate = (1.0 - weightHail)*rate + weightHail*rateKDP;

			}
			else {
				rate = RainRateOp::precipZrain.rainRate(valueDBZH);
			}

			*it = rateDst.odim.scaleInverse(rate);


		}



		if (DBZH)
			++itDBZH;

		if (ZDR)
			++itZDR;

		if (KDP)
			++itKDP;

	}


}



}

// Rack
