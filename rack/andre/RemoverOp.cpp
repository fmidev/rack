/**

    Copyright 2011-2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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


#include "RemoverOp.h"

//#include "hi5/Hi5Write.h"

//#include <drain/image/File.h>
//#include <drain/image/MathOpPack.h>


using namespace drain::image;
using namespace hi5;

#include "QualityCombinerOp.h"

namespace rack {

void RemoverOp::processDataSets(const DataSetMap<PolarSrc> & srcDataSets, DataSetMap<PolarDst> & dstDataSets) const {

	drain::Logger mout(name+"(RemoverOp)", __FUNCTION__);

	mout.debug(1) << "start" << mout.endl;

	DataSetMap<PolarSrc>::const_iterator its = srcDataSets.begin();
	DataSetMap<PolarDst>::iterator       itd = dstDataSets.begin();
	while (its != srcDataSets.end()){
		mout.info() << "processing elangle:" << its->first << mout.endl;
		if (its->first == itd->first){

			const DataSet<PolarSrc> & srcDataSet = its->second;
			DataSet<PolarDst> & dstDataSet = itd->second;

			/// MAIN COMMAND
			processDataSet(srcDataSet, dstDataSet);
			//dstProb.updateTree();

		}
		else {
			mout.warn() << "something went wrong, dst has different angle, dst=" << itd->first << " != " << its->first << mout.endl;
			return;
		}
		++its;
		++itd;
	}

}

void RemoverOp::processDataSet(const DataSet<PolarSrc> & srcDataSet, DataSet<PolarDst> & dstDataSet) const {

	drain::Logger mout(name+"(RemoverOp)", __FUNCTION__);

	mout.debug() << "start" << mout.endl;

	if (srcDataSet.size() == 0){
		mout.warn() << "dataset contains no data, skipping" << mout.endl;
		return;
	}
	else {

		const PlainData<PolarSrc> & srcDataSetQualityIndex = srcDataSet.getQualityData("QIND");
		//const PlainData<PolarSrc> & srcDataSetQualityClass = srcDataSet.getQualityData("CLASS"); // TODO
		const bool DATASETQUALITY = !srcDataSetQualityIndex.data.isEmpty();

		for (DataSet<PolarSrc>::const_iterator it = srcDataSet.begin(); it != srcDataSet.end(); ++it){
			mout.info() << "calling processData() for " << it->first << " elangle=" << it->second.odim.elangle << mout.endl;

			Data<PolarDst> & dstData = dstDataSet.getData(it->first);  // create or retrieve data

			if (DATASETQUALITY)
				QualityCombinerOp::updateLocalQuality(srcDataSet, dstData);

			const Data<PolarSrc> & srcData = it->second;
			//const PlainData<PolarSrc> & srcDataQuality = srcData.getQualityData();
			//const bool DATAQUALITY = !srcDataQuality.data.isEmpty();
			//

			const bool LOCALQUALITY = srcData.hasQuality();
			/*
			if (LOCALQUALITY){
				mout.toOStr() << "quantity '"<< srcData.odim.quantity << "' has local quality" << mout.endl;
			}
			*/
			// principle: dstData always has own quality after this, because orig (perhaps global) q data will be modified

			if (LOCALQUALITY){
				// TODO if (REQUIRE_STANDARD_DATA){ ?
				//mout.warn() << "quality data yes? " << dstData.getQualityData() << mout.endl;
				processData(srcData, dstData);
			}
			else {
				mout.note() << "no quality data for quantity=" << it->first << " elangle=" << it->second.odim.elangle << ", skipping " << mout.endl;
			}
		}

	}

}


void RemoverOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	drain::Logger mout(name, __FUNCTION__);

	mout.debug(1) << "start" << mout.endl;

	PlainData<PolarSrc> srcQIND = srcData.getQualityData();

	if (srcQIND.data.isEmpty()){
		mout.warn() << " src QIND emprty, skipping..." << mout.endl;
		return;
	}

	// TODO FIX use QualityThresholdOp!

	const double t = threshold * srcQIND.odim.scaleInverse(1.0);

	// mout.warn() << " t=" << t << mout.endl;

	Image::const_iterator  it = srcData.data.begin();
	Image::const_iterator qit = srcQIND.data.begin();
	Image::iterator dit = dstData.data.begin();
	//Image::iterator dqit = dstData.data.begin();

	const Image::iterator end = srcData.data.end();
	while (it != end){
		if (*qit < t)
			*dit = dstData.odim.nodata;
		else
			*dit = *it; // assume no scaling difference
		++it;
		++qit;
		++dit;
	}

}







}

