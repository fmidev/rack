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

#include "VolumeTraversalOp.h" // NEW


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





//template <class M>
void VolumeTraversalOp::processVolume(const HI5TREE &src, HI5TREE &dst) const {

	drain::Logger mout(this->name+"(VolumeTraversalOpOp)", __FUNCTION__);
	mout.debug(1) << "start" << mout.endl;

	DataSetMap<PolarSrc> srcDataSets;
	DataSetMap<PolarDst> dstDataSets;

	std::list<std::string> dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	DataSelector::getPaths(src, this->dataSelector, dataPaths);

	if (dataPaths.size() == 0)
		mout.note() << "no dataPaths matching selector="  << this->dataSelector << mout.endl;

	drain::RegExp quantityRegExp(this->dataSelector.quantity); // DataSet objects (further below)
	//drain::Variable elangles(typeid(double));
	//mout.warn() << "regExp: " << quantityRegExp << mout.endl;


	for (std::list<std::string>::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

		mout.debug() << "considering" << *it << mout.endl;

		const std::string parent = DataTools::getParent(*it);
		const double elangle = src(parent)["where"].data.attributes["elangle"];

		if (srcDataSets.find(elangle) == srcDataSets.end()){
			mout.info() << "add "  << elangle << ':'  << parent << mout.endl;

			/// its and itd for debugging
			//DataSetMap<PolarSrc>::const_iterator its = srcDataSets.insert(srcDataSets.begin(), DataSetMap<PolarSrc>::value_type(elangle, DataSet<>(src[parent], quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .
			srcDataSets.insert(DataSetMap<PolarSrc>::value_type(elangle, DataSet<PolarSrc>(src(parent), quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .

			//DataSetMap<PolarDst>::iterator itd = dstDataSets.begin();
			//itd = dstDataSets.insert(itd, DataSetMap<PolarDst>::value_type(elangle, DataSet<>(dst[parent], quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .
			dstDataSets.insert(DataSetMap<PolarDst>::value_type(elangle, DataSet<PolarDst>(dst(parent), quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .

			// elangles << elangle;

		}

	}

	mout.debug() << "ok, calling processDataSets " << mout.endl;

	processDataSets(srcDataSets, dstDataSets);


}


void VolumeTraversalOp::processDataSets(const DataSetMap<PolarSrc> & srcDataSets, DataSetMap<PolarDst> & dstDataSets) const {

	drain::Logger mout(name+"(DetectorOp)", __FUNCTION__);

	mout.debug(1) << "start1" << mout.endl;

	DataSetMap<PolarSrc>::const_iterator its = srcDataSets.begin();
	DataSetMap<PolarDst>::iterator itd = dstDataSets.begin();
	while (its != srcDataSets.end()){

		mout.info() << "processing elangle:" << its->first << mout.endl;

		itd = dstDataSets.find(its->first);

		//if (its->first == itd->first){
		if (itd != dstDataSets.end()){

			const DataSet<PolarSrc> & srcDataSet = its->second;
			DataSet<PolarDst> & dstDataSet = itd->second;

			/// MAIN COMMAND
			processDataSet(srcDataSet, dstDataSet);
			//dstDataSet.updateTree(); // create /what, /where etc.
			//DataTools::updateAttributes(dstProb.tree); // collect attributes from /what, /where to /data:data properties so that srcData.getQualityData() works below.
			// update other trees?


		}
		else {
			mout.warn() << "something went wrong, dst has no angle, src=" << its->first << mout.endl;
			return;
		}
		++its;
		//++itd;
	}

	mout.debug(1) << "end" << mout.endl;

}


}  // namespace rack

