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


#include "DamperOp.h"

#include "data/QuantityMap.h"

// using namespace drain::image;
// using namespace hi5;



namespace rack {



void DamperOp::processData(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality, PlainData<PolarDst> & dstData, PlainData<PolarDst> & dstQuality) const {


	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug() << "start" << mout.endl;

	drain::FuzzyStep<double> fstep(threshold, threshold+(1.0-threshold)/2.0);

	double minVal = this->minValue;

	const QuantityMap & qMap = getQuantityMap();
	if (isnan(minVal)){
		if (qMap.hasQuantity(srcData.odim.quantity)){
			const Quantity & quantity = qMap.get(srcData.odim.quantity);
			if (quantity.hasUndetectValue()){
				mout.info() << "using physical undetect value: " << quantity.undetectValue << ", set by --quantityConf" << mout.endl;
				minVal = quantity.undetectValue;
			}
		}
	}

	//if (minValue == -std::numeric_limits<double>::max()){
	if (isnan(minVal)){
		mout.error() << "minimum physical value unset for ["<< srcData.odim.quantity << "] (no parameter or quantityConf)" << mout.endl;
		return;
	}

	Image::iterator d  = dstData.data.begin();  // fixme: const object allows non-const iterator
	Image::const_iterator q = srcQuality.data.begin();
	const Image::const_iterator dEnd = dstData.data.end();
	double x;
	double w;
	//data.setLimits(-256.0, 255.0);

	mout.note() << "data " << dstData.data << mout.endl;
	const drain::image::Encoding & encoding = dstData.data.getConf();
	//mout.note() << "encoding " << encoding.get << mout.endl;
	mout.debug3() << "limits: " << encoding.getTypeMin<double>() << ',' << encoding.getTypeMax<double>() << mout.endl;
	//data.toOStr(std::cout);

	drain::typeLimiter<double>::value_t limit = encoding.getLimiter<double>();

	while (d != dEnd){

		x = *d;

		if (srcData.odim.isValue(x)){

			w = fstep(srcQuality.odim.scaleForward(*q));
			if (w < undetectThreshold){
				*d = dstData.odim.undetect;
			}
			else {
				x = srcData.odim.scaleForward(x);

				if (x < minVal)
					*d = dstData.odim.undetect;
				else
					*d = limit(srcData.odim.scaleInverse(minVal + w*(x-minVal)));
			}
		}
		++d;
		++q;
	};

	//drain::image::File::write(data,"Eras1.png");


}

}


// Rack
