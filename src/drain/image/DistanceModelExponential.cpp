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
#include <drain/image/ImageFile.h>
#include "DistanceModelExponential.h"

namespace drain {

namespace image {

float DistanceModelExponential::radius2Dec(float r, float rDefault) const { //  = -1.0

	if (std::isnan(r))
		return rDefault;
	else if (r < 0.0){ // == infty
		return 1.0;    // no decrement, spread to infinity
	}
	else if (r == 0.0){
		return 0.0;  // peak (no dist effect)
	}
	else { //  r > 0.0, the default:
		return pow(0.5, 2.0/r);
	}

}

void DistanceModelExponential::setRadius(float horz, float vert, float horzLeft, float vertUp){ // , bool diag, bool knight){

	// NEW : nominators now 1.0 => 2.0 to match better with linear half-widths
	drain::Logger mout(getImgLog(), __FUNCTION__, getName());

	/// NEW 2024
	if (std::isnan(vert))
		vert = horz;

	if (std::isnan(horzLeft))
		horzLeft = horz;

	if (std::isnan(vertUp))
		vertUp = vert;


	// Store all
	this->horzRadius.set(horz, horzLeft); // forward   = horz;
	this->vertRadius.set(vert, vertUp); // forward  = vert;

	//mout.warn("Radii: " , this->horzRadius , ", " , this->vertRadius ); // ", " << diag << mout.endl;

	float hRight = radius2Dec(horz,     0.5);
	float hLeft  = radius2Dec(horzLeft, hRight);
	float vDown  = radius2Dec(vert,     hRight);
	float vUp    = radius2Dec(vertUp,   vDown);

	setDecrement(hRight, vDown, hLeft, vUp);

	// mout.warn("Decs: " , this->horzDec , ", " , this->vertDec ); // ", " << diag << mout.endl;

}

float DistanceModelExponential::checkDec(float d, float dDefault) const {

	if (isnan(d)){
		return dDefault;
	}
	else if (d <= 0.0){ // Peak, "automatic"
		return 0.0;
	}
	// Note: also 1.0 allowed == "no decay".
	else {
		if (d > 1.0){
			drain::Logger mout(getImgLog(), __FUNCTION__, getName());
			mout.warn("decay (", d , ") greater than 1.0, setting it to 1.0." );
			d = 1.0;
		}
		return d;
	}

}
void DistanceModelExponential::setDecrement(float horz, float vert, float horzLeft, float vertUp){

	//drain::Logger mout(getImgLog(), __FUNCTION__, getName());
	horzDec.forward  = checkDec(horz);
	horzDec.backward = checkDec(horzLeft, horzDec.forward);

	vertDec.forward  = checkDec(vert,   horzDec.forward);
	vertDec.backward = checkDec(vertUp, vertDec.forward);

}


DistanceElement DistanceModelExponential::getElement(short dx, short dy, bool forward) const {

	if (!forward){ // Note: 180 deg, consider rotate 90deg?
		dx = -dx;
		dy = -dy;
	}

	float hLog = static_cast<float>(dx) * log((dx > 0) ? horzDec.forward : horzDec.backward);
	float vLog = static_cast<float>(dy) * log((dy > 0) ? vertDec.forward : vertDec.backward);


	return DistanceElement(dx, dy, exp(-sqrt(hLog*hLog + vLog*vLog)));

}


}
}


// Drain
