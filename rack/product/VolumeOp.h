/*

    Copyright 2001 - 2016  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */
/*
 * ProductOp.h
 *
 *  Created on: Mar 7, 2011
 *      Author: mpeura
 */

#ifndef RACKOP_H_
#define RACKOP_H_


//#include <drain/util/DataScaling.h>
//#include <drain/util/StringMapper.h>
//#include <drain/util/Variable.h>
//#include <drain/util/Tree.h>
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
// TODO: generalize for Cart
template <class M>
class VolumeOp : public ProductOp<const PolarODIM, M> {

public:

	VolumeOp(const std::string & name, const std::string &description="") : ProductOp<const PolarODIM, M>(name, description){
	};

	~VolumeOp(){};


	/// Traverse through given volume and create new, processed data (volume or polar product).
	/**
	 *  This default implementation converts the volume to DataSetMap<PolarSrc>, creates an instance of
	 *  DataSet<DstType<M> >
	 *  and calls processDataSets().
	 */
	virtual
	void processVolume(const HI5TREE &src, HI5TREE &dst) const;



protected:


	//static
	//void handleEncodingRequest(const std::string & encoding, PlainData<DstType<M> > & dst);





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
void VolumeOp<M>::handleEncodingRequest(const std::string & encoding, PlainData<DstType<M> > & dst){

	drain::Logger mout("VolumeOp<M>::", __FUNCTION__);

	ProductOp<PolarODIM,M>::handleEncodingRequest(dst.odim, encoding);

	if (!dst.odim.type.empty())
		dst.data.setType(dst.odim.type);
	else
		dst.odim.type = (const char)dst.data.getType2();

}
*/


template <class M>
void VolumeOp<M>::processVolume(const HI5TREE &src, HI5TREE &dst) const {

	drain::Logger mout(this->name+"(VolumeOp<M>)", __FUNCTION__);

	mout.debug() << "start" << mout.endl;
	mout.debug(2) << *this << mout.endl;
	mout.debug(1) << "DataSelector: "  << this->dataSelector << mout.endl;

	// Step 1: collect sweeps (/datasetN/)
	DataSetMap<PolarSrc> sweeps;

	/// Usually, the operator does not need groups sorted by elevation.
	mout.debug(2) << "collect the applicable paths"  << mout.endl;
	std::list<std::string> dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	DataSelector::getPaths(src,  this->dataSelector, dataPaths);

	mout.debug(2) << "populate the dataset map, paths=" << dataPaths.size() << mout.endl;
	drain::Variable elangles(typeid(double));
	for (std::list<std::string>::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

		mout.debug(2) << "elangles (this far> "  << elangles << mout.endl;

		const std::string parent = DataTools::getParent(*it);
		const double elangle = src(parent)["where"].data.attributes["elangle"];  // PATH

		mout.debug(2) << "check "  << *it << mout.endl;

		if (sweeps.find(elangle) == sweeps.end()){
			mout.debug(2) << "add "  << elangle << ':'  << parent << mout.endl;
			sweeps.insert(DataSetMap<PolarSrc>::value_type(elangle, DataSet<PolarSrc>(src(parent), drain::RegExp(this->dataSelector.quantity) )));  // Something like: sweeps[elangle] = src[parent] .
			elangles << elangle;
		}
	}

	// Copy metadata from the input volume (note that dst may have been cleared above)
	dst["what"]  = src["what"];
	dst["where"] = src["where"];
	dst["how"]   = src["how"];
	dst["what"].data.attributes["object"] = this->odim.object;
	// odim.copyToRoot(dst); NO! Mainly overwrites original data. fgrep 'declare(rootAttribute' odim/*.cpp

	/*
	std::string dataSetPath = "dataset1";
	if (ProductBase::appendResults.getType() != BaseODIM::NONE)  // empty()) // data or dataset
		DataSelector::getNextOrdinalPath(dst, ProductBase::appendResults+"[0-9]+/?$", dataSetPath);
	*/
	ODIMPathElem dataSetPath(BaseODIM::DATASET, 1);
	//if (!DataTools::getNextDescendant(dst, ProductBase::appendResults.getType(), dataSetPath))

	if (ProductBase::appendResults.is(BaseODIM::DATASET)){
		if (ProductBase::appendResults.getIndex()){
			dataSetPath.index = ProductBase::appendResults.getIndex();
		}
		else {
			DataTools::getNextChild(dst, dataSetPath);
		}
	}
	//++dataSetPath.index;

	//mout.warn() << "FAILED: "  << dataSetPath << mout.endl;
		//dataSetPath.push_back(ODIMPathElem(BaseODIM::DATASET, 1));


	mout.debug() << "storing product in path: "  << dataSetPath << mout.endl;

	HI5TREE & dstProduct = dst[dataSetPath];
	DataSet<DstType<M> > dstProductDataset(dstProduct); // PATH

	drain::VariableMap & how = dstProduct["how"].data.attributes;

	/// Main operation
	this->processDataSets(sweeps, dstProductDataset);

	//
	how["software"] = __RACK__;
	how["sw_version"] = __RACK_VERSION__;
	how["elangles"] = elangles;  // This service could be lower in hierarchy (but for PseudoRHI and pCappi ok here)

	//drain::VariableMap & what = dst[dataSetPath]["what"].data.attributes;
	//what["source"] = src["what"].data.attributes["source"];
	//what["object"] = odim.object;

}


}  // namespace rack


#endif /* RACKOP_H_ */
