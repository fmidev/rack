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

	if (bin1 >= odim.nbins)
		bin1 = odim.nbins-1;

	if (bin2 > odim.nbins)
		bin2 = odim.nbins;

	/// todo: re-adjust ranges?

	ray1 = static_cast<int>(azm1/360.0*odim.nrays);
	ray2 = static_cast<int>(azm2/360.0*odim.nrays);
	ray1 = ray1 %  odim.nrays;
	ray2 = ray2 % (odim.nrays*2);


}

void PolarSector::deriveWindow(const PolarODIM & srcOdim, int & ray1, int & bin1, int & bin2, int & ray2) const {

	drain::Logger mout("PolarWindow", __FUNCTION__);

	if (this->ray2 != this->ray1 ){
		ray1 = this->ray1;
		ray2 = this->ray2;
	}
	else {
		ray1 = this->azm1*srcOdim.nrays/360.0;
		ray2 = this->azm2*srcOdim.nrays/360.0;
	}

	ray1 = ray1 % srcOdim.nrays;
	ray2 = ray2 % srcOdim.nrays;
	if (ray2 < ray1)
		ray2 += srcOdim.nrays;

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

	if (bin2 > srcOdim.nbins){
		mout.warn() << "bin2 too large (" << bin2 << "), setting to nbins"<< mout.endl;
		bin2 = srcOdim.nbins;
	}

	if (bin1 > bin2){
		mout.error() << "bin1 (" << bin1 << ") larger than bin2 (" << bin2 << ") "<< mout.endl;
	}

}


}
