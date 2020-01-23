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

#ifndef HistogramOP2_H_
#define HistogramOP2_H_

#include <drain/util/LinearScaling.h>

#include "data/ODIM.h"
#include "data/Data.h"
//&#include "PolarProductOp.h"
#include "VolumeTraversalOp.h"
//#include "VolumeOpNew.h"


namespace rack {


// TODO: generalize to ProductOp<ODIM>
/// Computes histogram
/**
 *
 *  \tparam M - ODIM type
 *
 *  \see Conversion
 *
 */
/*
template <class M>
class HistogramOp: public ProductOp<M, M> {

public:

	typedef SrcType<M const> src_t;
	typedef DstType<M> dst_t;
	*/

class HistogramOp: public ProductOp<ODIM, ODIM> {

public:

	//HistogramOp(size_t size=256, double minValue = std::numeric_limits<double>::min(), double maxValue = std::numeric_limits<double>::max()) :
	//HistogramOp() : ProductOp<M, M>(__FUNCTION__, "Computes HDF5 histogram") {
	HistogramOp() : ProductOp<ODIM,ODIM>(__FUNCTION__, "Computes HDF5 histogram") {

		this->parameters.reference("count", count = 0);
		this->parameters.reference("raw", raw = false, "[0|1]");
		this->parameters.reference("min", minValue = -std::numeric_limits<double>::max());
		this->parameters.reference("max", maxValue = +std::numeric_limits<double>::max());
		this->parameters.reference("filename", filename = "-");

		this->allowedEncoding.reference("type", this->odim.type = "C");
		//this->odim.quantity = "HGHT";
		this->odim.type = "L";
	}

	virtual ~HistogramOp(){};

	/// Ensures data to be in standard type and scaling. Makes a converted copy if needed.
	//static	const Hi5Tree & getNormalizedDataOLD(const DataSet< src_t> & srcDataSet, DataSet<dst_t> & dstDataSet, const std::string & quantity){}:
	virtual	void setGeometry(const ODIM & srcODIM, PlainData<BasicDst> & dstData) const {
	}

	/*
	inline
	std::ofstream & ofstr(const std::string & fileName, std::ios::out) const {
		std::string outFileName = resources.outputPrefix + fileName;
		std::ofstream ofstr(fileName.c_str(), std::ios::out);
		ofstr;
	}
	*/


	void processH5(const Hi5Tree &src, Hi5Tree &dst) const; /* {

		drain::Logger mout(__FUNCTION__, __FILE__);

		ODIMPathList dataPaths;

		this->dataSelector.getPaths(src, dataPaths, ODIMPathElem::DATA | ODIMPathElem::QUALITY);

		for (ODIMPathList::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){
			mout.note() << "add: " << *it  << mout.endl;
			PlainData<BasicSrc> data(src(*it));
			mout.warn() << "add: " << data  << mout.endl;
			//sweeps.insert(typename DataSetMap<src_t>::value_type(index, DataSet<src_t>(src(*it), drain::RegExp(this->dataSelector.quantity) )));  // Something like: sweeps[elangle] = src[parent] .
		}


	}*/


	virtual
	void processData(const Data<BasicSrc> & srcData, Data<BasicDst> & dstData) const; /* {

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



	};
	*/

	/// REJECTED. Very difficult to select QUALITY quantities, if using DataSetMap->DataSet->Data->PlainData hierarchy---
	/*
	void processDataSets(const DataSetMap<src_t> & src, DataSet<dst_t> & dstProduct) const {

		drain::Logger mout(__FUNCTION__, __FILE__);
		mout.debug(2) << "start" << mout.endl;

		if (src.size() == 0)
			mout.warn() << "no data" << mout.endl;

		drain::Histogram histogram(0);

		EncodingODIM metadata;

		const drain::RegExp quantityRegExp(this->dataSelector.quantity);
		mout.note() << quantityRegExp << mout.endl;

		//std::string typeStr;
		drain::Type type0;

		double min = minValue;
		double max = maxValue;

		//int finalCount = count;

		for (typename DataSetMap<src_t >::const_iterator it = src.begin(); it != src.end(); ++it) {

			const DataSet<src_t> & srcDataSet = it->second;

			mout.note() << "calling processDataSet for #" << it->first << " datasets=" << srcDataSet.size() << mout.endl;
			mout.warn() << srcDataSet << mout.endl;

			if (srcDataSet.size() > 1)
				mout.warn() << "several data groups selected in dataset" << mout.endl;

			const PlainData<src_t> & srcData    = (srcDataSet.size()>0) ? srcDataSet.getFirstData() : srcDataSet.getQualityData(quantityRegExp);
			//const PlainData<src_t> & srcQuality = srcDataSet.getQualityData(quantityRegExp);
			mout.warn() << "using quantity: " << srcData.odim.quantity << mout.endl;

			const PlainData<src_t> & srcDataQuality = srcDataSet.getFirstData().getQualityData(quantityRegExp);
			mout.warn() << "primary: " << srcDataQuality << mout.endl;
			//mout.note() << "using quantity: " << srcData.odim.quantity << mout.endl;

			const std::type_info & type = srcData.data.getType();

			if (histogram.getSize() == 0){

				type0.setType(type);

				if (count == 0){
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

				if (drain::Type::call<drain::typeIsSmallInt>(type)){

					if (srcData.odim.gain == 1.00){
						// Consider CLASS
						mout.note() << "assuming index-like values" << mout.endl;

						if (min == -std::numeric_limits<double>::max())
							min = 0;
						else
							mout.warn() << "min explicitly set for index-like values (gain==1)" << mout.endl;

						if (max == +std::numeric_limits<double>::max())
							max = histogram.getSize()-1;
						else
							mout.warn() << "max explicitly set for index-like values (gain==1)" << mout.endl;

					}
					else {
						// Consider DBZH
						if (min == -std::numeric_limits<double>::max())
							min = srcData.odim.getMin();
						if (max == +std::numeric_limits<double>::max())
							max = srcData.odim.getMax();
					}
				}



				if (min == -std::numeric_limits<double>::max()){
					mout.warn() << "min value unset, and input type not small int" << mout.endl;
					min = -100.0;
				}

				if (max == +std::numeric_limits<double>::max()){
					mout.warn() << "max value unset, and input type not small int" << mout.endl;
					max = +100.0;
				}

				histogram.setScale(min, max);
				mout.note() << "setting range: [" << min << ',' << max << "]" << mout.endl;

				metadata.updateFromMap(srcData.odim);
			}

			if (type0.getType() != type){
				mout.warn() << "type changed from " << drain::Type::getTypeChar(type0) << " => " << drain::Type::getTypeChar(type) << mout.endl;
			}

			/// Main inner loop ("processData")
			const drain::image::Image & img = srcData.data;
			double x;
			for (drain::image::Image::const_iterator it = img.begin(); it != img.end(); ++it){
				x = srcData.odim.scaleForward(*it);
				if ((x>=min) && (x<=max)){ // undetect
					histogram.increment(x); // real
				}
				//histogram.increment(*it); // raw
				//if ((x != srcData.odim.undetect) && (x != srcData.odim.undetect))

			}
			mout.info() << histogram.getSampleCount() << " samples" << mout.endl;
		}

		//Data<dst_t> & dstData = dstProduct.getData("HIST");
		if (filename == "-"){
			histogram.dump(std::cout);
		}
		else {
			// std::string outFileName = resources.outputPrefix + filename;
			std::ofstream ofstr(filename.c_str(), std::ios::out);
			ofstr << '#' << metadata << '\n';
			histogram.dump(ofstr);
			ofstr.close();
		}

	}
	*/

	//mutable


protected:

	int count;
	double minValue;
	double maxValue;
	bool raw;

	std::string filename;
};


}

#endif /* HistogramOP_H_ */

// Rack
