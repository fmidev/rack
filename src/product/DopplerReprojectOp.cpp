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

#include <map>
#include <string>

#include <drain/Log.h>
#include <drain/TypeUtils.h>
#include "data/Data.h"
#include "data/PolarODIM.h"
#include "data/QuantityMap.h"
//#include <drain/image/SegmentProber.h> // for crawler
#include <drain/image/Image.h>
#include <drain/image/ImageChannel.h>
//#include <drain/image/ImageLike.h>
//#include <drain/util/Point.h>
//#include <drain/image/SegmentProber.h>
//#include "product/DopplerOp.h"
//#include <syslog.h>
//#include <drain/util/SmartMap.h>
//#include <product/RadarProductOp.h>

#include "product/DopplerReprojectOp.h"

namespace rack {



void DopplerReprojectOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	//const Data<PolarSrc> & srcData = srcSweep.getFirstData(); // VRAD or VRADH

	mout.attention("Start");

	const PlainData<PolarSrc> & srcDataU = srcSweep.getData("AMVU");
	if (srcDataU.data.isEmpty()){
		mout.warn("AMVU input missing" );
		return;
	}

	const PlainData<PolarSrc> & srcDataV = srcSweep.getData("AMVV");
	if (srcDataV.data.isEmpty()){
		mout.warn("AMVV input missing" );
		return;
	}


	const PlainData<PolarSrc> & srcDataVRAD = srcSweep.getData("VRAD"); // maybe empty
	const bool VRAD_SRC = !srcDataVRAD.data.isEmpty();
	if ((matchOriginal > 0) && !VRAD_SRC){
		mout.warn("No VRAD input, matching will be skipped" );
	}
	const bool MATCH_ALIASED  = (matchOriginal & 1) && VRAD_SRC;
	const bool MASK_DATA = (matchOriginal & 2) && VRAD_SRC;

	//const PlainData<PolarSrc> & dstDataV = srcSweep.getData("AMVV");

	const bool VRAD_OVERWRITE = (dstProduct.find("VRAD") != dstProduct.end());

	if (VRAD_OVERWRITE){
		mout.warn("Contains already VRAD, overwriting" );
		//return;
	}


	PlainData<PolarDst> & dstData = dstProduct.getData(odim.quantity);



	const QuantityMap & qm = getQuantityMap();
	if (dstData.data.isEmpty()){ // or !VRAD_OVERWRITE
		ProductBase::applyODIM(dstData.odim, odim, true);
		//qm.setQuantityDefaults(dstData, "VRAD", odim.type);
		dstData.setEncoding(typeid(unsigned short));
		setGeometry(srcDataU.odim, dstData);
		dstData.odim.NI = odim.NI;
		dstData.odim.setRange(-odim.NI, +odim.NI);
		dstData.data.setScaling(dstData.odim.scaling);
		///dstData.data.setType(dstData.odim.type);
		// dstData.setEncoding(typeid(unsigned short));
	}
	const double minCode = dstData.data.getConf().getTypeMin<double>();
	const double maxCode = dstData.data.getConf().getTypeMax<double>();

	PlainData<PolarDst> & dstQuality = dstProduct.getQualityData();
	qm.setQuantityDefaults(dstQuality);
	setGeometry(dstData.odim, dstQuality);

	//const double dstNI = abs(odim.NI);
	mout.info("Inverting (u,v) back to VRAD " );
	mout.info("srcU: " , srcDataU.data );
	mout.info("srcV: " , srcDataV.data );
	mout.info("dstD: " , dstData.data  );
	mout.info("dstQ: " , dstQuality.data );
	/*
	mout.info("src [" , srcDataU.odim.quantity , "] " , EncodingODIM(srcDataU.odim) );
	mout.info("src [" , srcDataV.odim.quantity , "] " , EncodingODIM(srcDataV.odim) );
	mout.info("dst [" , dstData.odim.quantity , "]  " , EncodingODIM(dstData.odim) , ", [" , minCode , ',' , maxCode , ']' );
	mout.info("dst [" , dstQuality.odim.quantity , "]" , EncodingODIM(dstQuality.odim) );
	*/
	// UNUSED const double srcNI2 = 2.0*srcDataVRAD.odim.getNyquist(); // 2.0*srcData.odim.NI;

	/// Azimuth in radians
	double azmR;

	/// Original value in VRAD
	double vOrig = 0.0;
	drain::Point2D<double> unitVOrig;

	/// Resolved (u,v), from AMVU and AMVV
	double u, v, quality=1.0;

	/// Resolved (u,v) projected back on beam
	double vReproj;
	drain::Point2D<double> unitVReproj;


	bool ORIG_UNDETECT;
	bool ORIG_NODATA;
	bool ORIG_USABLE = true; // ORIG_UNDETECT && ORIG_NODATA

	size_t address;
	// const size_t addressMax = dstData.data.getArea();

	mout.note(DRAIN_LOG(srcDataU.data));
	mout.note(DRAIN_LOG(srcDataV.data));
	mout.note(DRAIN_LOG(dstData.data));
	mout.note(DRAIN_LOG(dstQuality.data));

	const size_t size_debug = dstData.data.getArea();

	for (size_t j = 0; j < dstData.data.getHeight(); ++j) {

		azmR = dstData.odim.getBeamWidth() * static_cast<double>(j) ; // window.BEAM2RAD * static_cast<double>(j);

		for (size_t i = 0; i < dstData.data.getWidth(); ++i) {

			address = dstData.data.address(i,j);

			/*
			if (address > size_debug){
				mout.error("coord overflow: ", address, " > ", size_debug, " geom:", dstData.data.getGeometry());
			}
			*/

			u = srcDataU.data.get<double>(address);
			v = srcDataV.data.get<double>(address);

			if (MASK_DATA || MATCH_ALIASED){
				vOrig = srcDataVRAD.data.get<double>(address);
				ORIG_UNDETECT = (vOrig == srcDataVRAD.odim.undetect);
				ORIG_NODATA   = (vOrig == srcDataVRAD.odim.nodata);

				ORIG_USABLE = !(ORIG_UNDETECT || ORIG_NODATA);
				if (MASK_DATA && !ORIG_USABLE){
					if (ORIG_UNDETECT)
						dstData.data.put(address, dstData.odim.undetect);
					else
						dstData.data.put(address, dstData.odim.nodata);
					dstQuality.data.put(address, 0);
					continue;
				}

			}


			if (srcDataU.odim.isValue(u) && srcDataV.odim.isValue(v)){

				u = srcDataU.odim.scaleForward(u);
				v = srcDataV.odim.scaleForward(v);
				vReproj = this->project(azmR, u,v);

				if (VRAD_SRC){
					vOrig = srcDataVRAD.odim.scaleForward(vOrig);
					srcDataVRAD.odim.mapDopplerSpeed(vOrig,     unitVOrig.x,   unitVOrig.y);
					srcDataVRAD.odim.mapDopplerSpeed(vReproj, unitVReproj.x, unitVReproj.y);
				}

				if (MATCH_ALIASED && ORIG_USABLE){
					//vReproj = srcNI2*(floor(vReproj/srcNI2+0.5)-0.5) + vOrig;
					vReproj = vOrig; //srcNI2*floor(vReproj/srcNI2) + vOrig;
				}

				vReproj = dstData.odim.scaleInverse(vReproj);


				if ((vReproj > minCode) && (vReproj < maxCode)){ // continue processing

					//?
					dstData.data.put(address, vReproj);

					quality = 0.5 + 0.5*(unitVReproj.x*unitVOrig.x + unitVReproj.y*unitVOrig.y);
					dstQuality.data.put(address, dstQuality.odim.scaleInverse(quality) );
					// if (((i+50)==j) && ((i&7)==0)){
					//	std::cerr << i << '\t' << unitVOrig << '\t' << unitVReproj << '\t' << quality << '\n';
					// }
				}
				else {
					//?
					dstData.data.put(address, dstData.odim.undetect); // rand() & 0xffff); //
					dstQuality.data.put(address, 0);
				};

				/*
				if (((i+50)==j) && ((i&7)==0)){
					// std::cerr << i << '\t' << unitVOrig << '\t' << unitVReproj << '\t' << quality << '\n';
					mout.warn(i, '\t', unitVOrig, '\t', unitVReproj, '\t', quality);
				}
				*/

			}
			else {
				//?
				dstData.data.put(address, dstData.odim.undetect);
			}

		}
	}

	//@ dstDataVRAD.updateTree();
	// mout.experimental("FINISHED");
	// exit(13);

}


}  // rack::

// Rack
