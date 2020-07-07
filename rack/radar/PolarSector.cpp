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

	azm1 = std::numeric_limits<double>::min(); // ~0.0
	azm2 = std::numeric_limits<double>::min(); // ~0.0
	range1 = std::numeric_limits<int>::max();
	range2 = std::numeric_limits<int>::min();

	ray1 = std::numeric_limits<int>::max();
	ray2 = std::numeric_limits<int>::min();
	bin1 = std::numeric_limits<int>::max();
	bin2 = std::numeric_limits<int>::min();

}

/// Given (azm1,range1) and (azm2,range2), sets (ray1, bin1) and (ray2, bin2). Returns true if any value was truncated.
/**
 *
 *
 */
void PolarSector::adjustIndices(const PolarODIM & odim){

	if (range2 < range1){
		const double tmp = range1;
		range1 = range2;
		range2 = tmp;
	}

	if (range1 < 0.0)
		range1 = 0.0;

	bin1 = static_cast<int>((range1 - odim.rstart) / odim.rscale*1000.0);
	bin2 = static_cast<int>((range2 - odim.rstart) / odim.rscale*1000.0);

	if (bin1 >= odim.geometry.width)
		bin1 = odim.geometry.width-1;

	if (bin2 > odim.geometry.width)
		bin2 = odim.geometry.width;

	/// todo: re-adjust ranges?

	ray1 = static_cast<int>(azm1/360.0*odim.geometry.height);
	ray2 = static_cast<int>(azm2/360.0*odim.geometry.height);
	ray1 = ray1 %  odim.geometry.height;
	ray2 = ray2 % (odim.geometry.height*2);


}

void PolarSector::deriveWindow(const PolarODIM & srcOdim, int & ray1, int & bin1, int & bin2, int & ray2) const {

	drain::Logger mout("PolarWindow", __FUNCTION__);

	if (this->ray2 != this->ray1 ){
		ray1 = this->ray1;
		ray2 = this->ray2;
	}
	else {
		ray1 = this->azm1*srcOdim.geometry.height/360.0;
		ray2 = this->azm2*srcOdim.geometry.height/360.0;
	}

	ray1 = ray1 % srcOdim.geometry.height;
	ray2 = ray2 % srcOdim.geometry.height;
	if (ray2 < ray1)
		ray2 += srcOdim.geometry.height;

	if (this->bin2 != this->bin1 ){
		bin1 = this->bin1;
		bin2 = this->bin2;
	}
	else {
		bin1 = (this->range1 - srcOdim.rstart) / srcOdim.rscale*1000.0;
		bin2 = (this->range2 - srcOdim.rstart) / srcOdim.rscale*1000.0;
	}

	if (bin1 < 0){
		mout.warn() << "bin1 negative: " << bin1 << "), setting to zero" << mout.endl;
		bin1 = 0;
	}

	if (bin2 > srcOdim.geometry.width){
		mout.warn() << "bin2 too large (" << bin2 << "), setting to nbins"<< mout.endl;
		bin2 = srcOdim.geometry.width;
	}

	if (bin1 > bin2){
		mout.error() << "bin1 (" << bin1 << ") larger than bin2 (" << bin2 << ") "<< mout.endl;
	}

}


}

// Rack
