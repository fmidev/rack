/*

    Copyright 2012-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack library for C++.

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
//#include <math.h>

#include <drain/util/Fuzzy.h>
#include <drain/util/FunctorBank.h>

#include <drain/image/File.h> // debugging
//#include <drain/image/SegmentProber.h>
#include <drain/imageops/DistanceTransformOp.h>
#include <drain/imageops/GammaOp.h>
#include <drain/imageops/GaussianAverageOp.h>
#include <drain/imageops/SegmentAreaOp.h>

#include "radar/Constants.h"
#include "radar/Analysis.h"
#include "radar/PolarWindow.h"
#include "radar/PolarSmoother.h"


#include "CappiOp.h"
#include "EchoTopOp.h"
#include "MaxEchoOp.h"

#include "ConvOp.h"



namespace rack
{

using namespace drain::image;


// void ConvOp::filter(const HI5TREE &src, const std::map<double,std::string> & srcPaths, HI5TREE &dst) const {
void ConvOp::processDataSets(const DataSetMap<PolarSrc> & srcSweeps, DataSet<PolarDst> & dstProduct) const {


	drain::Logger mout(name, __FUNCTION__);
	mout.debug(1) << "start" << mout.endl;

	//const CoordinatePolicy polarCoordPolicy(CoordinatePolicy::POLAR, CoordinatePolicy::WRAP, CoordinatePolicy::LIMIT,CoordinatePolicy::WRAP);

	/// Use always maxEcho (consider changing to cappi?)
	/// cell and echo top are optional, but used by default.
	const bool USE_FCELL = (cellDiameter>0.0);
	const bool USE_FETOP = (echoTopThreshold>0.0);


	const QuantityMap & qm = getQuantityMap();

	//mout.warn() << quantityMap.get("DBZH").get('C').toStr() << mout.endl;
	MaxEchoOp maxOp;
	maxOp.odim.updateFromMap(qm.get("DBZH").get('C'));
	maxOp.odim.quantity = "MAX";
	maxOp.altitude    = 1000; //echoTopThreshold/2;
	maxOp.devAltitude = 500; //echoTopThreshold/2;
	maxOp.processDataSets(srcSweeps, dstProduct);
	Data<PolarDst> & maxEcho = dstProduct.getData(maxOp.odim.quantity); // ensure
	//mout.warn() << "maxEcho " << maxEcho.odim << mout.endl;

	CappiOp cappiOp;
	if (USE_FCELL){
		//cappiOp.setEncodingRequest(quantityMap.get("DBZH").get('C').getValues());
		cappiOp.odim.updateFromMap(qm.get("DBZH").get('C'));
		cappiOp.odim.quantity = "CAPPI";
		cappiOp.altitude = 2000;
		cappiOp.processDataSets(srcSweeps, dstProduct);
	}
	Data<PolarDst> & cappi = dstProduct.getData(cappiOp.odim.quantity);

	EchoTopOp echoTopOp;
	if (USE_FETOP){
		echoTopOp.odim.updateFromMap(qm.get("HGHT").get('C'));
		//echoTopOp.setEncodingRequest(quantityMap.get("HGHT").get('C').getValues());
		echoTopOp.odim.quantity = "ETOP";
		echoTopOp.minDBZ = echoTopDBZ; // maxEchoThreshold; //echoTopDBZ;
		echoTopOp.processDataSets(srcSweeps, dstProduct);
	}
	Data<PolarDst> & echoTop = dstProduct.getData(echoTopOp.odim.quantity);



	// Product
	Data<PolarDst> & dstData = dstProduct.getData(odim.quantity); // main data
	qm.setQuantityDefaults(dstData, odim.quantity);
	// See code at the end
	/*
	if (!encodingRequest.empty()){
		dstData.odim.setValues(encodingRequest);
		//dstData.odim.updateFromMap(odim);
	}
	*/
	deriveDstGeometry(srcSweeps, dstData.odim);
	mout.note() << ODIM(dstData.odim) << mout.endl;
	//mout.note() << PolarODIM(dstData.odim) << mout.endl;

	// Product quality (?)
	PlainData<PolarDst> & dstQuality = dstData.getQualityData();
	qm.setQuantityDefaults(dstQuality, "QIND");

	const drain::image::Geometry g(dstData.odim.nbins, dstData.odim.nrays);
	dstData.data.setGeometry(g);
	dstData.data.clear();
	//@ dstData.updateTree();
	//mout.warn() << "Destination data:" << dstData << mout.endl;

	dstQuality.data.setGeometry(g);
	dstQuality.data.fill(dstQuality.odim.scaleInverse(1.0));
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
	if (USE_FCELL)
		PolarSmoother::filter(dstData.odim, cappi.data, fuzzyCell.data, cellDiameter*1000.0);

	Data<PolarDst> & fuzzyEchoTop = dstProduct.getData("FETOP");
	qm.setQuantityDefaults(fuzzyEchoTop, "PROB");
	//@ fuzzyEchoTop.updateTree();  // if (ProductOp::outputDataVerbosity > 0){
	//
	RadarFunctorOp<drain::FuzzyStepsoid<double> > fuzzyEchoTopOp;
	fuzzyEchoTopOp.odimSrc = echoTop.odim;
	fuzzyEchoTopOp.functor.set(echoTopThreshold, abs(echoTopThreshold)/20.0);
	fuzzyEchoTopOp.process(echoTop.data, fuzzyEchoTop.data);

	drain::image::DistanceTransformExponentialOp smoothOp;
	//smoothOp.distanceModel.setRadius(smoothRad * 1000.0 / srcData.odim.rscale, smoothAzm * static_cast<double>(srcData.odim.nbins) / 360.0);
	double horz = smoothRad * 1000.0 / dstData.odim.rscale;
	double vert = smoothAzm * static_cast<double>(dstData.odim.nbins) / 360.0;
	smoothOp.setRadius(horz, vert);
	// Consider GaussianPolar
	if ((horz > 0) && (vert > 0.0)){
		//mout.warn() << "smoothing: " << smoothOp << mout.endl;
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

	if (!encodingRequest.empty()){  // Larissa
		const double gainOrig = dstData.odim.gain;
		dstData.odim.setValues(encodingRequest);
		Image::iterator  it = dstData.data.begin();
		Image::iterator mit = maxEcho.data.begin();
		while (it != dstData.data.end()){
			if (*mit == maxEcho.odim.undetect){
				*it = dstData.odim.undetect;
			}
			else {
				*it = dstData.odim.scaleInverse(gainOrig * static_cast<double>(*it));
			}
			++it, ++mit;
		}

	}


	mout.debug() << "Finished" << mout.endl;

}

}

