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

#ifndef RACKOP_H_
#define RACKOP_H_


#include <drain/util/DataScaling.h>
#include <drain/util/StringMapper.h>
#include <drain/util/Variable.h>
#include <drain/util/Tree.h>
#include <drain/util/ReferenceMap.h>



//#include "hi5/H52Image.h"
#include "main/rack.h"
#include "hi5/Hi5.h"
#include "data/ODIM.h"
#include "data/DataSelector.h"
#include "data/Data.h"
#include "data/Quantity.h" // NEW

#include "ProductOp.h" // NEW

namespace rack {

using namespace drain::image;

/// Base class for radar data processors.

/// Base class for radar data processors.
/** Input and output as HDF5 data, which has been converted to internal structure, drain::Tree<NodeH5>.
 *
 *  Basically, there are two kinds of polar processing
 *  - Cumulative: the volume is traversed, each sweep contributing to a single accumulation array, out of which the product layer(s) is extracted.
 *  - Sequential: each sweep generates new layer (/dataset) in the product; typically, the lowest only is applied.
 *
 *  TODO: Raise to RackOp
 */





/**
 *   \tparam M - ODIM type corresponding to products type (polar, vertical)
 */
template <class M>
class VolumeOp : public ProductOp {

public:

	VolumeOp(const std::string & name, const std::string &description="") : ProductOp(name, description){
	};

	~VolumeOp(){};

	/// The default data parameters for encoding.
	M odim;

	/// Traverse through given volume and create new, processed data (volume or polar product).
	/**
	 *  This default implementation converts the volume to DataSetSrcMap, creates an instance of
	 *  DataSetDst<DstType<M> >
	 *  and calls processDataSets().
	 */
	virtual
	void processVolume(const HI5TREE &src, HI5TREE &dst) const;

	/// Traverse the data applicable for this product and create new, processed data (volume or polar product).
	/** The input data is stored in SweepMapSrc, as Map indexed with elevation angles, containing sweep data
	 *  - typically reflectivity data (DBZH) but generally any parameter measured by radar. The applicable data
	 *  is set by selector.quantity, typically in the constructor of a derived class.
	 */
	virtual
	void processDataSets(const DataSetSrcMap & srcSweeps, DataSetDst<DstType<M> > & dstProduct) const;

	/// Process the data of a single sweep and and write the result to given product
	/**
	 *  Notice:
	 *  - several product data arrays may be generated
	 *  - also quality field(s) may be added
	 *  - overwriting original data may occur, if input data (src) is also given as output (dst); this is desired
	 *    action esp. in anomaly removal
	 */
	virtual
	void processDataSet(const DataSetSrc<PolarSrc> & srcSweep, DataSetDst<DstType<M> > & dstProduct) const;

	virtual
	void processData(const Data<PolarSrc> & srcData, Data<DstType<M> > & dstData) const {
		drain::MonitorSource mout(name, __FUNCTION__);
		mout.warn() << "not implemented" << mout.endl;
	};

	// template <class OD>
	//static 	void handleEncodingRequest(const std::string & encoding, OD & dstODIM);

protected:


	static
	void handleEncodingRequest(const std::string & encoding, PlainData<DstType<M> > & dst);


	/// initialises dst data by setting suitable ODIM metadata and geometry.
	/**
	 *
	 */
	virtual
	inline
	//void initDst(const PolarODIM & srcODIM, Data<DstType<M> > & dstData) const {
	void initDst(const PolarODIM & srcODIM, PlainData<DstType<M> > & dstData) const {

		drain::MonitorSource mout(name+"(VolumeOp<M>)", __FUNCTION__);

		setEncoding(srcODIM, dstData);
		setGeometry(srcODIM, dstData);

		mout.debug() << "final dstData: " << dstData << mout.endl;
	}


	///
	/**
	 *   \return - tree, because there is no (automatic) conversion form DataSetDst to DataSetSrc
	 */
	//const HI5TREE & getNormalizedData(const DataSetSrc<SrcType<M> > & srcDataSet, DataSetDst< DstType<M> > & dstDataSet, const std::string & quantity) const;

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
	void setEncoding(const ODIM & srcODIM, PlainData<DstType<M> > & dstData) const;




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
	void setGeometry(const PolarODIM & srcODIM, PlainData<DstType<M> > & dstData) const = 0; // {};

	/*
	/// Copy nbins, nrays, rscale
	inline
	void copyPolarGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		dstData.odim.nbins  = srcODIM.nbins;
		dstData.odim.nrays  = srcODIM.nrays;
		dstData.odim.rscale = srcODIM.rscale;
		dstData.data.setGeometry(dstData.odim.nbins, dstData.odim.nrays);
		if (!dstData.odim.type.empty()){
			dstData.data.setType(dstData.odim.type.at(0));
		}
		else {
			// warn type not set?
		}
	};
	*/


};

/*
template <class M>
const QuantityMap & VolumeOp<M>::quantityMap(getQuantityMap());
*/

template <class M>
void VolumeOp<M>::setEncoding(const ODIM & inputODIM, PlainData<DstType<M> > & dst) const {

	//ProductOp::applyDefaultODIM(dst.odim, odim);
	ProductOp::applyODIM(dst.odim, odim);

	//ProductOp::applyODIM(dst.odim, inputODIM);
	ProductOp::applyODIM(dst.odim, inputODIM, true);  // New. Use defaults if still unset


	ProductOp::handleEncodingRequest(dst.odim, encodingRequest);

	/// This applies always.
	//dst.odim.product = odim.product;
}



template <class M>
void VolumeOp<M>::handleEncodingRequest(const std::string & encoding, PlainData<DstType<M> > & dst){

	drain::MonitorSource mout("VolumeOp<M>::", __FUNCTION__);

	ProductOp::handleEncodingRequest(dst.odim, encoding);

	if (!dst.odim.type.empty())
		dst.data.setType(dst.odim.type);
	else
		dst.odim.type = (const char)dst.data.getType2();

}



template <class M>
void VolumeOp<M>::processVolume(const HI5TREE &src, HI5TREE &dst) const {

	drain::MonitorSource mout(name+"(VolumeOp<M>)", __FUNCTION__);

	mout.debug() << "start" << mout.endl;
	mout.debug(2) << *this << mout.endl;
	mout.debug(1) << "DataSelector: "  << dataSelector << mout.endl;

	// Step 1: collect sweeps (/datasetN/)
	DataSetSrcMap sweeps;

	/// Usually, the operator does not need groups sorted by elevation.
	mout.debug(2) << "collect the applicable paths"  << mout.endl;
	std::list<std::string> dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	DataSelector::getPaths(src,  dataSelector, dataPaths);

	mout.debug(2) << "populate the dataset map, paths=" << dataPaths.size() << mout.endl;
	drain::Variable elangles(typeid(double));
	for (std::list<std::string>::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

		mout.debug(2) << "elangles (this far> "  << elangles << mout.endl;

		const std::string parent = DataSelector::getParent(*it);
		const double elangle = src(parent)["where"].data.attributes["elangle"];  // PATH

		mout.debug(2) << "check "  << *it << mout.endl;

		if (sweeps.find(elangle) == sweeps.end()){
			mout.debug(2) << "add "  << elangle << ':'  << parent << mout.endl;
			sweeps.insert(DataSetSrcMap::value_type(elangle, DataSetSrc<PolarSrc>(src(parent), drain::RegExp(dataSelector.quantity) )));  // Something like: sweeps[elangle] = src[parent] .
			elangles << elangle;
		}
	}

	// Copy metadata from the input volume (note that dst may have been cleared above)
	dst["what"]  = src["what"];
	dst["where"] = src["where"];
	dst["how"]   = src["how"];
	dst["what"].data.attributes["object"] = odim.object;
	// odim.copyToRoot(dst); NO! Mainly overwrites original data. fgrep 'declare(rootAttribute' odim/*.cpp

	std::string dataSetPath = "dataset1";
	if (ProductOp::appendResults)
		DataSelector::getNextOrdinalPath(dst, "dataset[0-9]+/?$", dataSetPath);

	mout.debug(2) << "storing product in path: "  << dataSetPath << mout.endl;

	HI5TREE & dstProduct = dst(dataSetPath);
	DataSetDst<DstType<M> > dstProductDataset(dstProduct); // PATH

	drain::VariableMap & how = dstProduct["how"].data.attributes;

	/// Main operation
	processDataSets(sweeps, dstProductDataset);

	//
	how["software"] = __RACK__;
	how["sw_version"] = __RACK_VERSION__;
	how["elangles"] = elangles;  // This service could be lower in hierarchy (but for PseudoRHI and pCappi ok here)

	//drain::VariableMap & what = dst[dataSetPath]["what"].data.attributes;
	//what["source"] = src["what"].data.attributes["source"];
	//what["object"] = odim.object;

}


template <class M>
void VolumeOp<M>::processDataSets(const DataSetSrcMap & src, DataSetDst<DstType<M> > & dstProduct) const {

	drain::MonitorSource mout(this->name+"(VolumeOp<M>)", __FUNCTION__);
	mout.debug(2) << "start" << mout.endl;

	if (src.size() == 0)
		mout.warn() << "no data" << mout.endl;

	for (DataSetSrcMap::const_iterator it = src.begin(); it != src.end(); ++it) {

		mout.debug(2) << "calling processDataSet for elev=" << it->first << " #datasets=" << it->second.size() << mout.endl;
		processDataSet(it->second, dstProduct);
		// TODO: detect first init?
		// mout.warn() << "OK" << mout.endl;
	}

	/// Pick main product data and assign (modified) odim data
	// mout.warn() << "getFirst data" << mout.endl;
	Data<DstType<M> > & dstData = dstProduct.getFirstData(); // main data
	/// mout.warn() << "updateTree " << mout.endl;
	dstProduct.updateTree(dstData.odim);
	// mout.warn() << "end" << mout.endl;
}


template <class M>
void VolumeOp<M>::processDataSet(const DataSetSrc<PolarSrc> & srcSweep, DataSetDst<DstType<M> > & dstProduct) const {

	drain::MonitorSource mout(name+"(VolumeOp<M>)", __FUNCTION__);

	mout.debug() << "start" << mout.endl;

	const Data<PolarSrc> & srcData = srcSweep.getFirstData();

	Data<DstType<M> > & dstData = !odim.quantity.empty() ? dstProduct.getData(odim.quantity) : dstProduct.getFirstData();

	if (dstData.odim.quantity.empty())
		dstData.odim.quantity = odim.quantity;

	//mout.debug() << "calling setEncoding" << mout.endl;
	//setEncoding(srcData.odim, dstData.odim);
	initDst(srcData.odim, dstData);

	//mout.warn() << "calling processData" << mout.endl;
	processData(srcData, dstData);
	//mout.warn() << "updateTree" << mout.endl;
	dstData.updateTree();
}


}  // namespace rack


#endif /* RACKOP_H_ */

// Rack
