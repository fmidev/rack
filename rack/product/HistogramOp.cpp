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


#include "HistogramOp.h"


namespace rack {



	void HistogramOp::processH5(const Hi5Tree &src, Hi5Tree &dst) const {

		drain::Logger mout(__FUNCTION__, __FILE__);

		ODIMPathList dataPaths;

		this->dataSelector.getPaths(src, dataPaths, ODIMPathElem::DATA | ODIMPathElem::QUALITY);

		for (ODIMPathList::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){
			mout.note() << "add: " << *it  << mout.endl;
			PlainData<BasicSrc> data(src(*it));
			mout.warn() << "add: " << data  << mout.endl;
			//sweeps.insert(typename DataSetMap<src_t>::value_type(index, DataSet<src_t>(src(*it), drain::RegExp(this->dataSelector.quantity) )));  // Something like: sweeps[elangle] = src[parent] .
		}


	}


	void HistogramOp::processData(const Data<BasicSrc> & srcData, Data<BasicDst> & dstData) const {

		drain::Logger mout(__FUNCTION__, __FILE__);
		const drain::image::Image & img = srcData.data;

		const int min = 0;
		const int max = dstData.data.getWidth()-1;

		if (!drain::Type::call<drain::typeIsSmallInt>(img.getType())){
			mout.warn() << "src type not smallInt" << mout.endl;
		}

		int x;
		for (drain::image::Image::const_iterator it = img.begin(); it != img.end(); ++it){
			x = static_cast<int>(*it);
			if ((x>min) && (x<=max)){
				//img.ge
				//histogram.increment(x);
				//dstData.data.put(x, dstData.data.template get<int>(x) + 1);
			}
		}



	}


} // Rack

