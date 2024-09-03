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

#include <list>
#include <map>
#include <set>
#include <string>
#include <typeinfo>

#include <drain/Log.h>
#include <drain/RegExp.h>
#include <drain/Type.h>
#include <drain/TypeUtils.h>
#include <drain/util/StringMatcherList.h>
//#include "drain/util/LinearScaling.h"
#include "drain/util/SmartMap.h"
#include "drain/util/TreeOrdered.h"
#include "drain/util/ValueScaling.h"
#include "drain/util/VariableMap.h"

#include "drain/image/Geometry.h"
#include "drain/image/Image.h"
#include "drain/image/ImageFrame.h"

#include "data/Data.h"
#include "data/ODIM.h"
#include "data/ODIMPath.h"
#include "data/Quantity.h"
#include "data/QuantityMap.h"
//#include "drain/util/LinearScaling.h"
#include <hi5/Hi5.h>
#include "ProductOp.h"

//#include "VolumeTraversalOp.h"
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
class DataConversionOp: public ProductOp<M,M> {

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

		//this->dataSelector.pathMatcher.setElems(ODIMPathElem::DATASET, ODIMPathElem::DATA);
		this->dataSelector.setPathMatcher(ODIMPathElem::DATASET, ODIMPathElem::DATA);

	}

	virtual ~DataConversionOp(){};

	/// Ensures data to be in standard type and scaling. Makes a converted copy if needed.
	//  static	const Hi5Tree & getNormalizedDataOLD(const DataSet< src_t> & srcDataSet, DataSet<dst_t> & dstDataSet, const std::string & quantity){}:

	/// Converts src to dst such that dst applies desired gain, offset, undetect and nodata values.
	virtual
	void processH5(const Hi5Tree &src, Hi5Tree &dst) const;

	virtual
	void processDataSet(const DataSet<src_t> & srcSweep, DataSet<dst_t> & dstProduct) const;


	inline
	void processImage(const PlainData< src_t> & src, drain::image::Image & dst) const {
		processImage(src.odim, src.data, this->odim, dst);
	}

	/// Converts src to dst such that dst applies desired gain, offset, undetect and nodata values.
	/**
	 *   Sometimes this is applied directly (for alpha channel ops).
	 */
	void processImage(const ODIM & odimSrc, const drain::image::ImageFrame & src, const ODIM & odimDst, drain::image::Image & dst) const;

	void processImage2023(const ODIM & srcOdim, const drain::image::ImageFrame & src, drain::image::Image & dst) const;


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

	drain::Logger mout(__FILE__, __FUNCTION__);

	const std::string quantityExt = quantity+"_norm";  // std::string("~") +

	//typename DataSet< SrcType<M const> >::const_iterator it = normDataSet.find(quantityExt);
	typename DataSet<dst_t >::iterator it = normDataSet.find(quantityExt);
	if (it != normDataSet.end()){
		mout.note("using cached data: ", quantityExt);
		return it->second;
	}
	else {

		const PlainData<src_t> &  srcData = srcDataSet.getData(quantity);
		const EncodingODIM     & odimNorm = getQuantityMap().get(quantity).get(); //[srcData.odim.type.at(0)];

		mout.info("converting and adding to cache: " , quantityExt , " odim: " , odimNorm );
		PlainData<dst_t> & dstDataNew = normDataSet.getData(quantityExt);
		dstDataNew.setExcluded();
		DataConversionOp<M> op;
		//op.odim.importMap(odimNorm);
		// mout.warn("odimNorm: " , odimNorm );
		// mout.warn("op.odim: " , op.odim );
		dstDataNew.odim.importMap(srcData.odim);
		dstDataNew.odim.importMap(odimNorm);
		dstDataNew.odim.quantity = quantity;
		//op.processData(srcData, dstDataNew);
		op.processImage(srcData.odim, srcData.data, dstDataNew.odim, dstDataNew.data);
		dstDataNew.odim.quantity = quantityExt;
		dstDataNew.updateTree2(); // @?
		mout.debug("obtained: " , dstDataNew );

		return dstDataNew;
	}

	//return normDataSet.getData(quantityExt);

}



template <class M> //// copied from VolumeOp::processVolume
void DataConversionOp<M>::processH5(const Hi5Tree &src, Hi5Tree &dst) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug3(*this );
	mout.debug("DataSelector: ", this->dataSelector);

	/// The operator does not need groups sorted by elevation.
	ODIMPathList dataPaths;
	this->dataSelector.getPaths(src, dataPaths); //, ODIMPathElem::DATA);

	mout.special("obtained ", dataPaths.size(), " paths.");

	// Parents are needed because converted data are stored in parallel.
	std::set<ODIMPathElem> parents;

	//const drain::RegExp quantityRegExp(this->dataSelector.getQuantity());
	const drain::KeySelector & slct = this->dataSelector.getQuantitySelector();

	mout.special("slct: ", slct);

	// copy
	for (ODIMPath & path: dataPaths){

		if (path.front().is(ODIMPathElem::ROOT)){
			// is this needed?
			path.pop_front();
		}
		//const ODIMPathElem & parent = path.back();
		const ODIMPathElem & parent = path.front();

		mout.special("handling: ", parent, " -> ", path);

		if (parents.find(parent) == parents.end()){
			if (parent.getType() != ODIMPathElem::DATASET){
				mout.note("non-dataset group: ", parent);
			}
			mout.note("now handling: ", parent);
			parents.insert(parent);
			// DataSet<src_t> srcDataSet(src(path), slct);
			// DataSet<dst_t> dstDataSet(dst(path));
			DataSet<src_t> srcDataSet(src[parent], slct);
			DataSet<dst_t> dstDataSet(dst[parent]);
			processDataSet(srcDataSet, dstDataSet);
		}
		else {
			mout.note("already exists(?): ", parent); // ???
		}

	}
}

template <class M>
void DataConversionOp<M>::processDataSet(const DataSet<src_t> & srcSweep, DataSet<dst_t> & dstProduct) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	std::set<std::string> convertedQuantities;

	//const std::string extension("_X");

	mout.debug("number of layers (sub groups of DataSet): ", srcSweep.size());

	// Traverse quantities
	//for (typename DataSet<src_t>::const_iterator it = srcSweep.begin(); it != srcSweep.end(); ++it){
	for (const auto & entry: srcSweep){

		const std::string & quantity = entry.first;

		if (quantity.empty()){
			mout.warn("empty quantity for data, skipping");
			continue;
		}

		// Rescaled quantity, temporary name
		const std::string quantityTmp =  quantity+'*';

		mout.debug("quantity: ", quantity);

		const Data<src_t> & srcData = entry.second;
		Data<dst_t>       & dstData = dstProduct.getData(quantityTmp); // todo: getNewData
		//dstProduct.getData(quantity).setExcluded(true);
		mout.attention<LOG_DEBUG>("srcData: ", entry.second);

		mout.debug2(EncodingODIM(this->odim));
		//mout.toOStr() << "src " << (long int) &(srcData.data) << EncodingODIM(srcData.odim) << mout.endl;
		//mout.warn("dst " , (long int) &(dstData.data) , EncodingODIM(dstData.odim) );

		const drain::Type t(this->odim.type);

		//const bool IN_PLACE = (&dstData.data == &srcData.data) && (t == srcData.data.getType());
		const bool IN_PLACE = false;
		if (IN_PLACE){

			if (ODIM::haveSimilarEncoding(srcData.odim, this->odim)){
				mout.info("already similar encoding – no need to convert (1)");
				continue; // to next quantity
			}

			mout.unimplemented("in-place: not implemented");
			mout.debug("in-place");
			//processData(srcData, dstData);
			//processImage(srcData.odim, srcData.data, dstData.odim, dstData.data);
			//@ dstData.updateTree();

		}
		else {

			mout.info("using tmp data – in-place computation not possible");

			// convertedQuantities.insert(entry.first);

			if (ODIM::haveSimilarEncoding(srcData.odim, this->odim)){
				mout.info("already similar encoding – no need to convert (2)");
				continue; // to next quantity
			}

			convertedQuantities.insert(entry.first); // CHECK: was first abovw - ?

			//const M srcODIM(srcData.odim); // Copy, because src may be modified next
			const M srcODIM(srcData.data); // Copy, because src may be modified next
			mout.attention("srcData.odim: ", srcData.odim);
			mout.attention("srcODIM:      ", srcODIM);
			dstData.odim.quantity = quantity;
			dstData.odim.updateLenient(srcODIM); // <= dstData.odim.NI = srcData.odim.NI; // if Cart?
			ProductBase::completeEncoding(dstData.odim, this->targetEncoding);
			mout.special("Final encoding: ", (const EncodingODIM &)dstData.odim);
			//processData(srcData, dstData2);
			processImage(srcODIM, srcData.data, dstData.odim, dstData.data);

		}

	}

	/// SWAP & delete
	mout.debug("Swap & mark for deletion: ", drain::sprinter(convertedQuantities));
	for (const std::string & quantity: convertedQuantities){

		const std::string quantityTmp = quantity + '*';

		mout.special("Swapping quantity: ", quantity, " <-> ", quantityTmp);

		Data<dst_t> & dstDataOrig = dstProduct.getData(quantity);
		Data<dst_t> & dstDataConv = dstProduct.getData(quantityTmp);

		dstDataOrig.swap(dstDataConv); // calls updateTree2 (consider what:quantity)

		dstDataConv.odim.quantity = quantityTmp;
		dstDataConv.setExcluded(true);
	}

}

template <class M>
void DataConversionOp<M>::processImage2023(const ODIM & srcOdim, const drain::image::ImageFrame & srcImage,
		drain::image::Image & dstImage) const {

	// drain::Logger mout(__FILE__, __FUNCTION__);

	ODIM odim;
	odim.updateFromCastableMap(srcOdim); // quantity, etc
	ProductBase::completeEncoding(odim, this->targetEncoding);
	processImage(srcOdim, srcImage, odim, dstImage);

	//op.processImage2(srcOdim, srcImage, ctx.targetEncoding, ctx.grayImage);

}

template <class M>
void DataConversionOp<M>::processImage(const ODIM & srcOdim, const drain::image::ImageFrame & srcImage, const ODIM & dstOdim, drain::image::Image & dstImage) const {


	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.debug("type=", this->odim.type, ", geom=", srcImage.getGeometry() );

	// const drain::Type t(this->odim.type);
	const drain::Type t(dstOdim.type);

	const drain::image::Geometry g(srcImage.getGeometry());

	mout.attention<LOG_DEBUG>("srcOdim  ",   srcOdim);
	mout.attention<LOG_DEBUG>("srcImage:", srcImage);

	if (srcImage.hasOverlap(dstImage)){
		if ((t.getType() != srcImage.getType()) || (g != dstImage.getGeometry())){
			mout.warn("using temp image + swap");
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
			mout.warn("same type and geometry, hence only rescaling (in-place)");
		}
		/*
		if (t != srcImage.getType2()){
			mout.error("trying to change type when dst==src" );
			return;
		}
		if (g != dstImage.getGeometry()){
			mout.error("trying to change geometry when dst==src" );
			return;
		}
		 */
	}
	else {
		dstImage.setType(t);
		dstImage.setGeometry(g);
		//dstImage.setScaling(dstOdim.scaling.scale, dstOdim.scaling.offset);
		dstImage.setScaling(dstOdim.scaling); // ok separate
		mout.debug("dst:", dstImage);
		traverseImageFrame(srcOdim, srcImage, dstOdim, dstImage);
	}

	mout.attention<LOG_DEBUG>("dstOdim:  ",   dstOdim);
	mout.attention<LOG_DEBUG>("dstImage: ", dstImage);


}

template <class M>
void DataConversionOp<M>::traverseImageFrame(const ODIM & srcOdim, const drain::image::ImageFrame & srcImage,
		const ODIM & dstOdim, drain::image::ImageFrame & dstImage) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	// mout.debug3("input name: " , src.getName() );
	mout.debug2("src odim: ", EncodingODIM(srcOdim));
	mout.debug3("src props:", srcImage.properties);
	// std::cerr << src.properties << std::endl;

	mout.debug("dst:", dstImage);

	//dstImage.properties.updateFromCastableMap(srcImage.properties);
	dstImage.properties.importCastableMap(srcImage.properties);
	dstImage.properties.importCastableMap(dstOdim);
	dstImage.setScaling(dstOdim.scaling);
	dstImage.setCoordinatePolicy(srcImage.getCoordinatePolicy());

	// dst.odim.set(odim);
	// mout.debug2("op  odim: " , EncodingODIM(odim) );
	mout.debug("dst odim: ", EncodingODIM(dstOdim));
	mout.debug("dst props: ", dstImage.properties);
	//std::cerr << dst.properties << std::endl;

	// const drain::ValueScaling scaling(srcOdim.scaling.scale, srcOdim.scaling.offset, dstOdim.scaling.scale, dstOdim.scaling.offset);
	const drain::ValueScaling scaling(srcOdim.scaling, dstOdim.scaling); // 2023/04/21
	mout.debug("scaling: ", scaling);

	typedef drain::typeLimiter<double> Limiter;
	Limiter::value_t limit = drain::Type::call<Limiter>(dstOdim.type);

	Image::const_iterator s = srcImage.begin();
	Image::iterator d = dstImage.begin();

	// Tailored long int check by writing and reading a pixel.
	*d = dstOdim.nodata;
	if (static_cast<double>(*d) != dstOdim.nodata){
		mout.note("dstOdim.nodata=", dstOdim.nodata, " -> ", static_cast<double>(*d));
		mout.note("type conversion ", srcOdim.type, " -> ", dstOdim.type);
		mout.warn("type conversion ", dstOdim.type, " ~= ", drain::Type::getTypeChar(dstImage.getType()), " changed the value");
	}
	//mout.debug("dstOdim nodata long-int check " , dstOdim.nodata , " <> " , (long int)(*d = dstOdim.nodata) );

	mout.debug2("src: ", srcImage);
	mout.debug2("dst: ", dstImage);
	double x;
	while (s != srcImage.end()){
		x = *s;

		/// Checks 'undetect' first because 'undetect' and 'nodata' may be the same code
		if (x == srcOdim.undetect)
			*d = dstOdim.undetect;
		else if (x == srcOdim.nodata)
			*d = dstOdim.nodata;
		else {
			/*
			x =  srcOdim.scaleForward(x);
			*d = limit(dstOdim.scaleInverse(x));
			*/
			//  x = dst.odim.scaleInverse(x);
			// *d = dstImage.limit<double>( x );
			// *d = limit( scaling.forward(x) );
			*d = limit( scaling.inv(x) );

		}

		++s;
		++d;
	}

	mout.debug3("finished.");

}



}

#endif /* DATACONVERSIONOP_H_ */

// Rack
// REP // REP // REP // REP
