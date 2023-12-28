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
//#include <math.h>

#include <drain/image/ImageFile.h> // debugging
#include "drain/util/Fuzzy.h"
#include "drain/util/FunctorBank.h"

#include "drain/imageops/DistanceTransformOp.h"
#include "drain/imageops/GammaOp.h"
#include "drain/imageops/GaussianAverageOp.h"
#include "drain/imageops/SegmentAreaOp.h"

#include "radar/Constants.h"
#include "radar/Analysis.h"
#include "radar/PolarWindow.h"
#include "radar/PolarSmoother.h"



#include "CappiOp.h"
#include "EchoTopOp.h"
#include "MaxEchoOp.h"

#include "PolarSlidingWindowOp.h"

#include "ConvOp.h"



namespace rack
{

using namespace drain::image;


// void ConvOp::filter(const Hi5Tree &src, const std::map<double,std::string> & srcPaths, Hi5Tree &dst) const {
void ConvOp::computeSingleProduct(const DataSetMap<PolarSrc> & srcSweeps, DataSet<PolarDst> & dstProduct) const {


	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.debug2("start" );

	//const CoordinatePolicy polarCoordPolicy(CoordinatePolicy::POLAR, CoordinatePolicy::WRAP, CoordinatePolicy::LIMIT,CoordinatePolicy::WRAP);

	/// Use always maxEcho (consider changing to cappi?)
	/// cell and echo top are optional, but used by default.
	const bool USE_FCELL = (cellDiameter>0.0);
	const bool USE_FETOP = (echoTopThreshold>0.0);


	const QuantityMap & qm = getQuantityMap();

	//mout.warn(quantityMap.get("DBZH").get('C').toStr() );
	MaxEchoOp maxOp;
	maxOp.odim.updateFromMap(qm.get("DBZH").get('C'));
	maxOp.odim.quantity = "MAX";
	maxOp.altitude.set(500,1000);
	//maxOp.altitude    = 1000; //echoTopThreshold/2;
	//maxOp.devAltitude = 500; //echoTopThreshold/2;
	maxOp.computeSingleProduct(srcSweeps, dstProduct);
	Data<PolarDst> & maxEcho = dstProduct.getData(maxOp.odim.quantity); // ensure
	//mout.warn("maxEcho " , maxEcho.odim );
	//maxEcho.updateTree2();
	//mout.warn("maxEcho " , maxEcho );

	CappiOp cappiOp;
	if (USE_FCELL){
		//cappiOp.setEncodingRequest(quantityMap.get("DBZH").get('C').getValues());
		cappiOp.odim.updateFromMap(qm.get("DBZH").get('C'));
		cappiOp.odim.quantity = "CAPPI";
		cappiOp.altitude = 2000;
		cappiOp.computeSingleProduct(srcSweeps, dstProduct);
	}
	Data<PolarDst> & cappi = dstProduct.getData(cappiOp.odim.quantity);
	dstProduct.updateTree3(cappi.odim); // IMPORTANT
	//cappi.updateTree2();
	mout.warn("cappi " , cappi );

	EchoTopOp echoTopOp;
	if (USE_FETOP){
		echoTopOp.odim.updateFromMap(qm.get("HGHT").get('C'));
		//echoTopOp.setEncodingRequest(quantityMap.get("HGHT").get('C').getValues());
		echoTopOp.odim.quantity = "ETOP";
		echoTopOp.minDBZ = echoTopDBZ; // maxEchoThreshold; //echoTopDBZ;
		echoTopOp.computeSingleProduct(srcSweeps, dstProduct);
	}
	Data<PolarDst> & echoTop = dstProduct.getData(echoTopOp.odim.quantity);



	// Product (ie. the result)
	Data<PolarDst> & dstData = dstProduct.getData(odim.quantity); // main data
	qm.setQuantityDefaults(dstData, odim.quantity);
	// See code at the end
	/*
	if (!targetEncoding.empty()){
		dstData.odim.setValues(targetEncoding);
		//dstData.odim.updateFromMap(odim);
	}
	*/
	deriveDstGeometry(srcSweeps, dstData.odim);
	mout.note("dstData.odim: " , ODIM(dstData.odim) );

	// Product quality (?)
	PlainData<PolarDst> & dstQuality = dstData.getQualityData();
	qm.setQuantityDefaults(dstQuality, "QIND");

	const drain::image::Geometry g(dstData.odim.area.width, dstData.odim.area.height);
	dstData.data.setGeometry(g);
	dstData.data.clear();
	//mout.warn("Destination data:" , dstData );

	dstQuality.data.setGeometry(g);
	dstQuality.data.fill(dstQuality.odim.scaling.inv(1.0));
	//@ dstQuality.updateTree();

	//@ dstProduct.updateTree(dstData.odim);


	/// Step 2: fuzzification
	Data<PolarDst> & fuzzyCore = dstProduct.getData("FMAX");
	qm.setQuantityDefaults(fuzzyCore, "PROB");
	//@ fuzzyCore.updateTree();
	//
	RadarFunctorOp<drain::FuzzyStepsoid<double> > fuzzyMaxEchoOp;
	fuzzyMaxEchoOp.odimSrc = maxEcho.odim;
	fuzzyMaxEchoOp.functor.set(maxEchoThreshold, fabs(maxEchoThreshold)/20.0);
	fuzzyMaxEchoOp.process(maxEcho.data, fuzzyCore.data);


	Data<PolarDst> & fuzzyCell = dstProduct.getData("FCELL");
	qm.setQuantityDefaults(fuzzyCell, "PROB");
	//@ fuzzyCell.updateTree();  // if (ProductOp::outputDataVerbosity > 0){
	//
	if (USE_FCELL){
		//RadarWindowAvg<RadarWindowConfig> avg;
		//PolarSlidingAvgOp avgOp;
		//avgOp.processData()
		PolarSmoother::filter(dstData.odim, cappi.data, fuzzyCell.data, cellDiameter*1000.0);

		// NEW

		Data<PolarDst> & fuzzyCell2 = dstProduct.getData("FCELL2");
		qm.setQuantityDefaults(fuzzyCell2, "PROB");
		PolarSlidingAvgOp avgOp;
		//avgOp.conf.
		avgOp.conf.widthM  = cellDiameter*1000.0; // smoothAzm;
		avgOp.conf.heightD = cellDiameter*1000.0; // smoothRad;
		mout.warn("cappi.odim: " , cappi.odim );
		//avgOp.setPixelConf();
		avgOp.processPlainData(cappi, fuzzyCell2);

	}

	Data<PolarDst> & fuzzyEchoTop = dstProduct.getData("FETOP");
	qm.setQuantityDefaults(fuzzyEchoTop, "PROB");
	//@ fuzzyEchoTop.updateTree();  // if (ProductOp::outputDataVerbosity > 0){
	//
	RadarFunctorOp<drain::FuzzyStepsoid<double> > fuzzyEchoTopOp;
	fuzzyEchoTopOp.odimSrc = echoTop.odim;
	fuzzyEchoTopOp.functor.set(echoTopThreshold, abs(echoTopThreshold)/20.0);
	fuzzyEchoTopOp.process(echoTop.data, fuzzyEchoTop.data);

	drain::image::DistanceTransformExponentialOp smoothOp;
	//smoothOp.distanceModel.setRadius(smoothRad * 1000.0 / srcData.odim.rscale, smoothAzm * static_cast<double>(srcData.odim.geometry.width) / 360.0);
	double horz = smoothRad * 1000.0 / dstData.odim.rscale;
	double vert = smoothAzm * static_cast<double>(dstData.odim.area.width) / 360.0;
	smoothOp.setRadius(horz, vert);
	// Consider GaussianPolar
	if ((horz > 0) && (vert > 0.0)){
		//mout.warn("smoothing: " , smoothOp );
		fuzzyCore.data.setCoordinatePolicy(polarCoordPolicy);
		smoothOp.process(fuzzyCore.data, fuzzyCore.data);
		fuzzyCell.data.setCoordinatePolicy(polarCoordPolicy);
		smoothOp.process(fuzzyCell.data, fuzzyCell.data);
		fuzzyEchoTop.data.setCoordinatePolicy(polarCoordPolicy);
		smoothOp.process(fuzzyEchoTop.data, fuzzyEchoTop.data);
	}

	//int address;
	int probability;
	Image::iterator it  = dstData.data.begin();
	Image::const_iterator itM = fuzzyCore.data.begin();
	Image::const_iterator itC = fuzzyCell.data.begin();
	Image::const_iterator itE = fuzzyEchoTop.data.begin();
	while (it != dstData.data.end()){
		probability = *itM;
		++itM;
		if (USE_FCELL){
			probability = (probability*static_cast<int>(*itC))/0xff;
			++itC;
		}
		if (USE_FETOP){
			probability = (probability*static_cast<int>(*itE))/0xff;
			++itE;
		}
		*it = probability;
		++it;
	}


	double gamma = 1.0 + (USE_FCELL?1.0:0.0) + (USE_FETOP?1.0:0.0);

	//drain::image::GammaOp
	//drain::image::UnaryFunctorOpCloner<drain::image::GammaFunctor> gammaOp;
	drain::image::UnaryFunctorOp<drain::image::GammaFunctor> gammaOp;
	gammaOp.functor.gamma = gamma;
	gammaOp.process(dstData.data, dstData.data);

	//drain::image::GammaOp(gamma).process(dstData.data, dstData.data);

	if (!targetEncoding.empty()){  // Larissa
		const double gainOrig = dstData.odim.scaling.scale;
		dstData.odim.setValues(targetEncoding);
		Image::iterator  it = dstData.data.begin();
		Image::iterator mit = maxEcho.data.begin();
		while (it != dstData.data.end()){
			if (*mit == maxEcho.odim.undetect){
				*it = dstData.odim.undetect;
			}
			else {
				*it = dstData.odim.scaling.inv(gainOrig * static_cast<double>(*it));
			}
			++it, ++mit;
		}

	}


	mout.debug("Finished" );

}

}


// Rack
