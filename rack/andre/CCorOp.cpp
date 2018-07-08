/**

    Copyright 2010-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include "CCorOp.h"

#include <drain/util/Fuzzy.h>
#include <drain/image/File.h>
#include <drain/imageops/SlidingWindowMedianOp.h>

#include "hi5/Hi5Write.h"
//#include "data/ODIM.h"
//#include "main/rack.h"

//#include <drain/imageops/SegmentAreaOp.h>
//#include <drain/image/MathOpPack.h>

using namespace drain::image;

namespace rack {



//void CCorOp::processDataSet(const DataSetSrc & src, DataSetDst & dst) const {
void CCorOp::processDataSet(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & aux) const {

	drain::Logger mout(name, __FUNCTION__);
	//mout.debug() << parameters << mout.endl;

	const Data<PolarSrc> & srcTH   = src.getData("TH");
	const Data<PolarSrc> & srcDBZH = src.getData("DBZH");

	if (srcTH.data.isEmpty()){
		mout.warn() << "input data of TH missing, giving up." << mout.endl;
		return;
	}

	if (srcDBZH.data.isEmpty()){
		mout.warn() << "input data of DBZH missing, giving up." << mout.endl;
		return;
	}

	const drain::image::Geometry & geometry = srcTH.data.getGeometry();

	if (srcDBZH.data.getGeometry() != geometry){
		mout.warn() << "different geometry in TH and DBZH (unsupported), giving up." << mout.endl;
		return;
	}


	//PlainDataDst & dstProb = dst;
			//dst.getQualityData("ACCORD");
	//dstProb.data.setGeometry(geometry);
	const double QMAX = dstProb.odim.scaleInverse(1.0);

	/// Main loop
	double dbzh, th;
	drain::FuzzyBell<double> fuzzy(0.0, reflHalfWidth, QMAX);
	Image::const_iterator ith   = srcTH.data.begin();
	Image::const_iterator idbzh = srcDBZH.data.begin();
	Image::const_iterator it = dstProb.data.begin();
	while (ith != srcTH.data.end()){
		th = *ith;
		if (th == srcTH.odim.nodata){
			*it = 0;
		}
		else if (th != srcTH.odim.undetect){
			dbzh = *idbzh;
			if (dbzh != srcDBZH.odim.nodata){
				if (dbzh == srcDBZH.odim.undetect)
					dbzh = -32.0;
				else
					dbzh = srcDBZH.odim.scaleForward(dbzh);
				th = srcTH.odim.scaleForward(th);
				*it = QMAX - fuzzy(dbzh - th);
			}
		}
		else {
			*it = 0;
		}
		++ith;
		++idbzh;
		++it;
	}

	writeHow(dstProb);

}

}
