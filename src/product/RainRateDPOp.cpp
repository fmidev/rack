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


#include "drain/TypeUtils.h"
#include "drain/image/ImageFile.h"
#include "drain/util/Fuzzy.h"
#include "drain/util/FunctorPack.h"

#include "data/ODIM.h"
#include "data/Data.h"
#include "data/QuantityMap.h"
#include "radar/Geometry.h"
#include "radar/Analysis.h"
#include "radar/Precipitation.h"

#include "RainRateDPOp.h"
#include "RainRateOp.h"


//#include "drain/image/SegmentAreaOp.h"
//#include "drain/image/MathOpPack.h"



//using namespace drain::image;

namespace rack {


void RainRateDPOp::computeFuzzyWeight(const PlainData<PolarSrc> & srcData, const drain::Fuzzifier<double> & fuzzyFctor,
		PlainData<PolarDst> & dstData) const {

	getQuantityMap().setQuantityDefaults(dstData, "QIND", "C");
	// dstData.setGeometry(srcData.data.getGeometry());
	dstData.copyGeometry(srcData);

	drain::image::Image::const_iterator sit = srcData.data.begin();
	drain::image::Image::iterator dit = dstData.data.begin();

	while (sit != srcData.data.end()){

		if (srcData.odim.isValue(*sit)){
			*dit = dstData.odim.scaleInverse(fuzzyFctor(srcData.odim.scaleForward(*sit)));
		}
		++sit;
		++dit;
	}


}


void RainRateDPOp::computeProductNEW(const PlainData<PolarSrc> & srcData, const SingleParamPrecip & rateFnc, DataSet<PolarDst> & dstProduct) const {

	const std::string & quantity = srcData.odim.quantity;
	// const drain::image::Geometry & geometry = srcData.data.getGeometry();

	std::string debugQuantity = "RATE_" + quantity;
	PlainData<PolarDst> & dstData = dstProduct.getData(debugQuantity);
	setEncodingNEW(dstData, "RATE", "S");
	dstData.odim.quantity = debugQuantity;
	dstData.copyGeometry(srcData);


	typedef drain::typeLimiter<double> Limiter;
	Limiter::value_t limit = drain::Type::call<Limiter>(dstData.data.getType());

	drain::image::Image::const_iterator sit = srcData.data.begin();
	drain::image::Image::iterator dit = dstData.data.begin();

	double x;
	while (sit != srcData.data.end()){
		if (srcData.odim.isValue(*sit)){
			x = srcData.odim.scaleForward(*sit);
			*dit = limit(dstData.odim.scaleInverse(rateFnc.rainRate(x)));
		}
		++sit;
		++dit;
	}


}

void RainRateDPOp::computeProductNEW2(const PlainData<PolarSrc> & srcData1, const PlainData<PolarSrc> & srcData2,
		const DoubleParamPrecip & rateFnc, DataSet<PolarDst> & dstProduct) const {

	const std::string & quantity = srcData1.odim.quantity;
	// const drain::image::Geometry & geometry = srcData.data.getGeometry();

	std::string debugQuantity = "RATE_" + quantity;
	PlainData<PolarDst> & dstData = dstProduct.getData(debugQuantity);
	setEncodingNEW(dstData, "RATE", "S");
	dstData.odim.quantity = debugQuantity;
	dstData.copyGeometry(srcData1);


	typedef drain::typeLimiter<double> Limiter;
	Limiter::value_t limit = drain::Type::call<Limiter>(dstData.data.getType());

	drain::image::Image::const_iterator sit1 = srcData1.data.begin();
	drain::image::Image::const_iterator sit2 = srcData2.data.begin();
	drain::image::Image::iterator dit = dstData.data.begin();

	double x1,x2;
	while (sit1 != srcData1.data.end()){
		if (srcData1.odim.isValue(*sit1) && srcData2.odim.isValue(*sit2)){
			x1 = srcData1.odim.scaleForward(*sit1);
			x2 = srcData1.odim.scaleForward(*sit2);
			*dit = limit(dstData.odim.scaleInverse(rateFnc.rainRate(x1, x2)));
		}
		++sit1;
		++sit2;
		++dit;
	}


}


void RainRateDPOp::computeProduct(const PlainData<PolarSrc> & srcData, const drain::Fuzzifier<double> & fuzzyFtor,
		 const SingleParamPrecip & rateFnc, DataSet<PolarDst> & dstProduct) const {

	// const drain::Fuzzifier<double> & fuzzyFtor = fuzzyDBZheavy;
	// const std::string quantity("DBZH");
	const std::string & quantity = srcData.odim.quantity;
	// const drain::image::Geometry & geometry = srcData.data.getGeometry();

	std::string debugQuantity = "RATE_" + quantity;
	Data<PolarDst> & dstData = dstProduct.getData(debugQuantity);
	// getQuantityMap().setQuantityDefaults(dstData, "RATE", "C");
	// dstData.odim.quantity = "RATE";
	setEncodingNEW(dstData, "RATE", "S");
	dstData.odim.quantity = debugQuantity;
	dstData.copyGeometry(srcData);

	PlainData<PolarDst> & dstQuality = dstData.getQualityData("QIND");
	getQuantityMap().setQuantityDefaults(dstQuality, "QIND", "C");
	dstQuality.copyGeometry(srcData);

	double x;

	typedef drain::typeLimiter<double> Limiter;
	Limiter::value_t limit = drain::Type::call<Limiter>(dstData.data.getType());

	drain::image::Image::const_iterator sit = srcData.data.begin();
	drain::image::Image::iterator dit = dstData.data.begin();
	drain::image::Image::iterator qit = dstQuality.data.begin();

	while (sit != srcData.data.end()){

		if (srcData.odim.isValue(*sit)){
			x = srcData.odim.scaleForward(*sit);
			*dit = limit(dstData.odim.scaleInverse(rateFnc.rainRate(x)));
			*qit = dstQuality.odim.scaleInverse(fuzzyFtor(x));
		}
		++sit;
		++dit;
		++qit;
	}


}

void RainRateDPOp::computeProduct2(
		const PlainData<PolarSrc> & srcData1, const drain::Fuzzifier<double> & fuzzyFctor1,
		const PlainData<PolarSrc> & srcData2, const drain::Fuzzifier<double> & fuzzyFctor2,
		const DoubleParamPrecip & rateFnc, DataSet<PolarDst> & dstProduct) const {

	const std::string debugQuantity = "RATE_" + srcData1.odim.quantity + "x" + srcData2.odim.quantity;
	Data<PolarDst> & dstData = dstProduct.getData(debugQuantity);
	setEncodingNEW(dstData, "RATE", "S");
	dstData.odim.quantity = debugQuantity;
	dstData.copyGeometry(srcData1);

	PlainData<PolarDst> & dstQuality = dstData.getQualityData("QIND");
	getQuantityMap().setQuantityDefaults(dstQuality, "QIND", "C");
	dstQuality.copyGeometry(srcData1);

	typedef drain::typeLimiter<double> Limiter;
	Limiter::value_t limit = drain::Type::call<Limiter>(dstData.data.getType());

	drain::image::Image::const_iterator sit1 = srcData1.data.begin();
	drain::image::Image::const_iterator sit2 = srcData2.data.begin();
	drain::image::Image::iterator dit = dstData.data.begin();
	drain::image::Image::iterator qit = dstQuality.data.begin();

	double x1, x2;
	while (sit1 != srcData1.data.end()){

		if (srcData1.odim.isValue(*sit1) && srcData2.odim.isValue(*sit2)){
			x1 = srcData1.odim.scaleForward(*sit1);
			x2 = srcData2.odim.scaleForward(*sit2);
			*dit = limit(dstData.odim.scaleInverse(rateFnc.rainRate(x1, x2)));
			*qit = dstQuality.odim.scaleInverse(fuzzyFctor1(x1) * fuzzyFctor2(x2));
		}
		++sit1;
		++sit2;
		++dit;
		++qit;
	}


}


void RainRateDPOp::processDataSet(const DataSet<PolarSrc> & sweepSrc, DataSet<PolarDst> & dstProduct) const {

	/// For monitoring cmd execution as text dump. Use --debug (or \c --verbose \c \<level\> to define verbosity.
	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.info("start");

	// static
	// const drain::KeySelector dbzSelector("DBZH", "DBZHC", "DBZ");

	static
	const QuantityMap & qmap = getQuantityMap();

	//static	const drain::KeySelector & dbzSelector = qmap.get("DBZ").keySelector;

	// DBZ - the critical input. If missing, this product fails.
	const PlainData<PolarSrc> & srcDBZ = sweepSrc.getData(qmap.DBZ.keySelector);
	const bool DBZ  = !srcDBZ.data.isEmpty();

	PlainData<PolarDst> & dstDBZheavy = dstProduct.getData("FUZZY_DBZ_HEAVY");
	PlainData<PolarDst> & dstDBZhail  = dstProduct.getData("FUZZY_DBZ_HAIL"); // even heavier...

	if (DBZ){
		const double d = dbzRange.span() / 4.0;
		drain::FuzzyStep<double> thresholdDBZheavy(dbzRange.min-d, dbzRange.min+d);
		computeFuzzyWeight(srcDBZ, thresholdDBZheavy, dstDBZheavy);

		drain::FuzzyStep<double> thresholdDBZhail(dbzRange.max-d, dbzRange.max+d);
		computeFuzzyWeight(srcDBZ, thresholdDBZhail, dstDBZhail);

		computeProductNEW(srcDBZ, RainRateOp::precipZrain, dstProduct);

	}
	else {
		mout.fail("Could not find dBZ data with: ", qmap.DBZ.keySelector);
		mout.advice("Without dBZ input, rain rate will fail esp. for weak precip");
	}


	// ZDR
	// static const drain::KeySelector & zdrSelector = qmap.get("ZDR").keySelector;
	const PlainData<PolarSrc> & srcZDR = sweepSrc.getData(qmap.ZDR.keySelector);
	const bool ZDR = !srcZDR.data.isEmpty();
	PlainData<PolarDst> & dstZDRheavy = dstProduct.getData("FUZZY_ZDR_HEAVY");
	if (ZDR){
		drain::FuzzyStep<double> thresholdZDR(zdrRange.min, zdrRange.max);
		computeFuzzyWeight(srcZDR, thresholdZDR, dstZDRheavy);

	}
	else {
		dstZDRheavy.copyGeometry(srcZDR);
		mout.special("No ZDR data found with: ", qmap.ZDR.keySelector);
	}

	// KDP
	// static
	// const drain::KeySelector & kdpSelector = qmap.get("KDP").keySelector;
	const PlainData<PolarSrc> & srcKDP = sweepSrc.getData(qmap.KDP.keySelector);
	PlainData<PolarDst> & dstKDPheavy = dstProduct.getData("FUZZY_KDP_HEAVY");
	const bool KDP = !srcKDP.data.isEmpty();
	if (KDP){
		drain::FuzzyStep<double> thresholdKDP(kdpRange.min, kdpRange.max);
		computeFuzzyWeight(srcKDP, thresholdKDP, dstKDPheavy);
		computeProductNEW(srcKDP, RainRateOp::precipKDP, dstProduct);
	}
	else {
		dstKDPheavy.copyGeometry(srcKDP);
		mout.special("No KDP data found with: ", qmap.KDP.keySelector);
	}


	if (DBZ && ZDR){
		mout.special(RainRateOp::precipZZDR);
		computeProductNEW2(srcDBZ, srcZDR, RainRateOp::precipZZDR, dstProduct);
	}

	if (KDP && ZDR){
		mout.special(RainRateOp::precipKDPZDR);
		computeProductNEW2(srcKDP, srcZDR, RainRateOp::precipKDPZDR, dstProduct);
	}


}



void RainRateDPOp::processDataSetOLD(const DataSet<PolarSrc> & sweepSrc, DataSet<PolarDst> & dstProduct) const {

	/// For monitoring cmd execution as text dump. Use --debug (or \c --verbose \c \<level\> to define verbosity.
	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.info("start");

	// IDEA: FUZZY => PROB quality2/... so that finally class could be extracted?

	/// Retrieve dBZ data
	const PlainData<PolarSrc> & srcDBZH = sweepSrc.getData("DBZH");

	//const drain::image::AreaGeometry & area = srcDBZH.data.getGeometry();
	const QuantityMap & qmap = getQuantityMap();

	mout.note(parameters);


	const bool DBZ  = !srcDBZH.data.isEmpty();

	if (!DBZ){
		mout.warn("critical DBZH data missing, giving up..." );
		return;
	}


	//  Reserve main quantity ("RATE" unless specified) first (so it becomes dataset1/data1)
	//PlainData<PolarDst> & rateDst = dstProduct.getData(odim.quantity);
	Data<PolarDst> & rateDst = dstProduct.getData(odim.quantity);
	setEncodingNEW(rateDst);

	//dstProduct.g
	PlainData<PolarDst> & rateQuality = rateDst.getQualityData("QIND");
	qmap.setQuantityDefaults(rateQuality, "QIND", "C");

	// copy (nbins, nrays, rscale)
	rateDst.copyGeometry(srcDBZH);
	rateQuality.copyGeometry(srcDBZH);

	mout.debug("src data: ", sweepSrc);
	mout.debug("dst data: ", rateDst.data);

	// RHOHV
	// const PlainData<PolarSrc> & srcRHOHV = sweepSrc.getData("RHOHV");
	// const bool RHOHV = !srcRHOHV.data.isEmpty();

	// ZDR
	const PlainData<PolarSrc> & srcZDR = sweepSrc.getData("ZDR");
	const bool ZDR   = !srcZDR.data.isEmpty();
	if (!ZDR){
		mout.warn("ZDR missing" );
	}

	// KDP
	const PlainData<PolarSrc> & srcKDP = sweepSrc.getData("KDP");
	const bool KDP = !srcKDP.data.isEmpty();
	if (!KDP){
		mout.warn("KDP missing" );
	}

	double rateKDP    = 0.0;

	double rate;
	double rateRain = 0.0;
	double rateHeavy = 0.0;


	const double d = dbzRange.span() / 4.0;
	drain::FuzzyStep<double> thresholdDBZheavyINV(dbzRange.min+d, dbzRange.min-d);
	drain::FuzzyStep<double> thresholdDBZheavy(dbzRange.min-d, dbzRange.min+d);

	double weightHeavy;

	drain::FuzzyStep<double> thresholdDBZhail(dbzRange.max-d, dbzRange.max+d);
	double weightHail;

	drain::FuzzyStep<double> thresholdZDR(zdrRange.min, zdrRange.max);
	double weightZDR;

	drain::FuzzyStep<double> thresholdKDP(kdpRange.min, kdpRange.max);
	double weightKDP;

	// debugging
	if (mout.isLevel(LOG_INFO)){

		if (DBZ){
			mout.special(RainRateOp::precipZrain);
			if (mout.isLevel(LOG_DEBUG)){
				for (int i = 0; i < 32; ++i) {
					std::cerr << i << "dBZ\t" << RainRateOp::precipZrain.rainRate(i) << '\n';
				}
			}
		}

		if (KDP){
			mout.special(RainRateOp::precipKDP);
		}

		if (DBZ && ZDR){
			mout.special(RainRateOp::precipZZDR);
		}

		if (KDP && ZDR){
			mout.special(RainRateOp::precipKDPZDR);
		}

		if (KDP){
			mout.special(RainRateOp::precipKDP);
		}

	}

	mout.attention("outputDataVerb: ", outputDataVerbosity); // , " ~ ", outputDataVerbosity);
	// debugging
	//if (outputDataVerbosity > 0){
	if (outputDataVerbosity.isSet(OutputDataVerbosity::INTERMEDIATE) || outputDataVerbosity.isSet(OutputDataVerbosity::DEBUG)){

		mout.warn("Writing debug data");

		if (DBZ){
			mout.special(RainRateOp::precipZrain);
			computeFuzzyWeight(srcDBZH, thresholdDBZheavy, dstProduct.getData("fuzzy_dbz_heavy"));
			computeFuzzyWeight(srcDBZH, thresholdDBZhail,  dstProduct.getData("fuzzy_dbz_hail"));
			computeProduct(srcDBZH, thresholdDBZheavyINV, RainRateOp::precipZrain, dstProduct);
		}

		if (DBZ && ZDR){
			computeFuzzyWeight(srcZDR, thresholdZDR,      dstProduct.getData("fuzzy_zdr"));
		}


		if (KDP){
			mout.special(RainRateOp::precipKDP);
			computeFuzzyWeight(srcKDP, thresholdKDP,      dstProduct.getData("fuzzy_kdp"));
			computeProduct(srcKDP, thresholdKDP, RainRateOp::precipKDP, dstProduct);
		}

		if (DBZ && ZDR){
			mout.special(RainRateOp::precipZZDR);
			computeProduct2(srcDBZH, thresholdDBZheavyINV, srcZDR, thresholdZDR, RainRateOp::precipZZDR, dstProduct);
		}

		if (KDP && ZDR){
			mout.special(RainRateOp::precipKDPZDR);
			computeProduct2(srcKDP, thresholdKDP, srcZDR, thresholdZDR, RainRateOp::precipKDPZDR, dstProduct);
		}


		mout.note("added: " , dstProduct );

	}

	mout.note("main loop");

	/// Bin-wise checks
	bool dbzValid = false;
	bool zdrValid = false;
	bool kdpValid = false;

	double valueDBZH = -32;
	double valueKDP  = 0.0;
	double valueZDR  = 0.0;

	typedef drain::image::Image::const_iterator src_iter;
	src_iter itDBZ      = srcDBZH.data.begin();
	src_iter itZDR       = srcZDR.data.begin();
	src_iter itKDP       = srcKDP.data.begin();

	typedef drain::image::Image::iterator dst_iter;
	dst_iter itRate    = rateDst.data.begin();
	dst_iter itQuality = rateQuality.data.begin();

	while (itRate != rateDst.data.end()){

		rate = rateDst.odim.nodata;

		// isValue: true, if value not 'undetect' or 'nodata'
		if (DBZ)
			dbzValid = srcDBZH.odim.isValue(*itDBZ);

		if (ZDR)
			zdrValid = srcZDR.odim.isValue(*itZDR);

		if (KDP)
			kdpValid = srcKDP.odim.isValue(*itKDP);


		if (dbzValid){

			valueDBZH = srcDBZH.odim.scaleForward(*itDBZ);

			if (kdpValid){

				valueKDP  = srcKDP.odim.scaleForward(*itKDP);
				weightKDP = thresholdKDP(valueKDP);

				rateKDP = RainRateOp::precipKDP.rainRate(valueKDP);

				if (zdrValid){
					valueZDR  = srcZDR.odim.scaleForward(*itZDR);
					weightZDR = thresholdZDR(valueZDR);
					// Z & ZDR
					rateRain  = (1.0 - weightZDR)*RainRateOp::precipZrain.rainRate(valueDBZH) + weightZDR*RainRateOp::precipZZDR.rainRate(valueDBZH, valueZDR);
					// KDP & ZDR
					rateHeavy = (1.0 - weightZDR)*RainRateOp::precipKDP.rainRate(valueKDP)    + weightZDR*RainRateOp::precipKDPZDR.rainRate(valueKDP, valueZDR);
				}
				else {
					// Z
					rateRain = RainRateOp::precipZrain.rainRate(valueDBZH);
					// KDP
					rateHeavy = rateKDP;
				}


				// heavy vs. moderate
				weightHeavy = weightKDP * thresholdDBZheavy(valueDBZH);

				rate = (1.0 - weightHeavy)*rateRain + (weightHeavy)*rateHeavy;

				// Main branch
				weightHail = thresholdDBZhail(valueDBZH);

				rate = (1.0 - weightHail)*rate + (weightHail)*rateKDP;

			}
			else {
				//RainRateDPOp
				rate = RainRateOp::precipZrain.rainRate(valueDBZH);
			}

			// rate = RainRateOp::precipZrain.rainRate(valueDBZH);
			//  *it = random();
			// rate = RainRateOp::precipZrain.rainRate(valueDBZH);

			*itRate = rateDst.odim.scaleInverse(rate);
			// *itRate = rateDst.odim.scaleInverse(rateKDP);
			// *itQuality = rateQuality.odim.scaleInverse(weightKDP);
			// *itQuality = rateQuality.odim.scaleInverse(weightHail);
			// *itQuality = rateQuality.odim.scaleInverse(weightHeavy);
			*itQuality = rateQuality.odim.scaleInverse((1.0-weightHeavy) * (1.0-weightHail));

		}


		if (DBZ) // Kept here for debugging. In operational version unneeded.
			++itDBZ;

		if (ZDR)
			++itZDR;

		if (KDP)
			++itKDP;

		++itRate;
		++itQuality;

	}


}



}

// Rack
