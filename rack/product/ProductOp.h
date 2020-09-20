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


#include <data/Data.h>
#include <data/DataSelector.h>
#include <data/ODIM.h>
#include <data/ODIMPath.h>
#include <hi5/Hi5.h>
#include <main/rack.h>
#include "drain/util/Log.h"
#include "drain/util/RegExp.h"
#include "drain/util/SmartMap.h"
#include "drain/util/Tree.h"
#include "drain/util/Variable.h"
#include <exception>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>


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
	ProductOp(const std::string &name, const std::string & description) : ProductBase(name, description) {
		// dataSelector.path = "^.* / da ta[0-9]+/?$";  /// Contract: in Op's, path filters should be copy to group level.
	}

	/// The default data parameters for encoding output (the product).
	MD odim;

	/// Returns the primary output quantity (ODIM \c what:quantity , like DBZH)
	virtual inline
	const std::string & getOutputQuantity() const {
		return odim.quantity; //outputQuantity;
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
	void processDataSets(const DataSetMap<src_t> & srcSweeps, DataSet<dst_t> & dstProduct) const;

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
		drain::Logger mout(__FUNCTION__, __FILE__);
		mout.warn() << "not implemented" << mout.endl;
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

		drain::Logger mout(__FUNCTION__, __FILE__); //REPL this->name+"(VolumeOp<M>)", __FUNCTION__);

		setEncoding(srcODIM, dstData);
		setGeometry(srcODIM, dstData);

		mout.debug() << "final dstData: " << dstData << mout.endl;
	}


	/// Sets applicable encoding parametes (type, gain, offset, undetect, nodata) for this product.
	/**
	 *   Determines output encoding based on
	 *   - input data (srcODIM)
	 *   - output data (parameters already set in dstData)
	 *   - this->odim (values set in constructor and encodingRequest)
	 *   - encodingRequest
	 *
	 *   \see setGeometry
	 */
	virtual
	void setEncoding(const ODIM & srcODIM, PlainData<dst_t > & dstData) const;

	/*
	static
	void handleEncodingRequest(const std::string & encoding, PlainData<dst_t> & dst){

		drain::Logger mout(__FUNCTION__, __FILE__);

		ProductBase::completeEncoding(dst.odim, encoding);

		if (!dst.odim.type.empty())
			dst.data.setType(dst.odim.type);
		else
			dst.odim.type = (const char)dst.data.getType2();

	}
	*/
};

// template <class M> void VolumeOp<M>::
template <class MS, class MD>
void ProductOp<MS,MD>::setEncoding(const ODIM & inputODIM, PlainData<dst_t > & dst) const {

	//ProductOp::applyDefaultODIM(dst.odim, odim);
	ProductBase::applyODIM(dst.odim, this->odim);

	//ProductOp::applyODIM(dst.odim, inputODIM);
	ProductBase::applyODIM(dst.odim, inputODIM, true);  // New. Use defaults if still unset


	ProductBase::completeEncoding(dst.odim, this->encodingRequest);

	// NEW 2020/06
	dst.data.setScaling(dst.odim.scale, dst.odim.offset);
	/// This applies always.
	//dst.odim.product = odim.product;
}


template <class MS, class MD>  // copied from VolumeOp::processVolume
void ProductOp<MS,MD>::processH5(const Hi5Tree &src, Hi5Tree &dst) const {

	//drain::Logger mout(__FUNCTION__, __FILE__); //REPL this->name+"(VolumeOp<M>)", __FUNCTION__);
	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.debug() << "start" << mout.endl;
	mout.debug(2) << *this << mout.endl;
	mout.debug(1) << "DataSelector: "  << this->dataSelector << mout.endl;

	// Step 1: collect sweeps (/datasetN/)
	DataSetMap<src_t> sweeps;

	/// Usually, the operator does not need groups sorted by elevation.
	mout.debug(2) << "collect the applicable paths"  << mout.endl;
	ODIMPathList dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	int index = 0;

	// NEW
	this->dataSelector.getPaths3(src, dataPaths); //, ODIMPathElem::DATASET);
	mout.debug(2) << "populate the dataset map, paths=" << dataPaths.size() << mout.endl;
	for (ODIMPathList::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){
		mout.debug(2) << "add: " << index << '\t' << *it  << mout.endl;
		sweeps.insert(typename DataSetMap<src_t>::value_type(index, DataSet<src_t>(src(*it), drain::RegExp(this->dataSelector.quantity) )));  // Something like: sweeps[elangle] = src[parent] .
	}


	// OLD
	/*
	this->dataSelector.getPaths(src, dataPaths, ODIMPathElem::DATA);

	mout.debug(2) << "populate the dataset map, paths=" << dataPaths.size() << mout.endl;
	std::set<ODIMPath> parents;

	for (ODIMPathList::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

		ODIMPath parent = *it;
		parent.pop_back();

		mout.debug(2) << "check "  << *it << mout.endl;

		if (parents.find(parent) == parents.end()){
			mout.debug(2) << "add " << parent  << "=>" << index << mout.endl;
			// kludge (index ~ elevation)
			sweeps.insert(typename DataSetMap<src_t>::value_type(index, DataSet<src_t>(src(parent), drain::RegExp(this->dataSelector.quantity) )));  // Something like: sweeps[elangle] = src[parent] .
			//elangles << elangle;
		}
	}
	*/


	mout.debug(1) << "DataSets: "  << sweeps.size() << mout.endl;


	// Copy metadata from the input volume (note that dst may have been cleared above)
	dst["what"]  = src["what"];
	dst["where"] = src["where"];
	dst["how"]   = src["how"];
	dst["what"].data.attributes["object"] = this->odim.object;
	// odim.copyToRoot(dst); NO! Mainly overwrites original data. fgrep 'declare(rootAttribute' odim/*.cpp

	//ODIMPath dataSetPath;
	ODIMPathElem parent(ODIMPathElem::DATASET, 1); // /dataset1
	ODIMPathElem child(ODIMPathElem::DATA, 1); // /dataset1

	mout.note() << "appendResults path: " <<  ProductBase::appendResults << mout.endl;
	if (ProductBase::appendResults.getType() == ODIMPathElem::DATASET){
		DataSelector::getNextChild(dst, parent);
	}

	DataSelector::getNextChild(dst[parent], child);

	mout.note() << "storing product in path: " <<  parent << '|' << child << mout.endl;
	//mout.debug(2) << "storing product in path: "  << dataSetPath << mout.endl;

	Hi5Tree & dstProduct = dst[parent][child]; // (dataSetPath);
	DataSet<dst_t> dstProductDataset(dstProduct); // PATH

	drain::VariableMap & how = dstProduct["how"].data.attributes;

	/// Main operation
	this->processDataSets(sweeps, dstProductDataset);

	//
	how["software"] = __RACK__;
	how["sw_version"] = __RACK_VERSION__;
	// how["elangles"] = elangles;  // This service could be lower in hierarchy (but for PseudoRHI and pCappi ok here)

	//DataTools::updateInternalAttributes(dstProduct);
	//drain::VariableMap & what = dst[dataSetPath]["what"].data.attributes;
	//what["source"] = src["what"].data.attributes["source"];
	//what["object"] = odim.object;

}


//template <class M>
template <class MS, class MD>
void ProductOp<MS,MD>::processDataSets(const DataSetMap<src_t > & src, DataSet<DstType<MD> > & dstProduct) const {

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL this->name+"(VolumeOp<M>)", __FUNCTION__);
	mout.debug(2) << "start" << mout.endl;

	if (src.size() == 0)
		mout.warn() << "no data" << mout.endl;

	for (typename DataSetMap<src_t >::const_iterator it = src.begin(); it != src.end(); ++it) {

		mout.debug(2) << "calling processDataSet for elev=" << it->first << " #datasets=" << it->second.size() << mout.endl;
		processDataSet(it->second, dstProduct);
		// TODO: detect first init?
		// mout.warn() << "OK" << mout.endl;
	}

	/// Pick main product data and assign (modified) odim data
	// mout.warn() << "getFirst data" << mout.endl;
	//@ Data<DstType<MD> > & dstData = dstProduct.getFirstData(); // main data
	//@ dstProduct.updateTree(dstData.odim);
	// mout.warn() << "end" << mout.endl;
}


//template <class M>
template <class MS, class MD>
void ProductOp<MS,MD>::processDataSet(const DataSet<src_t > & srcSweep, DataSet<DstType<MD> > & dstProduct) const {

	//drain::Logger mout(__FUNCTION__, __FILE__); //REPL this->name+"(ProductOp<MS,MD>)", __FUNCTION__);
	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.debug() << "start" << mout.endl;

	const Data<src_t > & srcData = srcSweep.getFirstData();

	mout.debug() << "target quantity: " << odim.quantity << mout.endl;

	// NEW 2020/06
	const std::string & quantity = !odim.quantity.empty() ? odim.quantity : srcData.odim.quantity;
	Data<DstType<MD> > & dstData =  dstProduct.getData(quantity);
	dstData.odim.quantity = quantity;

	// OLD
	// Data<DstType<MD> > & dstData = !odim.quantity.empty() ? dstProduct.getData(odim.quantity) : dstProduct.getFirstData();
	//if (dstData.odim.quantity.empty())
	//	dstData.odim.quantity = odim.quantity;

	//mout.debug() << "calling setEncoding" << mout.endl;
	//setEncoding(srcData.odim, dstData.odim);
	this->initDst(srcData.odim, dstData);

	//mout.warn() << "calling processData" << mout.endl;
	processData(srcData, dstData);
	//mout.warn() << "updateTree" << mout.endl;
	//@ dstData.updateTree();
}





}  // namespace rack


#endif /* RACKOP_H_ */

// Rack
 // REP
 // REP // REP // REP // REP
