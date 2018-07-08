/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

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
