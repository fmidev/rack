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


void RainRateDPOp::computeFuzzyMembership(const PlainData<PolarSrc> & srcData, const drain::Fuzzifier<double> & fuzzyFctor,
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

	double x1, x2;
	typedef drain::typeLimiter<double> Limiter;
	Limiter::value_t limit = drain::Type::call<Limiter>(dstData.data.getType());

	drain::image::Image::const_iterator sit1 = srcData1.data.begin();
	drain::image::Image::const_iterator sit2 = srcData2.data.begin();
	drain::image::Image::iterator dit = dstData.data.begin();
	drain::image::Image::iterator qit = dstQuality.data.begin();

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

	static
	const drain::KeySelector & dbzSelector = qmap.get("DBZ").variants;

	// DBZ - the critical input. If missing, this product fails.
	const PlainData<PolarSrc> & srcDBZ = sweepSrc.getData(dbzSelector);
	if (srcDBZ.data.isEmpty()){
		mout.error("Could not find dBZ data with: ", dbzSelector);
		return;
	}

	PlainData<PolarDst> & dstDBZheavy = dstProduct.getData("FUZZY:DBZ_HEAVY");
	const double d = dbzRange.span() / 4.0;
	drain::FuzzyStep<double> thresholdDBZheavy(dbzRange.min-d, dbzRange.min+d);
	computeFuzzyMembership(srcDBZ, thresholdDBZheavy, dstDBZheavy);

	// ZDR
	static
	const drain::KeySelector & zdrSelector = qmap.get("ZDR").variants;
	const PlainData<PolarSrc> & srcZDR = sweepSrc.getData(zdrSelector);
	PlainData<PolarDst> & dstZDRheavy = dstProduct.getData("FUZZY:ZDR_HEAVY");
	if (!srcZDR.data.isEmpty()){
		drain::FuzzyStep<double> thresholdZDR(zdrRange.min, zdrRange.max);
		computeFuzzyMembership(srcZDR, thresholdZDR, dstZDRheavy);
	}
	else {
		dstZDRheavy.copyGeometry(srcZDR);
		mout.special("No ZDR data found with: ", zdrSelector);
	}

	// KDP
	const PlainData<PolarSrc> & srcKDP = sweepSrc.getData("KDP");



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


	const bool useDBZH  = !srcDBZH.data.isEmpty();

	if (!useDBZH){
		mout.warn("critical DBZH data missing, giving up..." );
		return;
	}


	//  Reserve main quantity ("RATE" unless specified) first (so it becomes dataset1/data1)
	//PlainData<PolarDst> & rateDst = dstProduct.getData(odim.quantity);
	Data<PolarDst> & rateDst = dstProduct.getData(odim.quantity);
	setEncodingNEW(rateDst);

	//dstProduct.g
	PlainData<PolarDst> & rateQuality = rateDst.getQualityData("QIND");
	//setEncodingNEW(rateDst); // rateDst, not rateQuality?
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
	const bool useZDR   = !srcZDR.data.isEmpty();
	if (!useZDR){
		mout.warn("ZDR missing" );
	}

	// KDP
	const PlainData<PolarSrc> & srcKDP = sweepSrc.getData("KDP");
	const bool useKDP = !srcKDP.data.isEmpty();
	if (!useKDP){
		mout.warn("KDP missing" );
	}

	// KDP x DBZH... needed?
	/*
	PlainData<PolarDst> & heavyKDPxDBZH = dstProduct.getData("KDPxDBZH_FUZZY");
	qmap.setQuantityDefaults(heavyKDPxDBZH, "PROB");
	heavyKDPxDBZH.setGeometry(geometry);
	drain::image::BinaryFunctorOp<MultiplicationFunctor> mul;
	mout.warn(heavyDBZH.data     );
	mout.warn(heavyKDP.data.getChannel(0)      );
	mout.warn(heavyKDPxDBZH.data );
	mul.traverseChannel(heavyDBZH.data, heavyKDP.data.getChannel(0), heavyKDPxDBZH.data);
	*/


	//double rateZ      = 0.0;
	//double rateZZDR   = 0.0;
	double rateKDP    = 0.0;
	//double rateKDPZDR = 0.0;

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

		if (useDBZH){
			mout.special(RainRateOp::precipZrain);
			if (mout.isLevel(LOG_DEBUG)){
				for (int i = 0; i < 32; ++i) {
					std::cerr << i << "dBZ\t" << RainRateOp::precipZrain.rainRate(i) << '\n';
				}
			}
		}

		if (useKDP){
			mout.special(RainRateOp::precipKDP);
		}

		if (useDBZH && useZDR){
			mout.special(RainRateOp::precipZZDR);
		}

		if (useKDP && useZDR){
			mout.special(RainRateOp::precipKDPZDR);
		}

		if (useKDP){
			mout.special(RainRateOp::precipKDP);
		}

		if (useDBZH && useZDR){
			mout.special(RainRateOp::precipZZDR);
		}

		if (useKDP && useZDR){
			mout.special(RainRateOp::precipKDPZDR);
		}

	}

	mout.attention("outputDataVerb: ", outputDataVerbosity); // , " ~ ", outputDataVerbosity);
	// debugging
	//if (outputDataVerbosity > 0){
	if (outputDataVerbosity.isSet(OutputDataVerbosity::INTERMEDIATE) || outputDataVerbosity.isSet(OutputDataVerbosity::DEBUG)){

		mout.warn("Writing debug data");

		if (useDBZH){
			mout.special(RainRateOp::precipZrain);
			computeFuzzyMembership(srcDBZH, thresholdDBZheavy, dstProduct.getData("fuzzy_dbz_heavy"));
			computeFuzzyMembership(srcDBZH, thresholdDBZhail,  dstProduct.getData("fuzzy_dbz_hail"));
			computeProduct(srcDBZH, thresholdDBZheavyINV, RainRateOp::precipZrain, dstProduct);
		}

		if (useDBZH && useZDR){
			computeFuzzyMembership(srcZDR, thresholdZDR,      dstProduct.getData("fuzzy_zdr"));
		}


		if (useKDP){
			mout.special(RainRateOp::precipKDP);
			computeFuzzyMembership(srcKDP, thresholdKDP,      dstProduct.getData("fuzzy_kdp"));
			computeProduct(srcKDP, thresholdKDP, RainRateOp::precipKDP, dstProduct);
		}

		if (useDBZH && useZDR){
			mout.special(RainRateOp::precipZZDR);
			computeProduct2(srcDBZH, thresholdDBZheavyINV, srcZDR, thresholdZDR, RainRateOp::precipZZDR, dstProduct);
		}

		if (useKDP && useZDR){
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
	src_iter itDBZH      = srcDBZH.data.begin();
	src_iter itZDR       = srcZDR.data.begin();
	src_iter itKDP       = srcKDP.data.begin();

	typedef drain::image::Image::iterator dst_iter;
	dst_iter itRate    = rateDst.data.begin();
	dst_iter itQuality = rateQuality.data.begin();

	while (itRate != rateDst.data.end()){

		rate = rateDst.odim.nodata;

		// isValue: true, if value not 'undetect' or 'nodata'
		if (useDBZH)
			dbzValid = srcDBZH.odim.isValue(*itDBZH);

		if (useZDR)
			zdrValid = srcZDR.odim.isValue(*itZDR);

		if (useKDP)
			kdpValid = srcKDP.odim.isValue(*itKDP);


		if (dbzValid){

			valueDBZH = srcDBZH.odim.scaleForward(*itDBZH);

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


		if (useDBZH) // Kept here for debugging. In operational version unneeded.
			++itDBZH;

		if (useZDR)
			++itZDR;

		if (useKDP)
			++itKDP;

		++itRate;
		++itQuality;

	}


}



}

// Rack
