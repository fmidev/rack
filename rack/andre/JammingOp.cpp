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

#include <algorithm>



//#include <drain/image/SegmentAreaOp.h>
#include <drain/util/Math.h>

#include <drain/util/Fuzzy.h>
//#include <drain/image/MathOpPack.h>

#include <drain/image/FastAverageOp.h>
#include <drain/image/MarginalStatisticOp.h>
#include <drain/image/DistanceTransformFillOp.h>
//#include <drain/image/FuzzyPeakOp.h>
//#include <drain/image/FuzzyThresholdOp.h>
//#include <drain/image/SlidingWindowMedianOp.h>
//#include "image/GammaOp.h"
#include <drain/image/HighPassOp.h>
#include <drain/image/SlidingWindowHistogramOp.h>
//#include <drain/image/ThresholdOp.h>

// debugging
#include <drain/image/File.h>

#include "data/ODIM.h"
#include "JammingOp.h"


// using namespace std;

using namespace drain;
using namespace drain::image;

namespace rack {



void JammingOp::processData(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const {
//void JammingOp::filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const {

	drain::MonitorSource mout(name, __FUNCTION__);
	//mout.debug() << *this << mout.endl;

	mout.debug() << *this << mout.endl;

	//mout.writeImage(10, src, "src");

	const size_t width  = src.data.getWidth();
	const int height    = src.data.getHeight();

	// Debugging
	if (mout.isDebug(10) && (debugRow >= 0)) {
		Image tmp;
		tmp.copyDeep(src.data);
		for (size_t i = 0; i < width; ++i) {
			if (i & 16)
				tmp.put(i, debugRow, 255);
		}
		File::write(tmp, name+"1-srcrow.png");
		//mout.writeImage(11, tmp, "src-marked");
	}


	/// STEP 1: Compute the standard-deviation in a 3x5 window

	// To handle nodata and undetect, a weight image has to be applied
	// TODO: generalize this
	Image srcWeight;
	srcWeight.setGeometry(src.data.getGeometry());
	Image::iterator  it = src.data.begin();
	Image::iterator wit = srcWeight.begin();
	while (it != src.data.end()){
		if ((*it != src.odim.nodata) && (*it != src.odim.undetect))
			*wit = 255;
		else
			*wit = 0;
		++it;
		++wit;
	}

	Image stdDev;
	//stdDev.setType<double>();
	//stdDev.initialize(typeid(float), src.getGeometry());
	SlidingWindowHistogramOp(7,1,"d").filter(src.data, srcWeight, stdDev, dst.data); // "d" = standard deviation
	//mout.writeImage(11, stdDev, "stdDev");
	//mout.writeImage(11, dst, "stdDevW");


	/// STEP 1b: Convert std.deviation to fuzzy smoothness value. The largest response is around undetectValue deviation.
	UnaryFunctorOp<FuzzyBell<double> > fuzzyBell;
	fuzzyBell.functor.set(0.0, smoothnessThreshold*2.0, 255.0);
	fuzzyBell.filter(stdDev, stdDev);
	//FuzzyBellOp(0.0, smoothnessThreshold*2.0, 255.0).filter(stdDev, stdDev);
	//mout.writeImage(11, stdDev, "stdDevFuzzy");

	//Image srcSmooth(typeid(double));
	Image srcSmooth;
	const Image & srcSmoothWeight = dst.data; // weight of the weighted std.dev.
	//SlidingWindowHistogramOp(21, 1, "m", 0.1).filter(src,  srcWeight, srcSmooth, dst); // "m" median
	SlidingWindowHistogramOp(10000.0/src.odim.rscale, 1, "a", 0.1).filter(src.data,  srcWeight, srcSmooth, dst.data); // "a" => average in 10 km window
	//mout.writeImage(11, srcSmooth, "src-med");
	//mout.writeImage(11, srcSmoothWeight, "src-med-w");


	const size_t iStart = (distanceMin*1000.0 - src.odim.rstart) / src.odim.rscale ;
	mout.debug() << "iStart=" << iStart << mout.endl;



	/// Measure of fit between original dBZ data and matched curve.
	FuzzyBell<double> fuzzyFitMeasure(0.0, smoothnessThreshold, 1.0);

	/// Similarity measure of slopes of the smoothed dBZ and matched curve.
	// drain::FuzzyPeak<double,double> fuzzySlopeSimilarityMeasure(0.0, derivativeDifferenceMax, 1.0); // dBZ per km

	/// Horizonal coordinate (i) in double precision
	//double x;
	/// Distance from radar
	double r;
	/// Original data value
	double dBZ;
	/// Smoothness (fuzzified stdDev)
	double smoothness;

	/// Modelled constant-power transmitter curve, in dBZ
	double dbzModel;
	double dbzModelBase = 0.0;

	double sum, sumWeights;

	for (int j = 0; j < height; ++j) {

		/// Step 1: fi
		sum = 0.0;
		sumWeights = 0.0;
		// sumR = 0.0;
		// sumRR = 0.0;
		// sumSR = 0.0;
		for (size_t i = iStart; i < width; ++i) {
			if (srcSmoothWeight.get<int>(i,j) > 0){
				r = src.odim.rstart + static_cast<double>(i)*src.odim.rscale;
				dBZ = src.odim.scaleForward(srcSmooth.get<double>(i, j));
				// dBZ power curve
				// dbzModel = modelledEmitter(0.0, r);
				smoothness = stdDev.get<float>(i, j);
				sum += smoothness*(dBZ - modelledEmitter(0.0, r)); // simple!
				//sumR = r
				sumWeights += smoothness;
			}
		}

		if (sumWeights > 0.0)
			dbzModelBase = sum / sumWeights;
		else
			continue;

		/// Step 2: refit
		if (weightLower != 1.0){
			sum = 0.0;
			sumWeights = 0.0;
			for (size_t i = iStart; i < width; ++i) {
				r = src.odim.rstart + static_cast<double>(i)*src.odim.rscale;
				dBZ = src.odim.scaleForward(srcSmooth.get<double>(i, j));
				dbzModel = modelledEmitter(dbzModelBase, r);
				smoothness = stdDev.get<float>(i, j);
				if (dBZ < dbzModel)
					smoothness *= 10.0;
				sum += smoothness*(dBZ - dbzModel); // simple!
				sumWeights += smoothness;
			}
			dbzModelBase += sum / sumWeights;
		}

		/// Derive probability from fuzzy difference measure and fuzzy smoothness measure
		//
		//const int widthI = src.getWidth();
		// r*r*dm
		double fit; //, r2;
		// inertia; sumR=0.0,
		//double sumR2=0.0, sumR2M=0.0, sumM=0.0, sumRM=0.0;
		double distanceMaxM = static_cast<double>(src.odim.nbins)*src.odim.rscale;
		double distanceMinM = distanceMin * 1000.0;
		drain::FuzzyBell<double> fuzzyLocation(0.0, (distanceMaxM-distanceMinM)/10.0, 1.0);
		double weight;
		double sumW = 0.0;
		double sumFitW = 0.0;
		//unsigned int n=0;
		for (unsigned int i = 0; i < width; ++i) {

			//dBZ = srcSmooth.get<double>(i, j);
			dBZ = src.data.get<double>(i, j);


			if ((dBZ != src.odim.nodata) && (dBZ != src.odim.undetect)){

				dBZ = src.odim.scaleForward(srcSmooth.get<double>(i, j));

				r = src.odim.rstart + static_cast<double>(i)*src.odim.rscale;
				dbzModel = modelledEmitter(dbzModelBase, r);

				//smoothness = stdDev.get<float>(i, j);

				//derivativeModelled = derivativeDBZ_Modelled(dbzModelBase, r, src.odim.rscale);
				//derivativeDetected = derivativeDBZ(src.odim, srcSmooth, i, j, 20);
				//fit = smoothness/255.0 * fuzzyFitMeasure(dbzModel-dBZ);
				fit = fuzzyFitMeasure(dbzModel-dBZ);

				dst.data.put(i, j, 255.0 * fit);


				weight = std::max(fuzzyLocation(r-distanceMinM-20000.0), fuzzyLocation(r-distanceMaxM+20000.0)); // 20 km after distanceMin, 20 km before max.
				sumFitW += weight*fit;
				sumW    += weight;


				// Inertia
				/*
				r = r*0.001;
				r2 = r*r; // (km2)
				sumR2M += r2*fit;
				sumRM  += r*fit;
				sumR2  += r2;
				//sumR   += r;
				sumM   += fit;
				//n++;
				//dst.put(i, j, smoothness * fit * fuzzySlopeSimilarityMeasure(derivativeDetected - derivativeModelled) );
				//dst.put(i, j, r2 );
				dst.put(i, j, 255.0 * fit);
				//dst.put(i, j, 255.0 * fuzzySlopeSimilarityMeasure(derivativeDetected - derivativeModelled));
				//dst.put(i, j, smoothness);
				 */
			}
			//if (j>330)
			//	dst.put(i, j, 255.0 * weight);

		}

		if (sumW > 0.0){
			fit = sumFitW/sumW;
			fit = fit*fit;
		}
		else
			fit = 0.0;

		//const double SCALE250 = 250.0/255.0;
		fit =  250.0/255.0*fit;

		for (unsigned int i = 0; i < width; ++i) {
			dst.data.put(i, j, fit * dst.data.get<double>(i, j));
		}
		/*
		const double Rm  = sumRM/sumM; //static_cast<double>(n);
		const double Rm2 = Rm*Rm;
		fit = (sumR2M - Rm2*sumM) / (sumR2 - Rm2) / sumM;


		if ((j&16) == 16){
			std::cout << "fit=" << fit << ",\t Rm=" << Rm  << ",\t sumM=" << sumM  << ",\t sumR2M=" << sumR2M  << ", sumR2=" << sumR2 << '\n';
		}

		// for (unsigned int i = width-60; i < width-50; ++i)
		//	dst.put(i, j, 255.0* sumM/static_cast<double>(n));
		for (unsigned int i = width-50; i < width-40; ++i)
			dst.put(i, j, Rm*2.0);
		for (unsigned int i = width-40; i < width-30; ++i)
			dst.put(i, j, 1.1234 * sumR2M);
		for (unsigned int i = width-30; i < width-20; ++i)
			dst.put(i, j, 1.1234 * sumR2);
		//for (unsigned int i = width-20; i < width-10; ++i)
		//	dst.put(i, j, 255.0 * sumR);

		for (unsigned int i = width-20; i < width; ++i)
			dst.put(i, j, 25500.0 * fit);
		 */

		// Debugging
		//
		// Dump all the features on a given beam. Store them as text file *.dat.  FIXME!
		if (false) // TODO
		if (j == debugRow){
			//_mout.task = "dump";
			std::ofstream ofstr((name+".dat").c_str(), std::ios::out);
			ofstr << "# r dBZ dBZ_smooth dBZ_modelled smoothness fit derivMod deriv\n";
			for (size_t i = 1; i < width; ++i) {

				/// Range
				r = src.odim.rstart + static_cast<double>(i)*src.odim.rscale;

				/// dBZ
				dBZ = src.odim.scaleForward(srcSmooth.get<double>(i, debugRow));

				//dbzModel = dbzModelBase + r*(0.016/1000.0) + 20.0*log10(r);
				dbzModel = modelledEmitter(dbzModelBase, r);

				//Smoothness
				smoothness = stdDev.get<float>(i, debugRow);
				//std::cout << i << '\t' << src.get<double>(i, debugRow);
				//std::cout << i;
				ofstr << r;
				ofstr << '\t' << src.odim.scaleForward(src.data.get<double>(i, debugRow));
				ofstr << '\t' << dBZ;
				ofstr << '\t' << dbzModel;
				ofstr << '\t' << (smoothness/26.0);  // smoothness
				ofstr << '\t' << 10.0 * fuzzyFitMeasure(dbzModel-dBZ);
				ofstr << '\t' << 5000.0*derivativeDBZ_Modelled(dbzModelBase, r, src.odim.rscale);;
				ofstr << '\t' << 5000.0*derivativeDBZ(src.odim, srcSmooth, i, debugRow);
				// std::cout << '\t' << ((w[i]>0.0) ? 100:0.0);
				//std::cout << '\t' << yFitStart << '\t' << yFitEnd << '\t' << ySpanMax << '\t' << spanMax;
				//std::cout << '\t' << yFitStart + (x/static_cast<double>(width))*(yFitEnd-yFitStart) << '\t' << ySpanMax << '\t' << spanMax;
				ofstr << '\n';
			}
			// std::cerr << "#inertia: " << (momemtumOfInertia/momemtumOfInertiaFlat) << "\t (" << momemtumOfInertia << " / " << momemtumOfInertiaFlat << ")" << std::endl;

		}

	}

	//_mout.writeImage(11, dst, "dst");



}













/*
void JammingOp::fitCurve(const std::vector<double> &src, const std::vector<double> &weight, std::vector<double> &coeff){

	coeff.resize(3);

	//const size_t n = src.size();

	double x = 0.0;
	drain::Matrix<double> XTX(3,3);
	**
	 *    XtX = SUM{}[1 x log]^T[1 x log] =
	 *    SUM{}
	 *    1   X    LOG
	 *    X   X2   XLOG
	 *    LOG XLOG LOGLOG
	 *
	 *
	double l; // log(x)
	double sum1, sumX, sumLOG, sumX2, sumXLOG, sumLOGLOG;
	sum1 = sumX = sumLOG = sumX2 = sumXLOG = sumLOGLOG = 0.0;
	drain::Matrix<double> XTXinv(3,3);

	double y;
	drain::Matrix<double> XTy(3,1);
	double & sumY   = XTy(0);
	double & sumYX  = XTy(1);
	double & sumYLOG = XTy(2);
	sumY = sumYX = sumYLOG = 0.0;

	double w;

	//for (size_t i = 0; i < n; ++i) {
	std::vector<double>::const_iterator its = src.begin();
	std::vector<double>::const_iterator itw = weight.begin();

	while (its != src.end()){

		x += 1.0;
		y = *its;
		w = *itw;
		l = log(x); // (x > 0.0, ok)

		sum1 +=     w* 1.0;
		sumX +=     w* x;
		sumLOG +=   w* l;

		sumX2   +=   w* x*x;
		sumXLOG +=   w* x*l;
		sumLOGLOG += w* l*l;

		sumY    += w* y*1.0;
		sumYX   += w* y*x;
		sumYLOG += w* y*l;

		++its;
		++itw;
	}

	XTX(0,0) = sum1;
	XTX(0,1) = XTX(1,0) = sumX;
	XTX(0,2) = XTX(2,0) = sumLOG;
	XTX(1,1) = sumX2;
	XTX(1,2) = XTX(2,1) = sumXLOG;
	XTX(2,2) = sumLOGLOG;

	XTX.inverse(XTXinv);  // Throws a runtime error, if fails.

	drain::Vector<double> coeffVector(3);
	XTXinv.multiply( XTy, coeffVector);
	coeff = (const std::vector<double> &)coeffVector;


}
*/
/*
void JammingOp::filterImageOLD(const PolarODIM &src.odim, const Image &src, Image &dst) const {

	drain::MonitorSource mout(drain::monitor, "JammingOp::filterImage");

	const size_t width  = src.getWidth();
	//const size_t height = src.getHeight();
	const int height = src.getHeight();

	// Debugging
	if (mout.isDebug(10) && (debugRow >= 0)) {
		Image tmp;
		tmp.copyDeep(src);
		for (size_t i = 0; i < width; ++i) {
			tmp.put(i, debugRow, 255);
		}
		File::write(tmp, "Jamming1-srcrow.png");
	}


	/// STEP 1: Compute the standard-deviation in a 3x5 window

	//  To handle nodata and undetect, a weight image has to be applied
	Image srcWeight;
	srcWeight.setGeometry(src.getGeometry());
	Image::iterator  it = src.begin();
	Image::iterator wit = srcWeight.begin();
	while (it != src.end()){
		if ((*it != odimIn.nodata) && (*it != odimIn.undetect))
			*wit = 255;
		else
			*wit = 0;
		++it;
		++wit;
	}

	Image stdDev;
	//stdDev.setType<double>();
	//stdDev.initialize(typeid(float), src.getGeometry());
	SlidingWindowHistogramOp(7,1,"d").filter(src, srcWeight, stdDev, dst); // "d" = standard deviation
	//op.valueFunc = "d"; // standard deviation
	//op.filter(src, srcWeight, stdDev, dst);
	if (mout.isDebug(10)){
	//if (true){
	  File::write(stdDev, "Jamming2-stdDev.png" );
	  File::write(dst, "Jamming3-stdDevW.png" );
	}

	/// STEP 1b: fuzzify the result, largest response around undetectValue.
	FuzzyPeakOp(0.0, smoothnessThreshold, 255.0).filter(stdDev, stdDev);
	//ThresholdOp(128.0).filter(stdDev, stdDev);
	if (mout.isDebug(1)){
	//if (true){
			File::write(stdDev, "Jamming5_stdDevFuzzy.png");
	}

	/// Filter small segments off using morphological opening.
	//Image srcSmooth;
	//SlidingWindowHistogramOp(11, 5, "m", 0.10).filter(stdDev, srcSmooth);
	//SlidingWindowHistogramOp(11, 5, "m", 0.80).filter(srcSmooth, stdDev);


	if (mout.isDebug(1)){
		//File::write(srcSmooth, "Jamming52_stdDev_erode.png");
		File::write(stdDev, "Jamming53_stdDev_close.png");
	}

	Image srcSmooth;
	/// Minimun.
	//SlidingWindowHistogramOp(21,9,"N").filter(src, srcSmooth);
	SlidingWindowHistogramOp(21, 3, "m", 0.1).filter(src,  srcWeight, srcSmooth, dst);
	if (mout.isDebug(1)){
		File::write(srcSmooth, "Jamming54_minimum.png");
	}

	//ThresholdOp(128.0).filter(stdDev, stdDev);

	const size_t iStart = (distanceMin*1000.0-odimIn.rstart) / odimIn.rscale ;
	mout.note() << "iStart=" << iStart << '\n';

	/// Vectors in which row data will be copied
	std::vector<double> s(width); // source data
	std::vector<double> w(width); // weight
	std::vector<double> w2(width); // weight after re-fitting
	/// Coefficient vector (bi's in the polynomial b0 + b1*x + b2*x*x)
	//drain::Matrix<double> coeffVector(3,1);
	drain::Vector<double> coeffVector(3);  // result of the first fitting
	drain::Vector<double> coeffVector2(3); // after re-fitting


	// Used in computing the total stddev along the beam,
	drain::FuzzyPeak<double,double> fuzzyFit(0.0, smoothnessThreshold, 1.0);


	/// Beam-wise indications

	//  Number of data points along the beam. Computed as a function of data (non-nodata) bins along a beam.
	//double fuzzyCont;
	//drain::FuzzyStep<double,double> fuzzyContent(sampleContent*static_cast<double>(width), width/2.0, 1.0);
	//int undetectCounter;

	// Measures the increase of dbz values along the beam
	//double fuzzyPos;
	//drain::FuzzyStep<double,double> fuzzyPositivity(positivity, 0.5*positivity, 1.0);   // based on coeffVector(1)

	// Measures the inverse of curvature
	//double fuzzyLin;
	//drain::FuzzyPeak<double,double> fuzzyLinearity(0.0, maxCurvature, 1.0); // based on coeffVector(2)

	/// vector [1 x logx]
	drain::Vector<double> xVector(3, false);
	xVector(0) = 1.0;

	drain::Vector<double> xEnd(3, false);
	xEnd(0,0) = 1.0;
	xEnd(0,1) = width;
	xEnd(0,2) = log(static_cast<double>(width));

	//std::cerr << xVector << std::endl;
	//std::cerr << xEnd    << std::endl;
	//exit(1);

	/// Horizonal coordinate (i) in double precision
	double x;
	/// Original data value in double precision
	double y;
	/// Polynomially approximated value f(x) in double precision
	double yFitted;
	/// Difference yFitted-y
	double fit;



	//for (size_t j = 0; j < height; ++j) {
	for (int j = 0; j < height; ++j) {

		/// Step A: collect samples for 2nd order curve fitting
		//sumI0 = sumI1 = sumI2 = 0;
		//undetectCounter = 0;
		for (size_t i = iStart; i < width; ++i) {
			//y = src.get<double>(i, j);
			y = srcSmooth.get<double>(i, j);
			x = stdDev.get<float>(i, j);
			s[i] = y;
			w[i] = x;
			//if ((y != odimIn.nodata) && (y != odimIn.undetect) && (x > 128.0)){
			//if ((y != odimIn.nodata) && (y != odimIn.undetect) && (x > 0.0)){
			//if ((y == odimIn.nodata) || (y == odimIn.undetect))
			//	++undetectCounter;
		}

		/// Fit the samples, ie. derive coeffVector[0], coeffVector[1], coeffVector[2]
		try {
			std::vector<double> v;
			//(s, w, v);
			fitCurve(s, w, v);
			coeffVector = v;
		}
		catch (std::runtime_error &e){
			//mout.debug(5) << "row" << j << ", " << (width-undetectCounter) << " samples,  error: " << e.what() << mout.endl;
			mout.debug(5) << "row" << j << ", error: " << e.what() << mout.endl;
			//continue;
			//coeffVector.fill(0.0);
			mout.debug(8) << coeffVector << mout.endl;
			//exit(-1);
			continue;
		}

		/// Re-fit
		for (size_t i = iStart; i < width; ++i) {
			x = static_cast<double>(i);
			//y = s[i];
			xVector(1) = x;
			xVector(2) = log(x);
			yFitted = xVector.innerProduct(coeffVector);
			fit = yFitted - s[i];
			if (fit > 0.0)
				w2[i] = w[i] *  fuzzyFit(fit);
			else
				w2[i] = w[i] *  fuzzyFit(2.0*fit);
		}

		try {
			//fitCurve(s, w2, coeffVector2);
			std::vector<double> v;
			fitCurve(s, w2, v);
			coeffVector2 = v;
			//Curve(s, w2, coeffVector2);
		}
		catch (std::runtime_error &e){

			//mout.debug(8) << "refit row" << j << ", " << (width-undetectCounter) << " samples,  error: " << e.what() << mout.endl;
			mout.debug(8) << "refit row:" << j << ", error: " << e.what() << mout.endl;
			//continue;
			//coeffVector2.clear();
			//coeffVector2.fill(0.0);
			//mout.debug(8) << coeffVector2 << mout.endl;
			continue;
		}




		/// Compute difference
		for (size_t i = 0; i < width; ++i) {
			y = src.get<double>(i, j);
			if ((y != odimIn.nodata) && (y != odimIn.undetect)){
				x = static_cast<double>(i);
				xVector(1) = x;
				xVector(2) = log(x);
				yFitted = xVector.innerProduct(coeffVector2);
				//dst.put(i, j, fuzzyPos * fuzzyLin * stdDev.get<float>(i,j) * fuzzyFit(yFitted-y));
				dst.put(i, j, stdDev.get<float>(i,j) * fuzzyFit(yFitted-y));
			}
		}

		/// Debugging algorithm: Mark positivity and linearity responses in the image
		if (mout.isDebug(25)){
			//for (size_t i = 20; i < 40; ++i) dst.put(i, j, fuzzyCont*255.0);
			//for (size_t i = 0;  i < 20; ++i) dst.put(i, j, fuzzyPos*255.0);
			//for (size_t i = 20; i < 40; ++i) dst.put(i, j, fuzzyLin*255.0);
			//for (size_t i = 80; i < 100; ++i) dst.put(i, j, );
		}

		//# Debug
		if (j == debugRow){
			std::cout << "# x dbz mask mask2 yFit yFit2 yLine ySpanMax spanMax\n";
			std::cout << "# x coeffVector:  " << coeffVector(0)  << ' ' << coeffVector(1)  << ' ' << coeffVector(2)  << '\n';
			std::cout << "# x coeffVector2: " << coeffVector2(0) << ' ' << coeffVector2(1) << ' ' << coeffVector2(2) << '\n';
			for (size_t i = 0; i < width; ++i) {
				x = static_cast<double>(i);
				xVector(1) = x;
				xVector(2) = log(x);
				//std::cout << i << '\t' << src.get<double>(i, debugRow);
				std::cout << i;
				std::cout << '\t' << src.get<double>(i, debugRow);
				std::cout << '\t' << srcSmooth.get<double>(i, debugRow);
				std::cout << '\t' << w[i] / 10.0; // scale to [0,10.0]
				std::cout << '\t' << w2[i] / 10.0; // scale to [0,10.0]
				std::cout <<	'\t' << xVector.innerProduct(coeffVector);
				std::cout <<	'\t' << xVector.innerProduct(coeffVector2);
				// std::cout << '\t' << ((w[i]>0.0) ? 100:0.0);
				//std::cout << '\t' << yFitStart << '\t' << yFitEnd << '\t' << ySpanMax << '\t' << spanMax;
				//std::cout << '\t' << yFitStart + (x/static_cast<double>(width))*(yFitEnd-yFitStart) << '\t' << ySpanMax << '\t' << spanMax;
				std::cout << '\n';
			}
			// std::cerr << "#inertia: " << (momemtumOfInertia/momemtumOfInertiaFlat) << "\t (" << momemtumOfInertia << " / " << momemtumOfInertiaFlat << ")" << std::endl;

		}

	}


}
*/



} // rack::

// Rack
