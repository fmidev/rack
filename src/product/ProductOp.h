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
/*
 * ProductOp.h
 *
 *  Created on: Mar 7, 2011
 *      Author: mpeura
 */

#ifndef RACK_PRODUCT_OP
#define RACK_PRODUCT_OP "ProductOP"

//#include <exception>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>

#include "drain/util/Log.h"
#include "drain/util/RegExp.h"
#include "drain/util/SmartMap.h"
#include "drain/util/TreeOrdered.h"
#include "drain/util/Variable.h"

#include "data/Data.h"
#include "data/DataSelector.h"
#include "data/ODIM.h"
#include "data/ODIMPath.h"
#include "data/ODIMPathTools.h"
#include "data/QuantityMap.h"

#include "hi5/Hi5.h"
//#include "main/rack.h"



#include "ProductBase.h"

namespace rack {

using namespace drain::image;


/// Polar and Cartesian products
/**
 *   \tparam MS - input (source) metadata type: PolarODIM or CartesianODIM.
 *   \tparam MD - output (destination) metadata type: PolarODIM or CartesianODIM.
 */
template <class MS, class MD>
class ProductOp : public ProductBase {

public:

	/// Type of input, that is, source.
	typedef SrcType<MS const> src_t;

	/// Type of output, that is, destination.
	typedef DstType<MD> dst_t;


	/// Default constructor
	inline
	ProductOp(const std::string &name, const std::string & description) : ProductBase(name, description) {
		// dataSelector.path = "^.* / da ta[0-9]+/?$";  /// Contract: in Op's, path filters should be copy to group level.
	}

	inline
	ProductOp(const ProductOp & product) : ProductBase(product){
		std::cerr << __FUNCTION__ << " copy ct " << product.getName() << this->dataSelector << '\n';
	};

	/// The default data parameters for encoding output (the product).
	MD odim;

	/// Returns the primary output quantity (ODIM \c what:quantity , like DBZH)
	/*
	virtual inline
	const std::string & getOutputQuantity() const {
		return odim.quantity; //outputQuantity;
	}
	*/

	/// The ODIM quantity of this product
	/**
	 *  \param defaultQuantity - input quantity or a quantity overriding default output quantity of this product
	 */
	virtual inline
	const std::string & getOutputQuantity(const std::string & inputQuantity = "") const {
		if (!odim.quantity.empty())
			return odim.quantity; //outputQuantity;
		else
			return inputQuantity;
	}


	//void processH5() // see MotionFill
	virtual // HistogramOp
	void processH5(const Hi5Tree &src, Hi5Tree &dst) const;


	/// Traverse the data applicable for this product and create new, processed data (volume or polar product).
	/** The input data is stored in SweepMapSrc, as Map indexed with elevation angles, containing sweep data
	 *  - typically reflectivity data (DBZH) but generally any parameter measured by radar. The applicable data
	 *  is set by selector.quantity, typically in the constructor of a derived class.
	 */
	virtual
	void computeSingleProduct(const DataSetMap<src_t> & srcSweeps, DataSet<dst_t> & dstProduct) const;

	/// Process the data of a single sweep and and write the result to given product
	/**
	 *  Notice:
	 *  - several product data arrays may be generated
	 *  - also quality field(s) may be added
	 *  - overwriting original data may occur, if input data (src) is also given as output (dst); this is desired
	 *    action esp. in anomaly removal
	 */
	virtual
	void processDataSet(const DataSet<src_t > & srcSweep, DataSet<DstType<MD> > & dstProduct) const;

	virtual
	void processData(const Data<src_t > & srcData, Data<dst_t > & dstData) const {
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.unimplemented(__FUNCTION__);
	};


protected:


	/// Sets automagically the suitable dst parameters.
	/**
	 *   Derives geometry for the resulting dst from three sources of parameters:
	 *   the implicit operator parameters this->odim and the two function parameters
	 *   \param srcODIM - metadata of the incoming data
	 *   \param dstODIM - metadata of the result (sometimes partially set).
	 *
	 *   Default implementation as an empty function, because some operators like VerticalProfileOp will not use?
	 */
	virtual
	void setGeometry(const MS & srcODIM, PlainData<dst_t > & dstData) const = 0; // {};


	/// initialises dst data by setting suitable ODIM metadata and geometry.
	/**
	 *
	 */
	virtual inline
	void initDst(const MS & srcODIM, PlainData<dst_t > & dstData) const {

		drain::Logger mout(__FILE__, __FUNCTION__); //REPL this->name+"(VolumeOp<M>)", __FUNCTION__);

		setEncoding(srcODIM, dstData);
		setGeometry(srcODIM, dstData);

		mout.debug("final dstData: " , dstData );
	}


	/// Sets applicable encoding parametes (type, gain, offset, undetect, nodata) for this product.
	/**
	 *   Determines output encoding based on
	 *   - output data (parameters already set in dstData)
	 *   - this->odim (values set in constructor and targetEncoding)
	 *   - targetEncoding
	 *
	 *   \see setGeometry
	 */
	//virtual
	//void setEncodingNEW(PlainData<dst_t > & dstData) const;

	/// Sets applicable encoding parametes (type, gain, offset, undetect, nodata) for this product.
	/**
	 *   Determines output encoding based on
	 *   - input data (srcODIM)
	 *   - output data (parameters already set in dstData)
	 *   - this->odim (values set in constructor and targetEncoding)
	 *   - targetEncoding
	 *
	 *   \see setGeometry
	 */
	virtual
	void setEncoding(const ODIM & srcODIM, PlainData<dst_t > & dstData) const;

	/// Sets applicable encoding parametes (type, gain, offset, undetect, nodata) for this product.
	/**
	 *   Determines output encoding based on
	 *   - output data (parameters already set in dstData)
	 *   - this->odim (values set in constructor and targetEncoding)
	 *   - targetEncoding
	 *
	 *   \see setGeometry
	 *
	 *  \param quantity - if given, applied as a "template", standard quantity to be copied; also odim.type, if empty, is set.
	 *  \param type - if given, odim.type is always changed
	 */
	// void ProductOp<MS,MD>::setEncodingNEW
	void setEncodingNEW(PlainData<dst_t> & dstData, const std::string quantity = "", const std::string type = "") const;

	/*
	static
	void handleEncodingRequest(const std::string & encoding, PlainData<dst_t> & dst){

		drain::Logger mout(__FILE__, __FUNCTION__);

		ProductBase::completeEncoding(dst.odim, encoding);

		if (!dst.odim.type.empty())
			dst.data.setType(dst.odim.type);
		else
			dst.odim.type = (const char)dst.data.getType2();

	}
	*/
};

/**
 *
 *  \param quantity - if given, applied as a "template", standard quantity to be copied; also odim.type, if empty, is set.
 *  \param type - if given, odim.type is always changed
 */

template <class MS, class MD>
void ProductOp<MS,MD>::setEncodingNEW(PlainData<dst_t> & dstData,
		const std::string quantity, const std::string type) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (dstData.odim.quantity.empty())
		dstData.odim.quantity = quantity;

	if (!type.empty())
		dstData.odim.type = type;

	const std::string & qty = quantity.empty() ? dstData.odim.quantity : quantity;

	if (qty.empty()){
		mout.warn("No quantity in metadata or as argument" );
	}

	const QuantityMap & qmap = getQuantityMap();
	qmap.setQuantityDefaults(dstData, qty, dstData.odim.type);
	dstData.odim.updateValues(targetEncoding);
	qmap.setQuantityDefaults(dstData, qty, dstData.odim.type); // type also?
	dstData.odim.updateValues(targetEncoding);
	// update
	dstData.data.setScaling(odim.scaling);

	/// This applies always.
	// dstData.odim.product = odim.product; ?
}

/*
template <class MS, class MD>
void ProductOp<MS,MD>::setEncodingNEW(PlainData<dst_t> & dst, const std::string quantity = "", const std::string type = "") const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (odim.quantity.empty())
		odim.quantity = quantity;

	if (!type.empty())
		odim.type = type;

	const std::string & qty = quantity.empty() ? odim.quantity : quantity;

	const QuantityMap & qmap = getQuantityMap();
	qmap.setQuantityDefaults(dst, qty, odim.type);
	dst.odim.updateValues(targetEncoding);
	qmap.setQuantityDefaults(dst, qty, dst.odim.type); // type also?
	dst.odim.updateValues(targetEncoding);
	// update
	dst.data.setScaling(dst.odim.scaling);

	/// This applies always.
	// dst.odim.product = odim.product; ?
}
*/

template <class MS, class MD>
void ProductOp<MS,MD>::setEncoding(const ODIM & inputODIM, PlainData<dst_t> & dst) const {

	ProductBase::applyODIM(dst.odim, this->odim);
	ProductBase::applyODIM(dst.odim, inputODIM, true);  // New. Use defaults if still unset
	ProductBase::completeEncoding(dst.odim, this->targetEncoding);

	// Set actual scaling up to date with metadata)
	dst.data.setScaling(dst.odim.scaling);
	/// This applies always.
	// dst.odim.product = odim.product;
}


template <class MS, class MD>  // copied from VolumeOp::processVolume
void ProductOp<MS,MD>::processH5(const Hi5Tree &src, Hi5Tree &dst) const {

	//drain::Logger mout(__FILE__, __FUNCTION__); //REPL this->name+"(VolumeOp<M>)", __FUNCTION__);
	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("start" );
	mout.debug3(*this );
	mout.debug2("DataSelector: "  , this->dataSelector );

	// Step 1: collect sweeps (/datasetN/)
	//DataSetMap<src_t> sweeps;
	DataSetMap<src_t> sweeps;

	/// Usually, the operator does not need groups sorted by elevation.
	mout.debug3("collect the applicable paths");
	ODIMPathList dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	int index = 0;


	mout.unimplemented("No index generator defined!");
	// NEW
	this->dataSelector.getPaths(src, dataPaths); //, ODIMPathElem::DATASET);
	mout.debug3("populate the dataset map, paths=" , dataPaths.size() );
	//for (ODIMPathList::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){
	for (const ODIMPath & path: dataPaths){
		mout.debug3("add: " , index , '\t' , path  );
		sweeps.insert(typename DataSetMap<src_t>::value_type("indexxx", DataSet<src_t>(src(path), drain::RegExp(this->dataSelector.getQuantity()) )));  // Something like: sweeps[elangle] = src[parent] .
	}


	// OLD
	/*
	this->dataSelector.getPaths(src, dataPaths, ODIMPathElem::DATA);

	mout.debug3("populate the dataset map, paths=" , dataPaths.size() );
	std::set<ODIMPath> parents;

	for (ODIMPathList::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

		ODIMPath parent = *it;
		parent.pop_back();

		mout.debug3("check "  , *it );

		if (parents.find(parent) == parents.end()){
			mout.debug3("add " , parent  , "=>" , index );
			// kludge (index ~ elevation)
			sweeps.insert(typename DataSetMap<src_t>::value_type(index, DataSet<src_t>(src(parent), drain::RegExp(this->dataSelector.quantity) )));  // Something like: sweeps[elangle] = src[parent] .
			//elangles << elangle;
		}
	}
	*/


	mout.debug2("DataSets: "  , sweeps.size() );


	// Copy metadata from the input volume (note that dst may have been cleared above)
	dst["what"]  = src["what"];
	dst["where"] = src["where"];
	dst["how"]   = src["how"];
	dst["what"].data.attributes["object"] = this->odim.object;
	// odim.copyToRoot(dst); NO! Mainly overwrites original data. fgrep 'declare(rootAttribute' odim/*.cpp

	//ODIMPath dataSetPath;
	ODIMPathElem parent(ODIMPathElem::DATASET, 1); // /dataset1
	ODIMPathElem child(ODIMPathElem::DATA, 1); // /dataset1

	mout.note("appendResults path: " ,  ProductBase::appendResults );
	if (ProductBase::appendResults.getType() == ODIMPathElem::DATASET){
		ODIMPathTools::getNextChild(dst, parent);
	}

	ODIMPathTools::getNextChild(dst[parent], child);

	mout.note("storing product in path: " ,  parent , '|' , child );
	//mout.debug3("storing product in path: "  , dataSetPath );

	Hi5Tree & dstProduct = dst[parent][child]; // (dataSetPath);
	DataSet<dst_t> dstProductDataset(dstProduct); // PATH


	/// Main operation
	this->computeSingleProduct(sweeps, dstProductDataset);

	ProductBase::setODIMsoftwareVersion(dstProduct["how"].data.attributes);
	//drain::VariableMap & how = dstProduct["how"].data.attributes;
	//how["software"] = __RACK__;
	//how["sw_version"] = __RACK_VERSION__;
	// how["elangles"] = elangles;  // This service could be lower in hierarchy (but for PseudoRHI and pCappi ok here)

	//DataTools::updateInternalAttributes(dstProduct);
	//drain::VariableMap & what = dst[dataSetPath]["what"].data.attributes;
	//what["source"] = src["what"].data.attributes["source"];
	//what["object"] = odim.object;

}


//template <class M>
template <class MS, class MD>
void ProductOp<MS,MD>::computeSingleProduct(const DataSetMap<src_t > & src, DataSet<DstType<MD> > & dstProduct) const {

	drain::Logger mout(__FILE__, __FUNCTION__); //REPL this->name+"(VolumeOp<M>)", __FUNCTION__);
	mout.debug3("start" );

	if (src.size() == 0)
		mout.warn("no data" );

	//for (typename DataSetMap<src_t >::const_iterator it = src.begin(); it != src.end(); ++it) {
	for (const auto & entry: src) {
		mout.debug3("calling processDataSet for elev=", entry.first,  " #datasets=", entry.second.size());
		processDataSet(entry.second, dstProduct);
		// TODO: detect first init?
		// mout.warn("OK" );
	}

	/// Pick main product data and assign (modified) odim data
	// mout.warn("getFirst data" );
	//@ Data<DstType<MD> > & dstData = dstProduct.getFirstData(); // main data
	//@ dstProduct.updateTree(dstData.odim);
	// mout.warn("end" );
}


//template <class M>
template <class MS, class MD>
void ProductOp<MS,MD>::processDataSet(const DataSet<src_t > & srcSweep, DataSet<DstType<MD> > & dstProduct) const {

	//drain::Logger mout(__FILE__, __FUNCTION__); //REPL this->name+"(ProductOp<MS,MD>)", __FUNCTION__);
	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("start" );

	const Data<src_t > & srcData = srcSweep.getFirstData();

	mout.debug("target quantity: " , odim.quantity );

	// NEW 2020/06
	const std::string & quantity = !odim.quantity.empty() ? odim.quantity : srcData.odim.quantity;
	Data<DstType<MD> > & dstData =  dstProduct.getData(quantity);
	dstData.odim.quantity = quantity;

	// OLD
	// Data<DstType<MD> > & dstData = !odim.quantity.empty() ? dstProduct.getData(odim.quantity) : dstProduct.getFirstData();
	//if (dstData.odim.quantity.empty())
	//	dstData.odim.quantity = odim.quantity;

	//mout.debug("calling setEncoding" );
	//setEncoding(srcData.odim, dstData.odim);
	this->initDst(srcData.odim, dstData);

	//mout.warn("calling processData" );
	processData(srcData, dstData);
	//mout.warn("updateTree" );
	//@ dstData.updateTree();
}





}  // namespace rack


#endif /* RACKOP_H_ */

// Rack
 // REP
 // REP // REP // REP // REP
