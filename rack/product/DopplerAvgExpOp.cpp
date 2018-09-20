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

#include <stdexcept>

#include <drain/image/File.h>
//#include <drain/imageops/ImpulseResponseOp.h>

#include "DopplerAvgExpOp.h"

namespace rack {



void DopplerAvgExpOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	drain::Logger mout(name, __FUNCTION__);

	//const int width  = srcData.data.getWidth();
	//const int height = srcData.data.getHeight();
	const QuantityMap & qm = getQuantityMap();

	dstData.data.setScaling(dstData.odim.gain, dstData.odim.offset);  // TODO: re-design, get rid of these

	PlainData<PolarDst> & dstQuality = dstData.getQualityData("QIND");
	qm.setQuantityDefaults(dstQuality);
	dstQuality.setGeometry(srcData.data.getWidth(), srcData.data.getHeight());

	drain::image::ImpulseResponseOp<drain::image::ImpulseAvg> impOp(conf);
	impOp.setExtensions(horzExt, vertExt);

	if (srcData.hasQuality()){

		impOp.traverseChannel(srcData.data, srcData.getQualityData().data, dstData.data, dstQuality.data);

	}
	else {
		mout.note() << "no quality field, creating default quality field" << mout.endl;
		drain::image::Image srcQuality;
		//double udq = DataCoder::undetectQualityCoeff;
		//if (udq > 0.0)
		mout.note() << "undetectQualityCoeff=" << DataCoder::undetectQualityCoeff << mout.endl;

		srcData.createSimpleQualityData(srcQuality, 1.0, 0.0, DataCoder::undetectQualityCoeff);

		impOp.traverseChannel(srcData.data, srcQuality, dstData.data, dstQuality.data);
	}


	//drain::image::CoordinateHandler2D coordHandler(srcData.data.getGeometry(), srcData.data.getCoordinatePolicy());


	/*
	drain::getLog().setVerbosity(10);
	drain::image::getImgLog().setVerbosity(10);


	imgDown.setPhysicalScale(-32, 96);
	qualityDown.setScaling(dstQuality.data.getScaling());
	drain::image::File::write(imgDown, "imgDown.png");

	imgUp.setPhysicalScale(-32, 96);
	qualityUp.setScaling(dstQuality.data.getScaling());
	drain::image::File::write(imgUp, "imgUp.png");
	*/

}



struct unitSpeed {
	double u;
	double v;
	double weight;
};


void DopplerAvgExpOp::processData1D(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	drain::Logger mout(name, __FUNCTION__);

	const double NI = srcData.odim.getNyquist();
	if (NI == 0){
		mout.error() << "NI=0" << mout.endl;
		return;
	}

	dstData.setEncoding(odim.type);
	dstData.setPhysicalRange(-NI, +NI);
	//dstData.initialize(typeid(unsigned short int), srcData.data.getGeometry());
	//dstData.copyEncoding(srcData);
	//dstData.setGeometry(srcData.data.getWidth(), srcData.data.getHeight());
	//dstData.setPhysicalRange(-1, +1);

	PlainData<PolarDst> & dstQuality = dstData.getQualityData();
	QuantityMap & qm = getQuantityMap();
	qm.setQuantityDefaults(dstQuality);
	//dstU1.setPhysicalRange(-1, +1);
	dstQuality.setGeometry(srcData.data.getWidth(), srcData.data.getHeight());


	const int height = srcData.data.getHeight();

	const double coeff     = M_PI/NI;
	const double coeffInv  = NI/M_PI;

	//const double orderD = static_cast<double>(order);

	double vrad, W, weight;

	double decay = conf.decay;

	const double wOld = 0.5; // smoothNess;
	const double wNew = 0.5; // 1.0-smoothNess;

	std::vector<unitSpeed> speedTmp(srcData.data.getHeight()); //(height);

	int j;
	unitSpeed speed;

	for (size_t i=0; i<srcData.data.getWidth(); i++){

		speed.u      = 0.0;
		speed.v      = 0.0;
		speed.weight = 0.0;

		for (int j0=height+10; j0>=0; --j0){ // init

			j = (j0%height);

			vrad = srcData.data.get<double>(i,j);
			if (srcData.odim.isValue(vrad)){
				W = coeff * srcData.odim.scaleForward(vrad);
				weight = speed.weight*wOld + wNew;
				speed.u = (speed.weight*wOld*speed.u + wNew*cos(W)) / weight;
				speed.v = (speed.weight*wOld*speed.v + wNew*sin(W)) / weight;
				speed.weight = 1.0;
			}
			// Store
			speedTmp[j] = speed;
			speed.weight *= decay; // muisto haalistuu
		}

		speed.u      = 0.0;
		speed.v      = 0.0;
		speed.weight = 0.0;

		for (int j0=-10; j0<height; j0++){

			j = (j0+height)%height;

			// Step 1: update accumulation
			vrad = srcData.data.get<double>(i,j);
			if (srcData.odim.isValue(vrad)){
				W = coeff * srcData.odim.scaleForward(vrad);
				weight = speed.weight*wOld + wNew;
				speed.u = (speed.weight*wOld*speed.u + wNew*cos(W)) / weight;
				speed.v = (speed.weight*wOld*speed.v + wNew*sin(W)) / weight;
				speed.weight = 1.0;
			}
			speed.weight *= decay; // muisto haalistuu

			// Step 2: update accumulation
			const unitSpeed & s = speedTmp[j];

			weight = speed.weight + s.weight;
			if (weight > 0.1){

				speed.u = (speed.weight*speed.u + s.weight*s.u) / weight;
				speed.v = (speed.weight*speed.v + s.weight*s.v) / weight;
				if ((speed.u != 0.0) && (speed.v != 0.0)){
					dstData.data.putScaled(i, j, atan2(speed.v, speed.u)*coeffInv);
					dstQuality.data.putScaled(i, j, weight/2.0);
				}
			}
			/*
			else {
				dstData.data.put(i, j, dstData.odim.undetect);
				dstQuality.data.putScaled(i, j, 0.0);
			}
			*/
		}

	}


}


/*
void DopplerModulatorOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	drain::Logger mout(name, __FUNCTION__);

	dstData.initialize(typeid(unsigned char), srcData.data.getGeometry());
	dstData.setPhysicalRange(-1, +1);

	const double NI = srcData.odim.getNyquist();
	if (NI == 0){
		mout.error() << "NI=0" << mout.endl;
		return;
	}

	const double W  = M_PI/NI;

	const double orderD = static_cast<double>(order);
	double sinBeam, cosBeam;

	PlainData<PolarDst> & dstU1 = dstData.getQualityData("UC");
	dstU1.setPhysicalRange(-1, +1);
	PlainData<PolarDst> & dstV1 = dstData.getQualityData("VC");
	dstV1.setPhysicalRange(-1, +1);
	PlainData<PolarDst> & dstU2 = dstData.getQualityData("US");
	dstU2.setPhysicalRange(-1, +1);
	PlainData<PolarDst> & dstV2 = dstData.getQualityData("VS");
	dstV2.setPhysicalRange(-1, +1);

	drain::image::AreaGeometry g(srcData.data.getWidth(), size_t(1));
	dstU1.data.setGeometry(g);
	dstU2.data.setGeometry(g);
	dstV1.data.setGeometry(g);
	dstV2.data.setGeometry(g);

	double u1,u2,v1,v2;

	double vrad, w;

	unsigned int count;

	for (size_t i=0; i<srcData.data.getWidth(); i++){

		count = 0;
		u1=u2=v1=v2 = 0;


		for (size_t j=0; j<srcData.data.getHeight(); j++){

			sinBeam = sin(orderD * srcData.odim.getAzimuth(j));
			cosBeam = cos(orderD * srcData.odim.getAzimuth(j));

			vrad = srcData.data.get<double>(i,j);
			if (srcData.odim.isValue(vrad)){

				vrad = srcData.odim.scaleForward(srcData.data.get<double>(i,j));
				w = vrad*W;

				u1 += cos(w) * cosBeam;
				v1 += cos(w) * sinBeam;
				u2 += sin(w) * cosBeam;
				v2 += sin(w) * sinBeam;

				dstData.data.putScaled(i, j, cos(w) * cosBeam );

				++count;

			}


		}

		dstU1.data.putScaled(i, 0, u1 / static_cast<double>(count) );
		dstV1.data.putScaled(i, 0, v1 / static_cast<double>(count) );
		dstU2.data.putScaled(i, 0, u2 / static_cast<double>(count) );
		dstV2.data.putScaled(i, 0, v2 / static_cast<double>(count) );

	}


}
*/

}  // rack::

// Rack
