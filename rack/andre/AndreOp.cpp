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

#include <drain/image/File.h>
//#include <drain/image/MathOpPack.h>
#include <drain/util/Debug.h>

#include "hi5/Hi5Write.h"

#include "AndreOp.h"


using namespace drain::image;
using namespace hi5;



namespace rack {

//bool AndreOp::universalMode(true);

// std::string AndreOp::pathCombinedQuality("data~");

/*
void AndreOp::processVolume(const HI5TREE &src, HI5TREE &dst) const {

	drain::MonitorSource mout(name+"(AndreOp)", __FUNCTION__);
	mout.debug(1) << "start" << mout.endl;

	DataSetSrcMap srcDataSets;
	DataSetDstMap<PolarDst> dstDataSets;

	std::list<std::string> dataPaths;  // Down to ../dataN/ level, eg. /dataset5/data4
	DataSelector::getPaths(src, dataSelector, dataPaths);

	if (dataPaths.size() == 0)
		mout.note() << "no dataPaths matching selector="  << dataSelector << mout.endl;

	drain::RegExp quantityRegExp(dataSelector.quantity); // DataSet objects (further below)
	//drain::Variable elangles(typeid(double));

	for (std::list<std::string>::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

		mout.debug() << "considering" << *it << mout.endl;

		const std::string parent = DataSelector::getParent(*it);
		const double elangle = src(parent)["where"].data.attributes["elangle"];

		if (srcDataSets.find(elangle) == srcDataSets.end()){
			mout.info() << "add "  << elangle << ':'  << parent << mout.endl;

			/// its and itd for debugging
			//DataSetSrcMap::const_iterator its = srcDataSets.insert(srcDataSets.begin(), DataSetSrcMap::value_type(elangle, DataSetSrc<>(src[parent], quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .
			srcDataSets.insert(DataSetSrcMap::value_type(elangle, DataSetSrc<PolarSrc>(src(parent), quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .

			//DataSetDstMap::iterator itd = dstDataSets.begin();
			//itd = dstDataSets.insert(itd, DataSetDstMap::value_type(elangle, DataSetDst<>(dst[parent], quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .
			dstDataSets.insert(DataSetDstMap<>::value_type(elangle, DataSetDst<PolarDst>(dst(parent), quantityRegExp)));  // Something like: sweeps[elangle] = src[parent] .

			// elangles << elangle;

		}

	}

	mout.debug() << "ok, calling processDataSets " << mout.endl;

	processDataSets(srcDataSets, dstDataSets);


}
*/



// obsolete
/*
Image & AndreOp::getCombinedQualityData(HI5TREE &dstRoot, const std::string &path) {

	drain::MonitorSource mout("(AndreOp)", __FUNCTION__);

	std::list<std::string> qualityPaths;
	DataSelector::getQualityPaths(dstRoot, path, qualityPaths);

	if (qualityPaths.empty()){
		mout.warn() << "No quality data found for path=" << path << mout.endl;
		static Image empty;
		return empty;
	}

	std::list<std::string>::const_iterator it = qualityPaths.begin();

	if (qualityPaths.size() == 1){
		mout.info() << " using unique field: "<< *qualityPaths.begin() << mout.endl;
		return (dstRoot[*it].data.dataSet);
	}
	else {

		Image & combinedQuality = dstRoot[path]["quality1"][pathCombinedQuality].data.dataSet;

		if (combinedQuality.isEmpty()){
			mout.debug(1) << " No previous field in " << path << "/quality1/" << pathCombinedQuality << '\n';
			mout << " ...Hence copying: "<< *it << mout.endl;
			combinedQuality.copyDeep(dstRoot[*it].data.dataSet);
			//DataSelector::getAttributes(dstRoot, path, combinedQuality.properties);
			DataSelector::getAttributes(dstRoot, *it, combinedQuality.properties); // NEW 2014
			//combinedQuality.setGeometry(dstRoot[*it].data.dataSet.getGeometry());
			//combinedQuality.fill(255);
			//combinedQuality.properties["class"]
		}

		//char c = '0';
		//drain::image::File::write(combinedQuality, std::string("outq")+c+".png");

		drain::image::MinimumOp minimumOp;
		++it;
		while (it != qualityPaths.end()){
			mout.debug() << " updating: "<< *it << mout.endl;
			minimumOp.filter(dstRoot[*it].data.dataSet, combinedQuality);
			// drain::image::File::write(dstRoot[*it].data.dataSet, std::string("outL.png"));
			//++c;
			// drain::image::File::write(combinedQuality, std::string("outq")+c+".png");
			++it;

		}

		return combinedQuality;
	}

}
*/

}  // rack::


// Rack
