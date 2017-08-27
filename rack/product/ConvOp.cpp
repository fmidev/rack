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

#include <drain/util/Fuzzy.h>

#include <drain/image/File.h> // debugging

#include <drain/image/BasicOps.h>
#include <drain/image/DistanceTransformOp.h>
#include <drain/image/GammaOp.h>
#include <drain/image/GaussianBlurOp.h>
#include <drain/image/SegmentProber.h>
#include <drain/image/SegmentAreaOp.h>

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
void ConvOp::processDataSets(const DataSetSrcMap & srcSweeps, DataSetDst<PolarDst> & dstProduct) const {


	drain::MonitorSource mout(name, __FUNCTION__);
	mout.debug(1) << "start" << mout.endl;

	const CoordinatePolicy polarCoordPolicy(CoordinatePolicy::POLAR, CoordinatePolicy::WRAP, CoordinatePolicy::LIMIT,CoordinatePolicy::WRAP);

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
	dstData.updateTree();
	//mout.warn() << "Destination data:" << dstData << mout.endl;

	dstQuality.data.setGeometry(g);
	dstQuality.data.fill(dstQuality.odim.scaleInverse(1.0));
	dstQuality.updateTree();

	dstProduct.updateTree(dstData.odim);


	/// Step 2: fuzzification
	Data<PolarDst> & fuzzyCore = dstProduct.getData("FMAX");
	qm.setQuantityDefaults(fuzzyCore, "PROB");
	fuzzyCore.updateTree();
	//
	RadarFunctorOp<drain::FuzzyStepsoid<double> > fuzzyMaxEchoOp;
	fuzzyMaxEchoOp.odimSrc = maxEcho.odim;
	fuzzyMaxEchoOp.functor.set(maxEchoThreshold, abs(maxEchoThreshold)/20.0);
	fuzzyMaxEchoOp.filter(maxEcho.data, fuzzyCore.data);


	Data<PolarDst> & fuzzyCell = dstProduct.getData("FCELL");
	qm.setQuantityDefaults(fuzzyCell, "PROB");
	fuzzyCell.updateTree();  // if (ProductOp::outputDataVerbosity > 0){
	//
	if (USE_FCELL)
		PolarSmoother::filter(dstData.odim, cappi.data, fuzzyCell.data, cellDiameter*1000.0);

	Data<PolarDst> & fuzzyEchoTop = dstProduct.getData("FETOP");
	qm.setQuantityDefaults(fuzzyEchoTop, "PROB");
	fuzzyEchoTop.updateTree();  // if (ProductOp::outputDataVerbosity > 0){
	//
	RadarFunctorOp<drain::FuzzyStepsoid<double> > fuzzyEchoTopOp;
	fuzzyEchoTopOp.odimSrc = echoTop.odim;
	fuzzyEchoTopOp.functor.set(echoTopThreshold, abs(echoTopThreshold)/20.0);
	fuzzyEchoTopOp.filter(echoTop.data, fuzzyEchoTop.data);

	drain::image::DistanceTransformExponentialOp smoothOp;
	//smoothOp.distanceModel.setRadius(smoothRad * 1000.0 / srcData.odim.rscale, smoothAzm * static_cast<double>(srcData.odim.nbins) / 360.0);
	smoothOp.horz = smoothRad * 1000.0 / dstData.odim.rscale;
	smoothOp.vert = smoothAzm * static_cast<double>(dstData.odim.nbins) / 360.0;
	// Consider GaussianPolar
	if ((smoothOp.horz > 0) && (smoothOp.vert > 0)){
		//mout.warn() << "smoothing: " << smoothOp << mout.endl;
		fuzzyCore.data.setCoordinatePolicy(polarCoordPolicy);
		smoothOp.filter(fuzzyCore.data, fuzzyCore.data);
		fuzzyCell.data.setCoordinatePolicy(polarCoordPolicy);
		smoothOp.filter(fuzzyCell.data, fuzzyCell.data);
		fuzzyEchoTop.data.setCoordinatePolicy(polarCoordPolicy);
		smoothOp.filter(fuzzyEchoTop.data, fuzzyEchoTop.data);
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
	/*
	for (int j = 0; j < dstData.odim.nrays; j++){
		for (int i = 0; i<dstData.odim.nbins; i++){
			address = dstData.data.address(i, j);
			probability = (fuzzyCore.data.get<double>(address) * fuzzyCell.data.get<double>(address) * fuzzyEchoTop.data.get<double>(address)) / 0xffff;
			//probability = (fuzzyZCoreImg.at(address) * fuzzyEchoTopImg.at(address)) / 0xff;
			//if (probability > dstData.data.get<unsigned char>(address)){
				dstData.data.put(address, probability);
			//}
		}
	}
	*/


	double gamma = 1.0 + (USE_FCELL?1.0:0.0) + (USE_FETOP?1.0:0.0);

	drain::image::GammaOp(gamma).filter(dstData.data, dstData.data);

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
	//drain::image::GammaOp g;






	/*
	if (mout.isDebug(20)){ //|| true){
		std::stringstream sstr;
		sstr << name << "_final_";
		File::write(dstData.data, sstr.str() + "CB.png");
		File::write(fuzzyZCoreImg, sstr.str() + "fuzzyReflectivity.png");
		//File::write(fuzzyCellAreaImg, sstr.str() + "fuzzyCellArea.png");
		File::write(fuzzyEchoTopImg, sstr.str() + "fuzzyEchoTop.png");
	}
	*/

	mout.debug() << "Finished" << mout.endl;

}

}


// Rack
