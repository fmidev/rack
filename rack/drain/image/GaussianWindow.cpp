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

#include "GaussianWindow.h"

namespace drain
{

namespace image
{

void GaussianStripe::initialize(){

		drain::MonitorSource mout(iMonitor, "GaussianStripe", __FUNCTION__);

		const int n = conf.width*conf.height; // width*1 or 1*height
		int bias = std::min(iMin, jMin);
		lookUp.resize(n);
		weightSum = 0.0;
		double f;
		const double radius2 = conf.radius * conf.radius;
		if (radius2 <= 0.0){
			mout.error() << "Zero radius2: " << radius2 << mout.endl;
		}
		int iNorm;
		mout.debug() << conf.width << 'x' << conf.height << mout.endl;
		for (int i = 0; i < n; ++i) {
			iNorm = (i + bias);
			f = exp2(-static_cast<double>(iNorm*iNorm) / radius2);
			lookUp[i] = f;
			weightSum += f;
			mout.debug() << i << '\t' << iNorm << '\t' << f << '\t' << weightSum << mout.endl;
		}

		scaleResult = this->src.getScale()/this->dst.getScale();
		mout.debug() << "scale = " << scaleResult << mout.endl;

		coordinateHandler.setLimits(srcWidth, srcHeight);
}



void GaussianStripeHorz::update() {

	value = 0.0;
	for (int i = this->iMin; i <= this->iMax; i++) {
		locationTmp.setLocation(location.x+i, location.y);
		if (coordinateHandler.validate(locationTmp))
			value += lookUp[i-iMin] * src.get<double>(locationTmp);
	}
	value = scaleResult * value / weightSum;

}



void GaussianStripeVert::update() {

	value = 0.0;
	for (int j = this->jMin; j <= this->jMax; j++) {
		locationTmp.setLocation(location.x, location.y+j);
		if (coordinateHandler.validate(locationTmp))
			value += (lookUp[j-jMin] * src.get<double>(locationTmp));
	}
	value = scaleResult * value/weightSum;

}


void GaussianStripeHorzWeighted::update() {

	value = 0.0;
	sumW  = 0.0;
	weightSum = 0.0;
	for (int i = this->iMin; i <= this->iMax; i++) {
		locationTmp.setLocation(location.x+i, location.y);
		if (coordinateHandler.validate(locationTmp)){
			w = lookUp[i-iMin];
			sumW += w;
			w *= srcWeight.get<double>(locationTmp);
			weightSum += w;
			value += w * src.get<double>(locationTmp);
		}
	}

	if (weightSum > 0.0){
		value = scaleResult * value/weightSum;
		weightSum = scaleResult * weightSum/sumW;
	}

}

void GaussianStripeVertWeighted::update() {

	value = 0.0;
	sumW  = 0.0;
	weightSum = 0.0;
	for (int j = this->jMin; j <= this->jMax; j++) {
		locationTmp.setLocation(location.x, location.y+j);
		if (coordinateHandler.validate(locationTmp)){
			w = lookUp[j-jMin];
			sumW += w;
			w *= srcWeight.get<double>(locationTmp);
			weightSum += w;
			value += w * src.get<double>(locationTmp);
		}
	}

	if (weightSum > 0.0){
		value = scaleResult * value/weightSum;
		weightSum = scaleResult * weightSum/sumW;
	}

}




}  // image::

}  // drain::

// Drain
