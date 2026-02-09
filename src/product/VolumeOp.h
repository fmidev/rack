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


#include <drain/util/ReferenceMap.h>

#include "data/DataSelector.h"
#include "data/Data.h"
#include "data/ODIM.h"
#include "data/ODIMPathTools.h"
#include "data/Quantity.h" // NEW

#include "hi5/Hi5.h"
#include "hi5/Hi5Write.h" // debugging


#include "radar/PolarSector.h"
#include "RadarProductOp.h"

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
class VolumeOp : public RadarProductOp<const PolarODIM, M> {

public:

	VolumeOp(const std::string & name, const std::string &description="") : RadarProductOp<const PolarODIM, M>(name, description){
	};

	virtual inline
	~VolumeOp(){};


	/// Traverse through given volume and create new, processed data (volume or polar product).
	/**
	 *  This default implementation converts the volume to DataSetMap<PolarSrc>, creates an instance of
	 *  DataSet<DstType<M> >
	 *  and calls computeSingleProduct().
	 *
	 *  \return /dataset<n> group of the HDF5 tree.
	 */
	virtual
	Hi5Tree & processVolume(const Hi5Tree &srcRoot, Hi5Tree &dstRoot) const final;

	virtual inline
	void setPolarSelector(const PolarSelector & ps){
		// polarSelector = ps;
		if (ps.azimuth.range.empty() && ps.radius.range.empty()){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.info(this->getName(), ": no support for polar selector");
		}
	}

protected:

	// PolarSelector polarSelector;


};



template <class M>
Hi5Tree & VolumeOp<M>::processVolume(const Hi5Tree &srcRoot, Hi5Tree &dstRoot) const{

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("start" );
	mout.debug3(*this );
	mout.debug2("DataSelector: "  , this->dataSelector );

	// Step 1: collect sweeps (/datasetN/)
	DataSetMap<PolarSrc> sweeps;

	/// Usually, the operator does not need groups sorted by elevation.
	mout.debug3("collect the applicable paths"  );
	ODIMPathList dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	//this->dataSelector.getPaths(src, dataPaths, ODIMPathElem::DATASET); // RE2

	this->dataSelector.getPaths(srcRoot, dataPaths);

	if (dataPaths.empty()){
		if (!this->dataSelector.getQuantitySelector().test("HGHT")){
			mout.experimental<LOG_INFO>("no dataset's selected, but height=[HGHT] requested, continuing");
		}
		else {
			mout.warn("no dataset's selected");
		}
	}
	else {
		mout.debug3("populate the dataset map, paths=" , dataPaths.size() );
	}

	ODIM dstODIM;

	for (ODIMPath & path: dataPaths){

		if (!path.front().is(ODIMPathElem::DATASET)){
			path.pop_front();
			if (path.empty()){
				mout.warn("odd 1st path elem (..), with selector: ", this->dataSelector);
				continue;
			}
		}

		const ODIMPathElem & parentElem = path.front();

		if (!parentElem.is(ODIMPathElem::DATASET)){
			mout.warn("path does not start with /dataset.. :", path, ", with selector: ", this->dataSelector);
			continue;
		}

		// mout.debug3("elangles (this far> "  , elangles );
		const Hi5Tree & srcDataSet = srcRoot(parentElem);

		// const double elangle = srcDataSet[ODIMPathElem::WHERE].data.attributes["elangle"];  // PATH
		// mout.deprecating("no more testing ", parent, ", elangle=", elangle, ':', srcDataSet.data.dataSet);


		const ODIM srcODIM(srcDataSet.data.image);
		std::string datetime = srcODIM.startdate + srcODIM.starttime;

		dstODIM.updateLenient(srcODIM);
		mout.attention<LOG_DEBUG+1>("start=", dstODIM.startdate, '+', dstODIM.starttime, " nominal<", datetime, "> end==", dstODIM.enddate, '+', dstODIM.endtime);

		if (sweeps.find(datetime) == sweeps.end()){
			mout.debug2("adding time=", datetime, ':', parentElem);
			// Consider removing RegExp form datasets?
			//sweeps.insert(DataSetMap<PolarSrc>::value_type(datetime, DataSet<PolarSrc>(srcDataSet, drain::RegExp(this->dataSelector.getQuantity()))));
			sweeps.insert(DataSetMap<PolarSrc>::value_type(datetime, DataSet<PolarSrc>(srcDataSet, this->dataSelector.getQuantitySelector())));
		}
		else {
			// mout.warn("datetime =", datetime, " already added?, skipping ",  parent);
		}

		/*
		if (sweeps.find(elangle) == sweeps.end()){
			mout.debug3("add "  , elangle , ':'  , parent , " quantity RegExp:" , this->dataSelector.quantity );
			sweeps.insert(DataSetMap<PolarSrc>::value_type(elangle, DataSet<PolarSrc>(srcDataSet, drain::RegExp(this->dataSelector.quantity) )));  // Something like: sweeps[elangle] = src[parent] .
			// elangles << elangle;
			//mout.warn("add " ,  DataSet<PolarSrc>(src(parent), drain::RegExp(this->dataSelector.quantity) ) );
		}
		else {
			mout.note("elange ="  , elangle , " already added, skipping " , parent );
		}
		*/
	}

	//mout.note("first elange =" , sweeps.begin()->first , " DS =" , sweeps.begin()->second );
	//mout.note("first qty =" , sweeps.begin()->second.begin()->first , " D =" , sweeps.begin()->second.getFirstData() );


	ODIMPathElem dataSetPathElem(ODIMPathElem::DATASET); // ,1) //  2024/11
	//if (!DataTools::getNextDescendant(dst, ProductBase::appendResults.getType(), dataSetPath))

	if (ProductBase::appendResults.is(ODIMPathElem::DATASET)){
		if (ProductBase::appendResults.getIndex()){
			dataSetPathElem.index = ProductBase::appendResults.getIndex();
		}
		else {
			ODIMPathTools::getNextChild(dstRoot, dataSetPathElem);
		}
	}
	else if (ProductBase::appendResults.is(ODIMPathElem::DATA)){
		//mout.info("appending to next available data group in " , dataSetPath );  // ALWAYS dataset1, then ?
	}
	else if (ProductBase::appendResults.is(ODIMPathElem::ROOT)){
		if (!dstRoot.empty() && (&srcRoot != &dstRoot)){  // latter is ANDRE test... (kludge)
			mout.info("clearing previous result, use --append [data|dataset] to avoid");
			dstRoot.clear();
		}
	}
	else {
		dataSetPathElem = ProductBase::appendResults;
		mout.warn("non-standard path location '", dataSetPathElem, "', consider --help append ");
	}
	//++dataSetPath.index;

	//mout.warn("FAILED: "  , dataSetPath );
	//dataSetPath.push_back(ODIMPathElem(ODIMPathElem::DATASET, 1));

	mout.debug("storing product in path: ", dataSetPathElem);

	//Hi5Tree & dstProduct = dst[dataSetPath];
	/*   /// WARNING: Root odim has to be modified explicitly, otherwise remains empty.
	RootData<DstType<M> > root(dst);
	drain::VariableMap & whatRoot = root.getWhat();
	whatRoot["object"]  = this->odim.object;
	whatRoot["version"] = this->odim.version;
	 */


	/// MAIN
	Hi5Tree & dstDataSet = dstRoot[dataSetPathElem];
	{ // SCOPE
		DataSet<DstType<M> > dstProductDataset(dstDataSet); // PATH
		this->computeSingleProduct(sweeps, dstProductDataset);

		drain::VariableMap & how = dstProductDataset.getHow();
		ProductBase::setRackVersion(how);
		if (!dstProductDataset.empty()){
			/// Todo: how to handle undefined
			how["angles"] = dstProductDataset.getFirstData().odim.angles; // FIX: this may be some tmp data, not main [odim.quantity]
		}
	}

	/* keep for debugging
	for (const auto & entry: dstDataSet){
		mout.warn(entry.first, ": ", entry.second[ODIMPathElem::ARRAY].data.image.getProperties());
	}
	*/


	// Copy metadata from the input volume (note that dst may have been cleared above)
	mout.revised<LOG_DEBUG>("check DataSet metadata");
	// drain::TreeUtils::dump(dstRoot, std::cout,  DataTools::treeToStream);

	// Root level
	for (const ODIMPathElem::group_t group: {ODIMPathElem::WHAT, ODIMPathElem::WHERE, ODIMPathElem::HOW}){
		dstRoot[group].data.attributes = srcRoot[group].data.attributes;
	}
	drain::VariableMap & rootWhat = dstRoot[ODIMPathElem::WHAT].data.attributes; // dstProduct["what"].data.attributes;
	/// rootWhat = src[ODIMPathElem::WHAT].data.attributes;
	rootWhat["object"]  = this->odim.object; // ?
	rootWhat["version"] = this->odim.version;

	dstDataSet[ODIMPathElem::WHAT].data.attributes["product"] = this->odim.product;
	dstDataSet[ODIMPathElem::WHAT].data.attributes["startdate"] = dstODIM.startdate;
	dstDataSet[ODIMPathElem::WHAT].data.attributes["starttime"] = dstODIM.starttime;
	dstDataSet[ODIMPathElem::WHAT].data.attributes["enddate"]   = dstODIM.enddate;
	dstDataSet[ODIMPathElem::WHAT].data.attributes["endtime"]   = dstODIM.endtime;

	// PRODPAR ... complicated! Fix at Sprinter level, getKeyList()
	if (ODIM::versionFlagger.isSet(ODIM::RACK_EXTENSIONS)){

		dstDataSet[ODIMPathElem::WHAT].data.attributes["product2"] = dstODIM.product;

		// keep scope, even if condition removed
		std::stringstream sstr;
		char sep = 0;
		const drain::ReferenceMap & params = this->getParameters();
		for (const std::string & key: params.getKeyList()){
			if (sep){
				sstr << sep;
			}
			else {
				sep = ',';
			}
			sstr << key << '=' << params[key];  // TODO: escape \" ?
		}
		dstDataSet[ODIMPathElem::WHAT].data.attributes["rack_prodpar"] = sstr.str();
		//dstDataSet[ODIMPathElem::WHAT].data.attributes["prodpar"] = drain::sprinter(this->getParameters(), drain::Sprinter::cmdLineLayout);
	}

	return dstDataSet;

}


}  // namespace rack


#endif
