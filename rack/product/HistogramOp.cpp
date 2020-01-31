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

#include <algorithm>

// #include <drain/util/Histogram.h>

#include <drain/imageops/ImageModifierPack.h>


#include "HistogramOp.h"


namespace rack {



void HistogramOp::processH5(Hi5Tree &dst) const {

	drain::Logger mout(__FUNCTION__, __FILE__);
	mout.note() << "start"  << mout.endl;


	ODIMPathList dataPaths;

	this->dataSelector.getPaths3(dst, dataPaths);

	if (dataPaths.size() > 1){
		//if ((!dataSelector.quantityRegExp.isSet()) && (!dataSelector.qualityRegExp.isSet())){
		if (dataSelector.quantity.empty()){
			mout.warn() << "no quality constraint set and (hence) several datasets found" << mout.endl;
		}
	}
	else if (dataPaths.empty()){
		mout.warn() << "no data found" << mout.endl;
	}

	//DataSet<BasicSrc> & result = dst();
	std::string quantity;

	for (ODIMPathList::iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

		// Strip
		if (it->back().is(ODIMPathElem::ARRAY)){
			it->pop_back();
		}

		mout.note() << "processing: " << *it << mout.endl;
		PlainData<BasicDst> data(dst(*it));

		if (quantity.empty()){
			quantity = data.odim.quantity;
		}
		else if (quantity != data.odim.quantity){
			mout.warn() << "collecting " << quantity << ", encountered also: " << data.odim.quantity << mout.endl;
		}

		drain::image::Image & img = data.data;

		drain::image::ImageHistogram histOp;
		histOp.bins = drain::Histogram::recommendSizeByType(img.getType(), count);

		histOp.filename = filename;

		histOp.traverseChannel(img.getChannel(0));

		mout.note() << "done: " << *it << mout.endl;
		mout.note() << "done: " << img.getChannel(0).getProperties() << mout.endl;
		//ODIMPath p(*it);
		//PlainData<BasicDst> result(dst(*it), data.odim.quantity + "_HIST");
		//PlainData<BasicDst> result2(dst(*it)["quality13"], data.odim.quantity + "_GHIST");

		//mout.warn() << "add: " << data  << mout.endl;
		//processData(data);

	}


}

// const PlainData<BasicSrc> & srcData,
void HistogramOp::processData(PlainData<BasicDst> & dstData) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	const drain::image::Image & img = dstData.data;
	const std::type_info & type = img.getType();


	if (!drain::Type::call<drain::typeIsSmallInt>(type)){
		mout.warn() << "src type not smallInt" << mout.endl;
		return;
	}

	drain::Histogram histogram; //(drain::Type::call<drain::sizeGetter>(img.getType()));

	histogram.setSize(256);

	/*
	if (count > 0){
		histogram.setSize(count);
	}
	else {
		if (drain::Type::call<drain::typeIsSmallInt>(type)){
			const size_t s = drain::Type::call<drain::sizeGetter>(type);
			const size_t bits = (s*8);
			mout.note() << bits << "bits => setting " << (1<<bits) << " bins " << mout.endl;
			histogram.setSize(1<<bits);
		}
		else {
			mout.note() << "assuming 256 bins" << mout.endl;
			histogram.setSize(256);
		}
		//dstData.setGeometry(1, 256);
	}
	*/

	// int x;
	for (drain::image::Image::const_iterator it = img.begin(); it != img.end(); ++it){
		if (dstData.odim.isValue(*it)){
			histogram.increment(dstData.odim.scaleForward(*it));
		}
		//x = static_cast<int>(*it);
		// if ((x>min) && (x<=max)){
	}

	//dstData.setGeometry(1, histogram.getSize());

	// const std::vector<drain::Histogram::count_t> & v = histogram.getVector();
	dstData.getHow()["histogram"] = histogram.getVector();
	dstData.updateTree2();

	/*
	std::vector<drain::Histogram::count_t>::const_iterator vit = v.begin();
	drain::image::Image::const_iterator it = dstData.data.begin();

	while ((vit != v.end()) && (it != dstData.data.end())){

		*it = *vit;

		++vit;
		++it;
	}
	*/
	// const std::type_info & type = srcData.data.getType();
	//std::copy(v.begin(), v.end(), dstData.data.get);


}


} // Rack

