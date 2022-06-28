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
#include "PolarSector.h"


namespace rack
{

void PolarSector::reset(){

	// natural coords
	azmRange.set(std::numeric_limits<double>::min(), std::numeric_limits<double>::min()); // ~0
	distanceRange.set(std::numeric_limits<int>::max(), std::numeric_limits<int>::min());

	// index coords
	rayRange.set(std::numeric_limits<int>::max(), std::numeric_limits<int>::min());
	binRange.set(std::numeric_limits<int>::max(), std::numeric_limits<int>::min());

}

/// Given (azm1,range1) and (azm2,range2), sets (ray1, bin1) and (ray2, bin2).
/**
 *
 *  ? Returns true if any value was truncated.
 */
void PolarSector::adjustIndices(const PolarODIM & odim){

	if (distanceRange.max < distanceRange.min){
		int tmp = distanceRange.min;
		distanceRange.min = distanceRange.max;
		distanceRange.max = tmp;
	}
	// if (distanceRange.min < 0.0)
	// warn()
	distanceRange.min = std::max(0.0, distanceRange.min);

	const int min = odim.getBinIndex(1000.0 * static_cast<double>(distanceRange.min));
	binRange.min = std::max(0, min);

	const int width = odim.area.width;

	if (binRange.min > width-1)
		binRange.min = width-1;

	int max = odim.getBinIndex(1000.0 * static_cast<double>(distanceRange.max));
	binRange.max = std::max(0, max);
	if (binRange.max > width)
		binRange.max = width;

	/*
	int min = odim.getBinIndex(1000.0 * static_cast<double>(distanceRange.min));
	min = std::max(0, min);

	if (min <= odim.area.width-1)
		binRange.min = min;
	else
		binRange.min = odim.area.width-1;

	int max = odim.getBinIndex(1000.0 * static_cast<double>(distanceRange.max));
	max = std::max(0, max);
	if (max < odim.area.width)
		binRange.max = max;
	else
		binRange.max = odim.area.width;
	*/

	binRange.set(min, max);

	/// todo: re-adjust ranges?

	rayRange.min = odim.getDRayIndex(azmRange.min); // static_cast<int>(azm.first/360.0*odim.geometry.height);
	rayRange.max = odim.getDRayIndex(azmRange.max); // static_cast<int>(azm.second/360.0*odim.geometry.height);
	rayRange.min = rayRange.min %  odim.area.height;
	rayRange.max = rayRange.max % (odim.area.height*2);


}

void PolarSector::deriveWindow(const PolarODIM & srcOdim, int & ray1, int & bin1, int & bin2, int & ray2) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (this->rayRange.max != this->rayRange.min ){
		ray1 = this->rayRange.min;
		ray2 = this->rayRange.max;
	}
	else {
		ray1 = this->azmRange.min*srcOdim.area.height/360.0;
		ray2 = this->azmRange.max*srcOdim.area.height/360.0;
	}

	ray1 = rayRange.min % srcOdim.area.height;
	ray2 = rayRange.max % srcOdim.area.height;
	if (ray2 < ray1)
		ray2 += srcOdim.area.height;

	if (this->binRange.max != this->binRange.min ){
		bin1 = this->binRange.min;
		bin2 = this->binRange.max;
	}
	else {
		bin1 = (this->distanceRange.min - srcOdim.rstart) / srcOdim.rscale*1000.0;
		bin2 = (this->distanceRange.max - srcOdim.rstart) / srcOdim.rscale*1000.0;
	}

	if (bin1 < 0){
		mout.warn() << "bin1 negative: " << bin1 << "), setting to zero" << mout.endl;
		bin1 = 0;
	}

	if (bin2 > srcOdim.area.width){
		mout.warn() << "bin2 too large (" << bin2 << "), setting to nbins"<< mout.endl;
		bin2 = srcOdim.area.width;
	}

	if (bin1 > bin2){
		mout.error() << "bin1 (" << bin1 << ") larger than bin2 (" << bin2 << ") "<< mout.endl;
	}

}


}

// Rack
