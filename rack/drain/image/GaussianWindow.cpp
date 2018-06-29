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

template <> // HORIZONTAL
void GaussianStripe<true,WindowCore>::update() {

	value = 0.0;
	weightSum = 0.0;
	for (int i = this->iMin; i <= this->iMax; i++) {
		locationTmp.setLocation(location.x+i, location.y);
		if (coordinateHandler.validate(locationTmp)){
			w = lookUp[i-iMin];
			//value += lookUp[i-iMin] * src.get<double>(locationTmp);
			value     += w * src.get<value_t>(locationTmp);
			weightSum += w;
		}
	}
	//if (this->weightSum > 0.0) ?
	value = scaleResult * value / weightSum;

}


template <> // VERTICAL
void GaussianStripe<false,WindowCore>::update() {

	value = 0.0;
	weightSum = 0.0;
	for (int j = this->jMin; j <= this->jMax; j++) {
		locationTmp.setLocation(location.x, location.y+j);
		if (coordinateHandler.validate(locationTmp)){
			w = lookUp[j-jMin];
			//value += (lookUp[j-jMin] * src.get<value_t>(locationTmp));
			value     += w * src.get<value_t>(locationTmp);
			weightSum += w;
		}
	}

	//if (this->weightSum > 0.0)
	value = scaleResult * value/weightSum;

}



// Note base class (not Weighted)

template <> // HORIZONTAL
void GaussianStripe<true, WeightedWindowCore>::update() {

	this->value = 0.0;
	sumW  = 0.0;
	this->weightSum = 0.0;
	for (int i = this->iMin; i <= this->iMax; i++) {
		this->locationTmp.setLocation(this->location.x+i, this->location.y);
		if (this->coordinateHandler.validate(this->locationTmp)){
			w = this->lookUp[i-iMin];
			sumW += w;
			w *= this->srcWeight.get<double>(this->locationTmp);
			this->weightSum += w;
			this->value += w * this->src.get<double>(this->locationTmp);
		}
	}

	if (this->weightSum > 0.0){
		this->value = this->scaleResult * this->value/this->weightSum;
		this->weightSum = this->scaleResult * this->weightSum/sumW;
	}

}

// Note base class (not Weighted)
template <> // VERTICAL
void GaussianStripe<false, WeightedWindowCore>::update() {

	this->value = 0.0;
	sumW  = 0.0;
	this->weightSum = 0.0;
	for (int j = this->jMin; j <= this->jMax; j++) {
		this->locationTmp.setLocation(this->location.x, this->location.y+j);
		if (this->coordinateHandler.validate(this->locationTmp)){
			w = this->lookUp[j-this->jMin];
			sumW += w;
			w *= this->srcWeight.get<double>(this->locationTmp);
			this->weightSum += w;
			this->value += w * this->src.get<double>(this->locationTmp);
		}
	}

	if (this->weightSum > 0.0){
		this->value     = this->scaleResult * this->value/this->weightSum;
		this->weightSum = this->scaleResult * this->weightSum/sumW;
	}

}


}  // image::

}  // drain::

// Drain
