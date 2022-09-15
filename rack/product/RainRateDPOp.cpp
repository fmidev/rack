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

void RainRateDPOp::addDebugProduct(const PlainData<PolarSrc> & srcData, const drain::Fuzzifier<double> & fuzzyFtor,
		 const SingleParamPrecip & rateFnc, DataSet<PolarDst> & dstProduct) const {

	//const drain::Fuzzifier<double> & fuzzyFtor = fuzzyDBZheavy;

	//const std::string quantity("DBZH");

	const std::string & quantity = srcData.odim.quantity;

	const drain::image::Geometry & geometry = srcData.data.getGeometry();

	std::string debugQuantity = "RATE_" + quantity;
	Data<PolarDst> & dstData = dstProduct.getData(debugQuantity);
	// getQuantityMap().setQuantityDefaults(dstData, "RATE", "C");
	// dstData.odim.quantity = "RATE";
	setEncodingNEW(dstData, "RATE");
	dstData.odim.quantity = debugQuantity;

	dstData.setGeometry(geometry);

	PlainData<PolarDst> & dstQuality = dstData.getQualityData("QIND");
	getQuantityMap().setQuantityDefaults(dstQuality, "QIND", "C");
	dstQuality.setGeometry(geometry);

	double x;

	drain::image::Image::const_iterator sit = srcData.data.begin();
	drain::image::Image::iterator dit = dstData.data.begin();
	drain::image::Image::iterator qit = dstQuality.data.begin();

	while (sit != srcData.data.end()){

		if (srcData.odim.isValue(*sit)){
			x = srcData.odim.scaleForward(*sit);
			*dit = dstData.odim.scaleInverse(rateFnc.rainRate(x));
			*qit = dstQuality.odim.scaleInverse(fuzzyFtor(x));
		}
		++sit;
		++dit;
		++qit;
	}


}

void RainRateDPOp::addDebugProduct2(
		const PlainData<PolarSrc> & srcData1, const drain::Fuzzifier<double> & fuzzyFctor1,
		const PlainData<PolarSrc> & srcData2, const drain::Fuzzifier<double> & fuzzyFctor2,
		const DoubleParamPrecip & rateFnc, DataSet<PolarDst> & dstProduct) const {


	//const std::string quantity = srcData1.odim.quantity + srcData2.odim.quantity;
	const drain::image::Geometry & geometry = srcData1.data.getGeometry();

	const std::string debugQuantity = "RATE_" + srcData1.odim.quantity + "x" + srcData2.odim.quantity;
	Data<PolarDst> & dstData = dstProduct.getData(debugQuantity);
	setEncodingNEW(dstData, "RATE");
	dstData.odim.quantity = debugQuantity;

	dstData.setGeometry(geometry);

	PlainData<PolarDst> & dstQuality = dstData.getQualityData("QIND");
	getQuantityMap().setQuantityDefaults(dstQuality, "QIND", "C");
	dstQuality.setGeometry(geometry);

	double x1, x2;

	drain::image::Image::const_iterator sit1 = srcData1.data.begin();
	drain::image::Image::const_iterator sit2 = srcData2.data.begin();
	drain::image::Image::iterator dit = dstData.data.begin();
	drain::image::Image::iterator qit = dstQuality.data.begin();

	while (sit1 != srcData1.data.end()){

		if (srcData1.odim.isValue(*sit1) && srcData2.odim.isValue(*sit2)){
			x1 = srcData1.odim.scaleForward(*sit1);
			x2 = srcData2.odim.scaleForward(*sit2);
			*dit = dstData.odim.scaleInverse(rateFnc.rainRate(x1, x2));
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


	//  Reserve main quantity ("RATE" unless specified) first (so it becomes dataset1/data1)
	//PlainData<PolarDst> & rateDst = dstProduct.getData(odim.quantity);
	Data<PolarDst> & rateDst = dstProduct.getData(odim.quantity);
	setEncodingNEW(rateDst);

	//dstProduct.g
	PlainData<PolarDst> & rateQuality = rateDst.getQualityData("QIND");
	setEncodingNEW(rateDst); // rateDst, not rateQuality?
	getQuantityMap().setQuantityDefaults(rateQuality, "QIND", "C");

	// Geometry
	rateDst.odim.rscale = srcDBZH.odim.rscale;
	rateDst.setGeometry(srcDBZH.data.getGeometry());

	rateQuality.setGeometry(srcDBZH.data.getGeometry());

	mout.debug("src data: ", sweepSrc);
	mout.debug("dst data: ", rateDst.data);

	// RHOHV
	// const PlainData<PolarSrc> & srcRHOHV = sweepSrc.getData("RHOHV");
	// const bool RHOHV = !srcRHOHV.data.isEmpty();

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


	const double d = dbzRange.span() / 4.0;
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

		if (DBZH){
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

		if (DBZH && ZDR){
			mout.special(RainRateOp::precipZZDR);
		}

		if (KDP && ZDR){
			mout.special(RainRateOp::precipKDPZDR);
		}

		if (KDP){
			mout.special(RainRateOp::precipKDP);
		}

		if (DBZH && ZDR){
			mout.special(RainRateOp::precipZZDR);
		}

		if (KDP && ZDR){
			mout.special(RainRateOp::precipKDPZDR);
		}

	}

	mout.warn(parameters);

	// debugging
	if (outputDataVerbosity > 0){

		mout.warn("Writing debug data");

		if (DBZH){
			mout.special(RainRateOp::precipZrain);
			addDebugProduct(srcDBZH, thresholdDBZheavy, RainRateOp::precipZrain, dstProduct);
		}

		if (KDP){
			mout.special(RainRateOp::precipKDP);
			addDebugProduct(srcKDP, thresholdDBZhail, RainRateOp::precipKDP, dstProduct);
		}

		if (DBZH && ZDR){
			mout.special(RainRateOp::precipZZDR);
			addDebugProduct2(srcDBZH, thresholdDBZheavy, srcZDR, thresholdZDR, RainRateOp::precipZZDR, dstProduct);
		}

		if (KDP && ZDR){
			mout.special(RainRateOp::precipKDPZDR);
			addDebugProduct2(srcKDP, thresholdKDP, srcZDR, thresholdZDR, RainRateOp::precipKDPZDR, dstProduct);
		}


		mout.note() << "added: " << dstProduct << mout.endl;

	}

	/// Bin-wise checks
	bool dbzh=false;
	bool zdr=false;
	bool kdp=false;

	double valueDBZH = -32;
	double valueKDP  = 0.0;
	double valueZDR  = 0.0;

	mout.note() << "main loop" <<  mout.endl;

	typedef drain::image::Image::const_iterator src_iter;
	src_iter itDBZH      = srcDBZH.data.begin();
	src_iter itZDR       = srcZDR.data.begin();
	src_iter itKDP       = srcKDP.data.begin();

	typedef drain::image::Image::iterator dst_iter;
	dst_iter itRate    = rateDst.data.begin();
	dst_iter itQuality = rateQuality.data.begin();

	while (itRate != rateDst.data.end()){
	//for (drain::image::Image::iterator it = rateDst.data.begin(); it!=rateDst.data.end(); ++it){

		rate = rateDst.odim.nodata;

		// isValue: true, if value is not 'undetect' or 'nodata'
		if (DBZH)
			dbzh = srcDBZH.odim.isValue(*itDBZH);

		if (ZDR)
			zdr = srcZDR.odim.isValue(*itZDR);

		if (KDP)
			kdp = srcKDP.odim.isValue(*itKDP);



		if (dbzh){

			valueDBZH = srcDBZH.odim.scaleForward(*itDBZH);

			if (kdp){

				valueKDP  = srcKDP.odim.scaleForward(*itKDP);
				weightKDP = thresholdKDP(valueKDP);

				rateKDP = RainRateOp::precipKDP.rainRate(valueKDP);

				if (zdr){
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

				rate = (1.0 - weightHeavy)*rateRain + weightHeavy*rateHeavy;

				// Main branch
				weightHail = thresholdDBZhail(valueDBZH);

				rate = (1.0 - weightHail)*rate + weightHail*rateKDP;

			}
			else {
				//RainRateDPOp
				rate = RainRateOp::precipZrain.rainRate(valueDBZH);
			}

			//rate = RainRateOp::precipZrain.rainRate(valueDBZH);


			//*it = random();
			//rate = RainRateOp::precipZrain.rainRate(valueDBZH);

			*itRate = rateDst.odim.scaleInverse(rate);
			//*itRate = rateDst.odim.scaleInverse(rateKDP);
			//*itQuality = rateQuality.odim.scaleInverse(weightKDP);
			// *itQuality = rateQuality.odim.scaleInverse(weightHail);
			// *itQuality = rateQuality.odim.scaleInverse(weightHeavy);
			*itQuality = rateQuality.odim.scaleInverse((1.0-weightHeavy) * (1.0-weightHail));


		}



		//if (DBZH)
		++itDBZH;

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
