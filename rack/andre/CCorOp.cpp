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
#include "CCorOp.h"

#include "drain/util/Fuzzy.h"
#include "drain/imageops/SlidingWindowMedianOp.h"

#include "hi5/Hi5Write.h"
//#include "data/ODIM.h"
//#include "main/rack.h"

//#include "drain/imageops/SegmentAreaOp.h"
//#include "drain/image/MathOpPack.h"

using namespace drain::image;

namespace rack {



//void CCorOp::processDataSet(const DataSetSrc & src, DataSetDst & dst) const {
void CCorOp::processDataSet(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & aux) const {

	drain::Logger mout(__FUNCTION__, __FILE__);
	//mout.debug() << parameters << mout.endl;

	// TODO: select H or V

	const Data<PolarSrc> & srcTH   = src.getData("TH");
	const Data<PolarSrc> & srcDBZH = src.getData("DBZH");

	if (srcTH.data.isEmpty()){
		mout.warn() << "input data of TH missing, giving up." << mout.endl;
		// or error flag?
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

	const double QMAX = dstProb.odim.scaleInverse(1.0);


	Data<PolarDst> & dstAux = aux.getData("TH_DBZH"); // TODO: toggle letter
	const QuantityMap & qm = getQuantityMap();
	dstAux.odim.updateFromCastableMap(srcDBZH.odim);
	dstAux.odim.quantity = "TH_DBZH";
	qm.setQuantityDefaults(dstAux, "DBZH", srcDBZH.odim.type); // SAFE, lower limit (like -32dBZ) never exceeded because TH-DBHZ always positive

	dstAux.setGeometry(geometry);

	/// Main loop
	double dbzh, th, diff;
	drain::FuzzyBell<double> fuzzy(0.0, reflHalfWidth, QMAX);
	Image::const_iterator ith   = srcTH.data.begin();
	Image::const_iterator idbzh = srcDBZH.data.begin();
	Image::iterator        it   = dstProb.data.begin();
	Image::iterator       cit   = dstAux.data.begin();
	while (ith != srcTH.data.end()){

		th = *ith;

		if (th == srcTH.odim.undetect){
			*it  = dstProb.odim.undetect;
			*cit = dstAux.odim.undetect;
		}
		else if (th == srcTH.odim.nodata){
			*it  = dstProb.odim.nodata;
			*cit = dstAux.odim.nodata;
		}
		else {
			dbzh = *idbzh;
			//if (dbzh != srcDBZH.odim.nodata){
			if (!srcDBZH.odim.isValue(dbzh)){
				dbzh = -32.0;
			}
			else {
				dbzh = srcDBZH.odim.scaleForward(dbzh);
			}
			th = srcTH.odim.scaleForward(th);
			diff = th - dbzh;
			*it  = QMAX - fuzzy(diff);
			if (diff != 0.0)
				*cit = dstAux.odim.scaleInverse(diff);
			else
				*cit = dstAux.odim.nodata;
		}

		++ith;
		++idbzh;
		++it;
		++cit;
	}

	writeHow(dstProb);

}

}

// Rack
