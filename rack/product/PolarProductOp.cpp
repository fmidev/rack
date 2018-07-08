/*

    Copyright 2012-2014  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

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
#include <stdexcept>

#include <drain/util/Variable.h>
//#include <drain/util/Fuzzy.h>
//#include <drain/image/AccumulationMethods.h>
//#include "RackOp.h"

//#include "data/Conversion.h"

//#include "radar/Extractor.h"
#include "PolarProductOp.h"



namespace rack {

using namespace drain::image;

const CoordinatePolicy PolarProductOp::polarCoordPolicy(CoordinatePolicy::POLAR, CoordinatePolicy::WRAP, CoordinatePolicy::LIMIT,CoordinatePolicy::WRAP);

void PolarProductOp::deriveDstGeometry(const DataSetMap<PolarSrc> & srcSweeps, PolarODIM & dstOdim) const {

	drain::Logger mout(name+"(CumulativeProductOp)", __FUNCTION__);

	const bool MAXIMIZE_AZM_RESOLUTION = (dstOdim.nrays == 0);
	const bool DERIVE_NBINS  = (dstOdim.nbins == 0); // ||(dstOdim.rscale == 0.0);
	const bool DERIVE_RSCALE = (dstOdim.rscale == 0.0); // (dstOdim.nbins == 0); //

	const bool AUTOSCALE_RANGE = (DERIVE_NBINS && DERIVE_RSCALE);

	double   rangeMax = 0;
	long int nbinsMax = 0;
	double  rscaleMin = 2000;

	mout.debug(1) << (dstOdim) << mout.endl;

	if (MAXIMIZE_AZM_RESOLUTION || DERIVE_NBINS || DERIVE_RSCALE){

		double range;

		for (DataSetMap<PolarSrc>::const_iterator it = srcSweeps.begin(); it != srcSweeps.end(); ++it){

			const DataSet<PolarSrc> & srcDataSet = it->second;
			const Data<PolarSrc>       & srcData    = srcDataSet.getFirstData();
			const PolarODIM            & srcOdim    = srcData.odim;

			if (srcData.data.isEmpty()){
				mout.warn() << "srcData empty" << mout.endl;
				continue; // warning issued later, in the main loop.
				//mout.warn() << "selected quantity=" << quantity << " not present in elangle=" << it->first << mout.endl;
			}

			mout.debug(1) << "testing: " << srcOdim << mout.endl;

			if (MAXIMIZE_AZM_RESOLUTION){
				if (srcOdim.nrays > dstOdim.nrays){
					dstOdim.nrays = srcOdim.nrays;
					mout.info() << "Updating dst nrays to: " << dstOdim.nrays << mout.endl;
				}
			}

			nbinsMax  = std::max(nbinsMax,  srcOdim.nbins);
			rscaleMin = std::min(rscaleMin, srcOdim.rscale);
			range = static_cast<double>(srcOdim.nbins) * srcOdim.rscale;
			if (range > rangeMax){
				rangeMax = range;
				if (AUTOSCALE_RANGE){
					dstOdim.nbins  = srcOdim.nbins;
					dstOdim.rscale = srcOdim.rscale;
				}
			}

		}

		if (AUTOSCALE_RANGE){
			mout.debug() << "Applied input geometry with maximum range" << mout.endl;
		}
		else if (dstOdim.nbins ==0){
			dstOdim.nbins = rangeMax/static_cast<int>(dstOdim.rscale);
			mout.debug() << "Derived nbins=" << dstOdim.nbins << mout.endl;
		}
		else if (dstOdim.rscale ==0){
			dstOdim.rscale = rangeMax/static_cast<double>(dstOdim.nbins);
			mout.debug() << "Derived rscale=" << dstOdim.rscale << mout.endl;
		}
		else {
			mout.debug() << "Adapting user-defined nbins and rscale" << mout.endl;
		}

		mout.info() << "Setting dst geometry:" << dstOdim.nbins << "bin x " << dstOdim.rscale << "m/bin (" << (dstOdim.getMaxRange()/1000.0) << "km) " << mout.endl;

	}
	else {
		//mout.warn() << "output prop" << dstOdim << mout.endl;
		mout.info() << "output properties: " << EncodingODIM(dstOdim) << mout.endl;
	}
}




}  // namespace rack
