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


// Rack
