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

#include "DopplerAvgExpOp.h"

namespace rack {



void DopplerAvgExpOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	const int width  = srcData.data.getWidth();
	const int height = srcData.data.getHeight();
	const QuantityMap & qm = getQuantityMap();

	dstData.data.setScaling(dstData.odim.gain, dstData.odim.offset);  // TODO: re-design, get rid of these

	PlainData<PolarDst> & dstQuality = dstData.getQualityData("QIND");
	qm.setQuantityDefaults(dstQuality);
	dstQuality.setGeometry(srcData.data.getWidth(), srcData.data.getHeight());

	/*
	PlainData<PolarDst> & dstAux = dstData.getQualityData("AUX");
	qm.setQuantityDefaults(dstAux, odim.quantity);
	dstAux.data.setScaling(dstAux.odim.gain, dstAux.odim.offset);
	*/



	drain::image::Image imgDown(typeid(float), width, height, 1, 1);
	drain::image::Channel & qualityDown = imgDown.getAlphaChannel();

	drain::image::Image imgUp(typeid(float), width, height, 1, 1);
	drain::image::Channel & qualityUp = imgUp.getAlphaChannel();

	drain::image::CoordinateHandler2D coordHandler(srcData.data.getGeometry(), srcData.data.getCoordinatePolicy());

	// Temp value
	double x;

	// Temp weight
	double w;

	int count;

	drain::image::Point2D<int> point;
	double value, weight;

	for (int j=0; j<height; j++){

		for (int i=0; i<width; i++){

			count = 0;
			weight = 0.0;
			value = 0.0; // ? dstData.odim.undetect;

			x = srcData.data.get<double>(i,j);
			if (srcData.odim.isValue(x)){
				value  += srcData.odim.scaleForward(x);
				weight += 1.0; // or: quality?
				++count;
			}

			/// DST prev values
			point.setLocation(i-1, j);
			if (coordHandler.validate(point)){
				w = qualityDown.get<double>(point);
				if (w > 0.0){
					w *= decay;
					value  += w*imgDown.get<double>(point);
					weight += w;
					++count;
				}
			}

			point.setLocation(i, j-1);
			if (coordHandler.validate(point)){
				w = qualityDown.get<double>(point);
				if (w > 0.0){
					w *= decay;
					value  += w*imgDown.get<double>(point);
					weight += w;
					++count;
				}
			}


			if (weight > 0.0){
				//dstData.data.put(i,j, dstData.odim.scaleInverse(value/weight));
				imgDown.put<float>(i,j, value/weight);
				qualityDown.put<float>(i,j, weight/static_cast<float>(count));
				// I.e.
				// dstData.data.putScaled(i,j, value/weight);
				// dstQuality.data.putScaled(i, j, weight/static_cast<float>(count));
			}
			else {
				dstData.data.put(i,j, dstData.odim.undetect);
				dstQuality.data.putScaled(i, j, 0.0);
			}


		}

	}



	for (int j=height-1; j>=0; --j){

		for (int i=width-1; i>=0; --i){

			count  = 0;
			weight = 0.0;
			value  = 0.0;

			x = srcData.data.get<double>(i,j);
			if (srcData.odim.isValue(x)){
				value  += srcData.odim.scaleForward(x);
				weight += 1.0; // or: quality?
				++count;
			}

			/// DST prev values
			point.setLocation(i+1, j);
			if (coordHandler.validate(point)){
				w = qualityUp.get<double>(point);
				if (w > 0.0){
					w *= decay;
					value  += w*imgUp.get<double>(point);
					weight += w;
					++count;
				}
			}

			point.setLocation(i, j+1);
			if (coordHandler.validate(point)){
				w = qualityUp.get<double>(point);
				if (w > 0.0){
					w *= decay;
					value  += w*imgUp.get<double>(point);
					weight += w;
					++count;
				}
			}


			if (weight > 0.0){
				imgUp.put<float>(i,j, value/weight);
				qualityUp.put<float>(i,j, weight/static_cast<float>(count));
			}

			/*
			if (weight > 0.0){
				dstData.data.putScaled(i,j, value/weight);
				dstQuality.data.putScaled(i, j, weight/static_cast<float>(count));
			}
			else {
				dstData.data.put(i,j, dstData.odim.undetect);
				dstQuality.data.putScaled(i, j, 0.0);
			}
			*/

		}

	}

	float w1, w2;
	for (int j=0; j<height; j++){
		for (int i=0; i<width; i++){
			w1 = qualityDown.get<float>(i,j);
			w2 = qualityUp.get<float>(i,j);
			weight = (w1+w2);
			if (weight > 0.0){
				dstData.data.putScaled(i,j, (w1*imgDown.get<float>(i,j)+w2*imgUp.get<float>(i,j)) /weight);
				dstQuality.data.putScaled(i, j, weight/2.0f);
			}
			else {
				dstData.data.put(i,j, dstData.odim.undetect);
				dstQuality.data.putScaled(i, j, 0.0);
			}
		}
	}

	drain::getLog().setVerbosity(10);
	drain::image::getImgLog().setVerbosity(10);

	//imgDowimgDown.n.adoptScaling(dstData.data);
	//imgDown.adoptScaling(dstData.data);
	imgDown.setPhysicalScale(-32, 96);
	qualityDown.setScaling(dstQuality.data.getScaling());
	//imgDown.getScaling().setPhysicalRange(dstData.data.requestPhysicalMin(), dstData.data.requestPhysicalMax());
	//qualityDown.getScaling().setPhysicalRange(0.0, 1.0);
	drain::image::File::write(imgDown, "imgDown.png");

	//imgUp.adoptScaling(dstData.data);
	imgUp.setPhysicalScale(-32, 96);
	qualityUp.setScaling(dstQuality.data.getScaling());
	//qualityUp.getScaling().setPhysicalRange(0.0, 1.0);
	drain::image::File::write(imgUp, "imgUp.png");
	//imgUp.adoptScaling()


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

	const double wOld = smoothNess;
	const double wNew = 1.0-smoothNess;

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
