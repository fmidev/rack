/*

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

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
#include "PolarWindow.h"


namespace rack
{


void GaussianStripeVertPolarWeighted::update() {

	value = 0.0;
	sumW  = 0.0;
	weightSum = 0.0;

	// Consider: {NEAR,LINEAR,FAR}
	if (location.x <= rangeNorm){  // if (RANGE < 0){
		for (int j = this->jMin; j <= this->jMax; j++) {
			locationTmp.setLocation(location.x, location.y + j);
			if (coordinateHandler.validate(locationTmp)){
				w = lookUp[j-jMin];
				sumW += w;
				w *= srcWeight.get<double>(locationTmp);
				weightSum += w;
				value += w * src.get<double>(locationTmp);
			}
			//	value += (lookUp[j-jMin] * src.get<double>(locationTmp));
		}
		//value = value/weightSum;
	}
	else if (location.x < rangeNormEnd){ //  if (RANGE == 0){  // "normalized" ie. spread j = (rangeNorm*j)/location.x
		for (int j = this->jMin; j <= this->jMax; j++) {
			locationTmp.setLocation(location.x, location.y + (rangeNorm*j)/(location.x+1));
			if (coordinateHandler.validate(locationTmp)){
				w = lookUp[j-jMin];
				sumW += w;
				w *= srcWeight.get<double>(locationTmp);
				weightSum += w;
				value += w * src.get<double>(locationTmp);
			}
		}
		//value = value/weightSum;
	}
	else {
		value = src.get<double>(location);
		weightSum = srcWeight.get<double>(location);
		return;
	}

	if (weightSum > 0.0){
		value = value/weightSum;
		weightSum = weightSum/sumW;
	}

}




}
