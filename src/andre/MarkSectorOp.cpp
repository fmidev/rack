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


#include <drain/imageops/ImageModifierPack.h>

#include "hi5/Hi5Write.h"
#include "MarkSectorOp.h"


namespace rack {


void MarkSectorOp::runDetection(const DataSet<PolarSrc> & srcDataSet, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & cache) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.warn(*this );
	// mout.debug("start" );

	//PlainData<PolarDst> & dstData = cache.getData("DBZH"); // drain::RegExp(dataSelector.quantity));

	//mout.warn(cache);

	drain::image::ImageBox boxOp;
	//boxOp.physicalScale = false; // physicalValue;

	if (iRange.width() > dstProb.data.getWidth()){
		mout.error("iRange ", iRange, " exceeds data array width: ", dstProb.data.getWidth());
	}
	if (jRange.width() > dstProb.data.getHeight()){
		mout.error("jRange ", jRange, " exceeds data array height:", dstProb.data.getHeight());
	}

	boxOp.iRange = iRange;
	boxOp.jRange = jRange;

	boxOp.setParameter("value", this->prob); // dstProb.odim.scaleInverse(this->prob));
	boxOp.physicalScale = true; // use physical probability value, like 0.75 or 1.0;
	boxOp.process(dstProb.data);

	if (!value.empty()){

		// if cache.empty() warn?

		double v;
		boxOp.physicalScale = false; // physical value handled externally below.

		if (cache.size() > 1){
			mout.note("Writing marker for several (", cache.size(), ") quantities");
			mout.pending<LOG_NOTICE>(cache);
		}

		for (auto & entry: cache){

			PlainData<PolarDst> & dstData = entry.second; // cache.getFirstData(); // drain::RegExp(dataSelector.quantity));
			if (value == "nodata"){
				v = dstData.odim.nodata;
			}
			else if (value == "undetect"){
				v = dstData.odim.undetect;
			}
			else {
				v = getParameter<double>("value");
				if (physicalValue){
					v = dstData.odim.scaling.inv(v);
				}
			}

			if (cache.size() > 1){
				mout.experimental("Marking data\t[", dstData.odim.quantity,"]\t with bit value: ", v, " user arg value: ", value);
			}

			//boxOp.setParameter("value", this->value); //  dstData.odim.scaleInverse(this->value));
			boxOp.setParameter("value", v); //  dstData.odim.scaleInverse(this->value));
			// boxOp.physicalScale = false; // physicalValue;
			boxOp.process(dstData.data);
		}
	}


	// mout.warn(boxOp );

}


}

// Rack
