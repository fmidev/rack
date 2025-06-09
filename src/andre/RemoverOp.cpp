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

	drain::Logger mout(__FILE__, __FUNCTION__); //REPL name+"(RemoverOp)", __FUNCTION__);

	mout.debug2("start" );

	DataSetMap<PolarSrc>::const_iterator its = srcDataSets.begin();
	DataSetMap<PolarDst>::iterator       itd = dstDataSets.begin();
	while (its != srcDataSets.end()){
		mout.info("processing elangle:" , its->first );
		if (its->first == itd->first){

			const DataSet<PolarSrc> & srcDataSet = its->second;
			DataSet<PolarDst> & dstDataSet = itd->second;

			/// MAIN COMMAND
			processDataSet(srcDataSet, dstDataSet);
			//dstProb.updateTree();

		}
		else {
			mout.warn("something went wrong, dst has different angle, dst=" , itd->first , " != " , its->first );
			return;
		}
		++its;
		++itd;
	}

}

void RemoverOp::processDataSet(const DataSet<PolarSrc> & srcDataSet, DataSet<PolarDst> & dstDataSet) const {

	drain::Logger mout(__FILE__, __FUNCTION__); //REPL name+"(RemoverOp)", __FUNCTION__);

	mout.debug("start" );

	if (srcDataSet.size() == 0){
		mout.warn("dataset contains no data, skipping" );
		return;
	}
	else {

		const PlainData<PolarSrc> & srcDataSetQualityIndex = srcDataSet.getQualityData("QIND");
		const bool DATASET_QUALITY = !srcDataSetQualityIndex.data.isEmpty();
		mout.debug("DataSet level quality: "  , (int) DATASET_QUALITY );

		for (DataSet<PolarSrc>::const_iterator it = srcDataSet.begin(); it != srcDataSet.end(); ++it){
			mout.info("calling processData() for " , it->first , " elangle=" , it->second.odim.elangle );

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
				mout.info("using local quality data" );
				processData(srcData, srcData.getQualityData(), dstData, dstData.getQualityData());
			}
			else if (DATASET_QUALITY) {
				mout.note("using dataset-level quality data" );
				processData(srcData, srcDataSetQualityIndex, dstData, dstDataSet.getQualityData());
			}
			else {
				mout.note("no quality data for quantity=" , it->first , " elangle=" , it->second.odim.elangle , ", skipping " );
			}
		}

	}

}


void RemoverOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.error("explicitly selected quality field required, skipping" );

	//processData(srcData, srcData.getQualityData(), dstData, dstData.getQualityData());

}

void RemoverOp::processData(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality,
		PlainData<PolarDst> & dstData, PlainData<PolarDst> & dstQuality) const {


	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug2("start" );
	// PlainData<PolarSrc> srcQuality = srcData.getQualityData();

	if (srcQuality.data.isEmpty()){
		mout.warn(" src Quality empty, skipping..." );
		return;
	}

	// File::write(srcQIND.data, "srcQIND.png");
	// TODO FIX use QualityThresholdOp!

	//const double t = threshold * srcQIND.odim.scaleInverse(1.0);
	const double t = srcQuality.odim.scaleInverse(threshold * 1.0);

	mout.debug(" t=" , t );
	/*
	mout.note(" srcData" , srcData );
	mout.warn(" srcQuality" , srcQuality );
	mout.note(" dstData" , dstData );
	*/
	double replaceCode = dstData.odim.nodata;
	if (replace == "nodata"){
		replaceCode = dstData.odim.nodata;
	}
	else if (replace == "undetect"){
		replaceCode = dstData.odim.undetect;
	}
	else {
		// std::numeric_limits<double>::quiet_NaN();
		try {
			double d = std::stod(replace);
			replaceCode = dstData.odim.scaleInverse(d);
		}
		catch (const std::exception & e) {
			mout.warn(e.what());
			mout.error("Failed in converting replace='", replace, "' to a number");
			return;
		}
		//drain::StringTools::lazyConvert(replace, d);
	}
	mout.note(replace , " = > replaceCode: " , replaceCode );

	const double ZERO_QUALITY = dstQuality.odim.scaleInverse(0.0);
	//const bool ZERO_QUALITY = dstQuality.data.getConf().getTypeMin<double>();
	if (clearQuality){
		mout.special("zero quality code: " , ZERO_QUALITY );
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
