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
#include "File.h"
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

	/*
	if (std::isnan(vert))
		vert = horz;

	if (std::isnan(horzLeft))
		horzLeft = horz;

	if (std::isnan(vertUp))
		vertUp = vert;
	*/

	this->widths[0]  = horz;
	this->widths[1]  = horzLeft;
	this->heights[0] = vert;
	this->heights[1] = vertUp;

	mout.debug(1) << "radii: " << horz << ", " << vert << mout.endl; // ", " << diag << mout.endl;

	//std::cerr << getName() << ':' <<__FUNCTION__ << " 3" << std::endl;

	float hRight = radius2Dec(horz,     0.5);
	float hLeft  = radius2Dec(horzLeft, hRight);
	float vDown  = radius2Dec(vert,     hRight);
	float vUp    = radius2Dec(vertUp,   vDown);

	/*
	float h = 0.0;
	float v = 0.0;

	// TODO: interpret handle 0 and -1 better
	if (horz < 0.0)
		h = 1.0;    // no decay, spread to infinity
	else if (horz == 0.0)
		h = 0.0;    // full decay, ~ peak
	else if (horz > 0.0)
		h = pow(0.5, 2.0/horz);  // 0.5^(1/horz)

	if (std::isnan(vert))      // default; copy horz
		v = h;
	else if (vert < 0.0)  // no decay, spread to infinity
		v = 1.0;
	else if (vert == 0.0)
		v = 0.0;   // full decay, ~ peak
	else
		v = pow(0.5, 2.0/vert);  // 0.5^(1/horz)

	 */
	setDecrement(hRight, vDown, hLeft, vUp);

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
			mout.warn() << "decay ("<< d << ") greater than 1.0, setting it to 1.0." << mout.endl;
			d = 1.0;
		}
		return d;
	}

}
void DistanceModelExponential::setDecrement(float horz, float vert, float horzRight, float vertUp){

	//drain::Logger mout(getImgLog(), __FUNCTION__, getName());

	horzDec  = checkDec(horz);
	horzDec2 = checkDec(horzRight, horzDec);

	vertDec  = checkDec(vert,   horzDec);
	vertDec2 = checkDec(vertUp, vertDec);


	/*
	if (horz < 0.0){
		mout.error() << "'horz' less than zero." << mout.endl;
		//throw std::runtime_error();
	}
	else if (horz > 1.0) {
		mout.error() << "'horz' greater than 1.0." << mout.endl;
		// throw std::runtime_error("setDecrement: 'horz' greater than 1.0.");
	}
	else
		horzDec = horz;  // 0.5^(1/horz)  ????


	if (std::isnan(vert)){
		vertDec = horzDec;
	}
	else if (vert < 0.0){
		mout.error() << "'vert' less than zero." << mout.endl;
	}
	else if (vert > 1.0) {
		mout.error() << "'vert' greater than 1.0." << mout.endl;
	}
	else
		vertDec = vert;
	*/

	/*
	const float hLog = log(horzDec);
	const float vLog = log(vertDec);

	diagDecay =       exp(-sqrt(    hLog*hLog +     vLog*vLog));
	knightDecayHorz = exp(-sqrt(4.0*hLog*hLog +     vLog*vLog));
	knightDecayVert = exp(-sqrt(    hLog*hLog + 4.0*vLog*vLog));

	mout.debug() << "decays: " << horzDecay << ", " << vertDecay;
	if (DIAG){
		mout << ", (" << diagDecay;
		if (KNIGHT){
			mout << ", (" << knightDecayHorz << ','  << knightDecayVert << ") ";
		}
		mout << ") ";
	}
	mout << mout.endl;
	*/

}


DistanceElement DistanceModelExponential::getElement(short dx, short dy, bool forward) const {
	float hLog;
	float vLog;
	if (forward){
		hLog = static_cast<float>(dx) * log(horzDec);
		vLog = static_cast<float>(dy) * log(vertDec);
		return DistanceElement(dx, dy, exp(-sqrt(hLog*hLog + vLog*vLog)));
	}
	else {
		hLog = static_cast<float>(dx) * log(horzDec);
		vLog = static_cast<float>(dy) * log(vertDec);
		return DistanceElement(-dx, -dy, exp(-sqrt(hLog*hLog + vLog*vLog)));
	}
}

/*
void DistanceModelExponential::createChain(DistanceNeighbourhood & chain, unsigned short topology) const {
	switch (topology) {
	case 2:
		chain.push_back(DistanceElement(-1,-2, this->knightDecayVert));
		chain.push_back(DistanceElement(+1,-2, this->knightDecayVert));
		chain.push_back(DistanceElement(-2,-1, this->knightDecayHorz));
		chain.push_back(DistanceElement(+2,-1, this->knightDecayHorz));
		// no break
	case 1:
		// 8-adjacency
		chain.push_back(DistanceElement(-1,-1, this->diagDecay));
		chain.push_back(DistanceElement(+1,-1, this->diagDecay));
		// no break
	case 0:
		// 4-adjacency
		chain.push_back(DistanceElement(-1, 0, this->horzDecay));
		chain.push_back(DistanceElement( 0,-1, this->vertDecay));
		break;
	default:
		break;
	}
}
*/

}
}


// Drain
