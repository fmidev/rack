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

//#include "drain/image/File.h"
//#include "drain/image/MathOpPack.h"


using namespace drain::image;
using namespace hi5;

#include "QualityCombinerOp.h"

namespace rack {

void RemoverOp::processDataSets(const DataSetMap<PolarSrc> & srcDataSets, DataSetMap<PolarDst> & dstDataSets) const {

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL name+"(RemoverOp)", __FUNCTION__);

	mout.debug2() << "start" << mout.endl;

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

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL name+"(RemoverOp)", __FUNCTION__);

	mout.debug() << "start" << mout.endl;

	if (srcDataSet.size() == 0){
		mout.warn() << "dataset contains no data, skipping" << mout.endl;
		return;
	}
	else {

		const PlainData<PolarSrc> & srcDataSetQualityIndex = srcDataSet.getQualityData("QIND");
		const bool DATASET_QUALITY = !srcDataSetQualityIndex.data.isEmpty();
		mout.debug() << "DataSet level quality: "  << (int) DATASET_QUALITY <<  mout.endl;

		for (DataSet<PolarSrc>::const_iterator it = srcDataSet.begin(); it != srcDataSet.end(); ++it){
			mout.info() << "calling processData() for " << it->first << " elangle=" << it->second.odim.elangle << mout.endl;

			Data<PolarDst> & dstData = dstDataSet.getData(it->first);  // create or retrieve data

			if (DATASET_QUALITY)
				QualityCombinerOp::updateLocalQuality(srcDataSet, dstData);

			const Data<PolarSrc> & srcData = it->second;

			const bool LOCAL_QUALITY = srcData.hasQuality();
			/*
			if (LOCALQUALITY){
				mout.toOStr() << "quantity '"<< srcData.odim.quantity << "' has local quality" << mout.endl;
			}
			*/
			// principle: dstData always has own quality after this, because orig (perhaps global) q data will be modified

			if (LOCAL_QUALITY){
				// TODO if (REQUIRE_STANDARD_DATA){ ?
				mout.info() << "using local quality data" << mout.endl;
				processData(srcData, srcData.getQualityData(), dstData, dstData.getQualityData());
			}
			else if (DATASET_QUALITY) {
				mout.warn() << "using dataset level data" << mout.endl;
				processData(srcData, srcDataSetQualityIndex, dstData, dstDataSet.getQualityData());
			}
			else {
				mout.note() << "no quality data for quantity=" << it->first << " elangle=" << it->second.odim.elangle << ", skipping " << mout.endl;
			}
		}

	}

}


void RemoverOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.error() << "explicitly selected quality field required, skipping" << mout.endl;

	//processData(srcData, srcData.getQualityData(), dstData, dstData.getQualityData());

}

void RemoverOp::processData(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality,
		PlainData<PolarDst> & dstData, PlainData<PolarDst> & dstQuality) const {


	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.debug2() << "start" << mout.endl;
	// PlainData<PolarSrc> srcQuality = srcData.getQualityData();

	if (srcQuality.data.isEmpty()){
		mout.warn() << " src Quality empty, skipping..." << mout.endl;
		return;
	}

	// File::write(srcQIND.data, "srcQIND.png");
	// TODO FIX use QualityThresholdOp!

	//const double t = threshold * srcQIND.odim.scaleInverse(1.0);
	const double t = srcQuality.odim.scaleInverse(threshold * 1.0);

	mout.debug() << " t=" << t << mout.endl;
	/*
	mout.note() << " srcData" << srcData << mout.endl;
	mout.warn() << " srcQuality" << srcQuality << mout.endl;
	mout.note() << " dstData" << dstData << mout.endl;
	*/
	double replaceCode = dstData.odim.nodata;
	if (replace == "nodata"){
		replaceCode = dstData.odim.nodata;
	}
	else if (replace == "undetect"){
		replaceCode = dstData.odim.undetect;
	}
	else {
		replaceCode = dstData.odim.scaleInverse(drain::StringTools::convert<double>(replace));
	}
	mout.note() << replace << " = > replaceCode: " << replaceCode << mout.endl;

	const bool ZERO_QUALITY = dstQuality.odim.scaleInverse(0.0);
	if (clearQuality){
		mout.debug() << "zero quality code: " << ZERO_QUALITY << mout.endl;
	}

	//File::write(dstData.data, "dst1.png");

	Image::const_iterator  it = srcData.data.begin();
	Image::const_iterator qit = srcQuality.data.begin();
	Image::iterator dit  = dstData.data.begin();
	Image::iterator dqit = dstQuality.data.begin();


	const Image::iterator end = srcData.data.end();
	while (it != end){
		if (static_cast<double>(*qit) < t){
			*dit = replaceCode; //dstData.odim.nodata;
			if (clearQuality)
				*dqit = ZERO_QUALITY;
		}
		else
			*dit = *it; // assume no scaling difference
		++it;
		++qit;
		++dit;
		++dqit;
	}

	// File::write(dstData.data, "dst2.png");

}







}


// Rack
