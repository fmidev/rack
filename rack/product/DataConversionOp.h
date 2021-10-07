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

#ifndef DATACONVERSIONOP2_H_
#define DATACONVERSIONOP2_H_

#include "data/Data.h"
#include "data/ODIM.h"
#include "data/ODIMPath.h"
#include "data/Quantity.h"
#include "data/QuantityMap.h"
//#include "drain/util/LinearScaling.h"
#include <hi5/Hi5.h>
#include "drain/image/Geometry.h"
#include "drain/image/Image.h"
#include "drain/image/ImageFrame.h"
#include "ProductOp.h"
//#include "drain/util/LinearScaling.h"
#include "drain/util/ValueScaling.h"
#include "drain/util/Log.h"
#include "drain/util/RegExp.h"
#include "drain/util/SmartMap.h"
#include "drain/util/Tree.h"
#include "drain/util/Type.h"
#include "drain/util/TypeUtils.h"
#include "drain/util/VariableMap.h"
//#include "VolumeTraversalOp.h"
#include <list>
#include <map>
#include <set>
#include <string>
#include <typeinfo>
//#include "drain/utility>

//#include "VolumeOpNew.h"


namespace rack {

/// Converts HDF5 data to use desired data type, scaling and encoding (ODIM gain, offset, undetect and nodata values).
/**
 *
 *  \tparam M - ODIM type
 *
 *  \see Conversion
 *
 */
template <class M>
class DataConversionOp: public ProductOp<M, M> {

public:

	typedef SrcType<M const> src_t;
	typedef DstType<M> dst_t;

	DataConversionOp(const std::string & type="C", double gain=1.0, double offset=0.0,
			double undetect=0.0, double nodata=255.0, std::string copyGroupSuffix="") :
				ProductOp<M, M>(__FUNCTION__, "Converts HDF5 data to use desired data type, scaling and encoding") {

		this->allowedEncoding.link("what:type", this->odim.type = type);
		this->allowedEncoding.link("what:gain", this->odim.scaling.scale = gain);
		this->allowedEncoding.link("what:offset", this->odim.scaling.offset = offset);
		this->allowedEncoding.link("what:undetect", this->odim.undetect = undetect);
		this->allowedEncoding.link("what:nodata", this->odim.nodata = nodata);

	}

	virtual ~DataConversionOp(){};

	/// Ensures data to be in standard type and scaling. Makes a converted copy if needed.
	//static	const Hi5Tree & getNormalizedDataOLD(const DataSet< src_t> & srcDataSet, DataSet<dst_t> & dstDataSet, const std::string & quantity){}:


	void processH5(const Hi5Tree &src, Hi5Tree &dst) const;

	virtual
	void processDataSet(const DataSet< src_t> & srcSweep, DataSet<dst_t> & dstProduct) const;

	/// Converts src to dst such that dst applies desired gain, offset, undetect and nodata values.
	//inline
	//void processPlainData(const PlainData< src_t> & src, PlainData<dst_t> & dst) const;

	inline
	void processImage(const PlainData< src_t> & src, drain::image::Image & dst) const {
		processImage(src.odim, src.data, this->odim, dst);
	}


	/// Converts src to dst such that dst applies desired gain, offset, undetect and nodata values.
	/**
	 *   Sometimes this is applied directly (for alpha channel ops).
	 */
	void processImage(const ODIM & odimSrc, const drain::image::ImageFrame & src, const ODIM & odimDst, drain::image::Image & dst) const;

	void traverseImageFrame(const ODIM & odimSrc, const drain::image::ImageFrame & src, const ODIM & odimDst, drain::image::ImageFrame & dst) const;


	inline
	void setGeometry(const M & srcODIM, PlainData<dst_t> & dstData) const {
		// Does not change geometry.
	}


	static
	PlainData< DstType<M> > & getNormalizedData(const DataSet<src_t> & srcDataSet, DataSet<dst_t> & dstDataSet, const std::string & quantity);


protected:

	/// Suffix for trailing path element ("/data") for storing the original.
	std::string copyGroupSuffix;

};


template <class M> //
PlainData< DstType<M> > & DataConversionOp<M>::getNormalizedData(const DataSet<src_t> & srcDataSet, DataSet<dst_t> & normDataSet,
		const std::string & quantity) { // , const PlainData< src_t> & mika

	drain::Logger mout("DataConversionOp<>", __FUNCTION__);

	const std::string quantityExt = quantity+"_norm";  // std::string("~") +

	//typename DataSet< SrcType<M const> >::const_iterator it = normDataSet.find(quantityExt);
	typename DataSet<dst_t >::iterator it = normDataSet.find(quantityExt);
	if (it != normDataSet.end()){
		mout.note() << "using cached data: " << quantityExt << mout.endl;
		return it->second;
	}
	else {

		const PlainData<src_t> &  srcData = srcDataSet.getData(quantity);
		const EncodingODIM     & odimNorm = getQuantityMap().get(quantity).get(); //[srcData.odim.type.at(0)];

		mout.info() << "converting and adding to cache: " << quantityExt << " odim: " << odimNorm << mout.endl;
		PlainData<dst_t> & dstDataNew = normDataSet.getData(quantityExt);
		dstDataNew.setNoSave();
		DataConversionOp<M> op;
		//op.odim.importMap(odimNorm);
		// mout.warn() << "odimNorm: " << odimNorm << mout.endl;
		// mout.warn() << "op.odim: " << op.odim << mout.endl;
		dstDataNew.odim.importMap(srcData.odim);
		dstDataNew.odim.importMap(odimNorm);
		dstDataNew.odim.quantity = quantity;
		//op.processData(srcData, dstDataNew);
		op.processImage(srcData.odim, srcData.data, dstDataNew.odim, dstDataNew.data);
		dstDataNew.odim.quantity = quantityExt;
		dstDataNew.updateTree2(); // @?
		mout.debug() << "obtained: " << dstDataNew << mout.endl;

		return dstDataNew;
	}

	//return normDataSet.getData(quantityExt);

}



template <class M> //// copied from VolumeOp::processVolume
void DataConversionOp<M>::processH5(const Hi5Tree &src, Hi5Tree &dst) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.debug() << "start" << mout.endl;
	mout.debug3() << *this << mout.endl;
	mout.debug2() << "DataSelector: "  << this->dataSelector << mout.endl;

	/// Usually, the operator does not need groups sorted by elevation.
	mout.debug3() << "collect the applicable paths"  << mout.endl;
	ODIMPathList dataPaths;
	this->dataSelector.getPaths(src, dataPaths); //, ODIMPathElem::DATA);

	mout.debug3() << "populate the dataset map, paths=" << dataPaths.size() << mout.endl;
	// Parents are needed because converted data are stored in parallel.
	std::set<ODIMPathElem> parents;

	const drain::RegExp quantityRegExp(this->dataSelector.quantity);

	for (ODIMPathList::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

		//mout.debug3() << "elangles (this far> "  << elangles << mout.endl;
		//mout.debug() << *it << mout.endl;

		const ODIMPath & parentPath = *it;
		//parentPath.pop_back();
		const ODIMPathElem & parent = parentPath.back();

		mout.debug() << "check " << parent << '<' << *it << mout.endl;

		if (parents.find(parent) == parents.end()){
			if (parent.getType() != ODIMPathElem::DATASET){
				mout.note() << "non-dataset group: " << parent << mout.endl;
			}
			mout.note() << "append " <<  parent << mout.endl;
			parents.insert(parent);
			//const
			DataSet<src_t> srcDataSet(src(parentPath), quantityRegExp);
			DataSet<dst_t> dstDataSet(dst(parentPath));
			processDataSet(srcDataSet, dstDataSet);
		}
		else {
			mout.note() << "already exists" << parent << mout.endl;
		}

	}
}

template <class M>
void DataConversionOp<M>::processDataSet(const DataSet<src_t> & srcSweep, DataSet<dst_t> & dstProduct) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	std::set<std::string> convertedQuantities;

	const std::string extension("_X");

	// Traverse quantities
	for (typename DataSet<src_t>::const_iterator it = srcSweep.begin(); it != srcSweep.end(); ++it){

		const std::string & quantity = it->first;

		if (quantity.empty()){
			mout.warn() << "empty quantity for data, skipping" << mout.endl;
			continue;
		}

		//std::stringstream sstr; //quantity); //+"-tmp");
		//sstr << "data0." << quantity;

		mout.debug() << "quantity: " << quantity << mout.endl;

		const Data< src_t> & srcData = it->second;
		Data<dst_t>       & dstData = dstProduct.getData(quantity + extension); // todo: getNewData
		//dstProduct.getData(quantity).setNoSave(true);

		mout.debug2() << EncodingODIM(this->odim) << mout.endl;
		//mout.toOStr() << "src " << (long int) &(srcData.data) << EncodingODIM(srcData.odim) << mout.endl;
		//mout.warn() << "dst " << (long int) &(dstData.data) << EncodingODIM(dstData.odim) << mout.endl;

		const drain::Type t(this->odim.type);

		//const bool IN_PLACE = (&dstData.data == &srcData.data) && (t == srcData.data.getType());
		const bool IN_PLACE = false;
		if (IN_PLACE){

			if (ODIM::haveSimilarEncoding(srcData.odim, this->odim)){
				mout.info() << "already similar encoding, no need to convert" << mout.endl;
				continue; // to next quantity
			}

			mout.fatal() << "not implemented" << mout.endl;
			mout.debug() << "in-place" << mout.endl;
			//processData(srcData, dstData);
			//processImage(srcData.odim, srcData.data, dstData.odim, dstData.data);
			//@ dstData.updateTree();

		}
		else {

			mout.info() << "using tmp data (in-place computation not possible)" << mout.endl;

			convertedQuantities.insert(it->first);

			if (ODIM::haveSimilarEncoding(srcData.odim, this->odim)){
				mout.info() << "already similar encoding, no need to convert" << mout.endl;
				continue; // to next quantity
			}

			const M srcODIM(srcData.odim); // copy, because src may be modified next
			dstData.odim.quantity = quantity;
			dstData.odim.updateLenient(srcODIM); // <= dstData.odim.NI = srcData.odim.NI; // if Cart?
			ProductBase::completeEncoding(dstData.odim, this->targetEncoding);
			//processData(srcData, dstData2);
			processImage(srcODIM, srcData.data, dstData.odim, dstData.data);



		}

	}

	/// SWAP & delete
	mout.debug3() << "Swap & mark for delete" << mout.endl;
	for (std::set<std::string>::const_iterator qit = convertedQuantities.begin(); qit != convertedQuantities.end(); ++qit){

		mout.debug() << "Swapping quantity: " << *qit << '/' << extension << mout.endl;

		Data<dst_t> & dstDataOrig = dstProduct.getData(*qit);
		Data<dst_t> & dstDataConv = dstProduct.getData(*qit + extension);

		dstDataOrig.swap(dstDataConv); // calls updateTree2 (consider what:quantity)

		dstDataConv.odim.quantity = *qit+extension;
		dstDataConv.setNoSave(true);
	}

}


template <class M>
void DataConversionOp<M>::processImage(const ODIM & srcOdim, const drain::image::ImageFrame & srcImage, const ODIM & dstOdim, drain::image::Image & dstImage) const {


	drain::Logger mout(__FUNCTION__, __FILE__);
	mout.debug2() << "start, type=" << this->odim.type << ", geom=" << srcImage.getGeometry() << mout.endl;

	// const drain::Type t(this->odim.type);
	const drain::Type t(dstOdim.type);

	const drain::image::Geometry g(srcImage.getGeometry());

	if (srcImage.hasOverlap(dstImage)){
		if ((t.getType() != srcImage.getType()) || (g != dstImage.getGeometry())){
			mout.warn() << "using temp image + swap" << mout.endl;
			drain::image::Image tmp;
			tmp.setType(t);
			tmp.setGeometry(g);
			tmp.setScaling(dstOdim.scaling);
			traverseImageFrame(srcOdim, srcImage, dstOdim, tmp);
			dstImage.swap(tmp);
			//dstImage.copyDeep(tmp);
			return;
		}
		else {
			mout.warn() << "same type and geometry, hence only rescaling (in-place)" << mout.endl;
		}
		/*
		if (t != srcImage.getType2()){
			mout.error() << "trying to change type when dst==src" << mout.endl;
			return;
		}
		if (g != dstImage.getGeometry()){
			mout.error() << "trying to change geometry when dst==src" << mout.endl;
			return;
		}
		 */
	}
	else {
		dstImage.setType(t);
		dstImage.setGeometry(g);
		//dstImage.setScaling(dstOdim.scaling.scale, dstOdim.scaling.offset);
		dstImage.setScaling(dstOdim.scaling); // ok separate
		mout.debug() << "dst:" << dstImage << mout.endl;
		traverseImageFrame(srcOdim, srcImage, dstOdim, dstImage);
	}


}

template <class M>
void DataConversionOp<M>::traverseImageFrame(const ODIM & srcOdim, const drain::image::ImageFrame & srcImage,
		const ODIM & dstOdim, drain::image::ImageFrame & dstImage) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.debug2() << "dst:" << dstImage << mout.endl;

	dstImage.setCoordinatePolicy(srcImage.getCoordinatePolicy());

	//const double ud = std::max(odimOut.undetect, dst.getMin<double>());
	//const double nd = std::min(odimOut.nodata, dst.getMax<double>());

	//mout.debug3() << "input name: " << src.getName() << mout.endl;

	mout.debug2() << "src odim: " << EncodingODIM(srcOdim) << mout.endl;
	mout.debug(4) << "src props:" << srcImage.properties << mout.endl;
	//std::cerr << src.properties << std::endl;


	dstImage.properties = srcImage.properties;
	dstImage.properties.updateFromMap(dstOdim);
	//dst.odim.set(odim);
	//mout.debug2() << "op  odim: " << EncodingODIM(odim) << mout.endl;
	mout.debug2() << "dst odim: " << EncodingODIM(dstOdim) << mout.endl;
	mout.debug2() << "dst props: " << dstImage.properties << mout.endl;
	//std::cerr << dst.properties << std::endl;

	// const drain::LinearScaling scaling(srcOdim.scaling.scale, srcOdim.scaling.offset, dstOdim.scaling.scale, dstOdim.scaling.offset);
	const drain::ValueScaling scaling(srcOdim.scaling.scale, srcOdim.scaling.offset, dstOdim.scaling.scale, dstOdim.scaling.offset);

	typedef drain::typeLimiter<double> Limiter;
	Limiter::value_t limit = drain::Type::call<Limiter>(dstOdim.type);


	mout.debug2() << "scaling: " << scaling << mout.endl;

	Image::const_iterator s = srcImage.begin();
	Image::iterator d = dstImage.begin();

	// Long int check by wrting to pixel at (0,0)
	*d = dstOdim.nodata;
	if (static_cast<double>(*d) != dstOdim.nodata){
		mout.note() << "dstOdim.nodata=" << dstOdim.nodata << " -> " << static_cast<double>(*d) << mout.endl;
		mout.warn() << "dstOdim.nodata type conversion " << dstOdim.type << " -> " << drain::Type::getTypeChar(dstImage.getType()) << " changed the value" << mout.endl;
	}
	//mout.debug() << "dstOdim nodata long-int check " << dstOdim.nodata << " <> " << (long int)(*d = dstOdim.nodata) << mout.endl;


	mout.debug3() << "src:    " << srcImage << mout.endl;
	mout.debug3() << "dst: " << dstImage << mout.endl;
	double x;
	while (s != srcImage.end()){
		x = *s;

		/// Checks 'undetect' first because 'undetect' and 'nodata' may be the same code
		if (x == srcOdim.undetect)
			*d = dstOdim.undetect;
		else if (x == srcOdim.nodata)
			*d = dstOdim.nodata;
		else {
			//  x = srcOdim.scaleForward(x);
			//  x = dst.odim.scaleInverse(x);
			// *d = dstImage.limit<double>( x );
			// *d = limit( scaling.forward(x) );
			*d = limit( scaling.fwd(x) );
			//dstImage.scaling.limit<double>( scaling.forward(x) );
		}

		++s;
		++d;
	}

	mout.debug3() << "finished." << mout.endl;

}



}

#endif /* DATACONVERSIONOP_H_ */

// Rack
// REP // REP // REP // REP
