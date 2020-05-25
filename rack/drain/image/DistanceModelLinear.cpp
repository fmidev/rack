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
#include "DistanceModelLinear.h"

namespace drain {

namespace image {

float DistanceModelLinear::radius2Dec(float r, float rDefault) const { //  = -1.0

	if (std::isnan(r))
		return rDefault;
	else if (r < 0.0){ // == infty
		return 0.0;    // no decrement, spread to infinity
	}
	else if (r == 0.0){
		return getMax();  // peak (no dist effect)
	}
	else { //  r > 0.0, the default:
		return (getMax()/r + 0.0); // epsilon?
	}
}

void DistanceModelLinear::setRadius(float horz, float vert, float horzLeft, float vertUp){ // , bool diag, bool knight){


	drain::Logger mout(getImgLog(), __FUNCTION__, getName());
	//std::cerr << getName() << ':' <<__FUNCTION__ << " 2" << std::endl;
	//mout.warn() << "calling DM Linear! " << horz << ", " << vert  << mout.endl; // ", " << diag << mout.

	/*
	if (std::isnan(vert))
		vert = horz;

	if (std::isnan(horzLeft))
		horzLeft = horz;

	if (std::isnan(vertUp))
		vertUp = vert;
	 */

	mout.debug(1) << "radii: " << horz << ", " << vert << mout.endl; // ", " << diag << mout.endl;
	this->widths[0]  = horz;
	this->widths[1]  = horzLeft;
	this->heights[0] = vert;
	this->heights[1] = vertUp;
	mout.warn() << this->getParameters() << mout.endl;

	if (getMax() == 0.0){
		mout.warn() << "max unset " << mout.endl; // ", " << diag << mout.endl;
	}

	/*
	float h = getMax();
	float v = getMax();

	if (horz < 0.0)
		h = 0.0;    // no decrement, spread to infinity
	else if (horz > 0.0)
		h = (getMax()/horz  + 0.0); // "default"

	if (std::isnan(vert))
		v = h;
	else if (vert < 0.0)
		v = 0.0;   // no decrement, spread to infinity
	else if (vert > 0.0)
		v = (getMax()/vert + 0.0); // "default"
	*/

	// Decrements
	float hRight = radius2Dec(horz,     1.0); // getMax()
	float hLeft  = radius2Dec(horzLeft, hRight);
	float vDown  = radius2Dec(vert,     hRight);
	float vUp    = radius2Dec(vertUp,   vDown);

	mout.warn() << "Decrements " << hRight << ',' << hLeft << ' ' << vDown << ',' << vUp << mout.endl; // ", " << diag << mout.endl;

	setDecrement(hRight, vDown, hLeft, vUp);  // handles diag and knight

}

float DistanceModelLinear::checkDec(float d, float dDefault) const {

	if (isnan(d)){
		return dDefault;
	}
	else if (d < 0.0){ // Peak, "automatic"
		return getMax();
	}
	// Note: zero allowed == "no decay".
	else {
		return d;
	}

}


void DistanceModelLinear::setDecrement(float horz, float vert, float horzRight, float vertUp){

	drain::Logger mout(getImgLog(), __FUNCTION__, getName());

	// Default decrement: 1.0

	horzDec  = checkDec(horz);
	horzDec2 = checkDec(horzRight, horzDec);

	vertDec  = checkDec(vert,   horzDec);
	vertDec2 = checkDec(vertUp, vertDec);

	mout.warn() << "Decrements " << horzDec << ':' << horzDec2 << ',' << vertDec << ':' << vertDec2 << mout.endl; // ", " << diag << mout.endl;

	/*
	diagDecrement =       sqrt(    horzDecrement*horzDecrement +     vertDecrement*vertDecrement);
	knightDecrementHorz = sqrt(4.0*horzDecrement*horzDecrement +     vertDecrement*vertDecrement);   //diagDecrement * sqrt(5.0)/sqrt(2.0);
	knightDecrementVert = sqrt(    horzDecrement*horzDecrement + 4.0*vertDecrement*vertDecrement);

	mout.debug() << "decs: " << horzDecrement << ", " << vertDecrement;
	if (DIAG){
		mout << ", (" << diagDecrement;
		if (KNIGHT){
			mout << ", (" << knightDecrementHorz << ','  << knightDecrementVert << ") ";
		}
		mout << ") ";
	}
	mout << mout.endl;
	*/
}

DistanceElement DistanceModelLinear::getElement(short dx, short dy, bool forward) const {

	if (!forward){ // Note: 180 deg, consider rotate 90deg?
		dx = -dx;
		dy = -dy;
	}

	float distX = static_cast<float>(dx) * ((dx > 0) ? horzDec : horzDec2);
	float distY = static_cast<float>(dy) * ((dy > 0) ? vertDec : vertDec2);

	return DistanceElement(dx, dy, sqrt(distX*distX + distY*distY));

}



/*
void DistanceModelLinear::createChain(DistanceNeighbourhood & chain, unsigned short topology) const {
	switch (topology) {
	case 2:
		chain.push_back(DistanceElement(-1,-2, this->knightDecrementVert));
		chain.push_back(DistanceElement(+1,-2, this->knightDecrementVert));
		chain.push_back(DistanceElement(-2,-1, this->knightDecrementHorz));
		chain.push_back(DistanceElement(+2,-1, this->knightDecrementHorz));
		// no break
	case 1:
		// 8-adjacency
		chain.push_back(DistanceElement(-1,-1, this->diagDecrement));
		chain.push_back(DistanceElement(+1,-1, this->diagDecrement));
		// no break
	case 0:
		// 4-adjacency
		chain.push_back(DistanceElement(-1, 0, this->horzDecrement));
		chain.push_back(DistanceElement( 0,-1, this->vertDecrement));
		break;
	default:
		break;
	}
}
*/

}
}


// Drain
