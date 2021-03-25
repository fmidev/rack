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

#include <andre/ClutterOp.h>
#include "data/Data.h"
#include "data/DataTools.h"
#include "data/PolarODIM.h"
#include "data/QuantityMap.h"
#include "data/SourceODIM.h"
#include <hi5/Hi5Read.h>
#include "drain/image/Image.h"
#include "radar/Geometry.h"
#include <stddef.h>
#include "drain/util/Log.h"
#include "drain/util/StringMapper.h"
#include "drain/util/Tree.h"
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>

using namespace drain::image;

namespace rack {

void ClutterOp::setClutterMap(const std::string & filename) const {

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL __FUNCTION__, __FILE__);

	if (!clutterMap.getChildren().empty()){
		if (clutterMap.data.attributes["filename"].toStr() == filename){
			mout.note() << "required map '" << filename << "' already loaded, skipping reload"  << mout.endl;
			return;
		}
	}

	mout.debug() << "reading " << filename << mout.endl;
	try {
		hi5::Reader::readFile(filename, clutterMap);
		DataTools::updateInternalAttributes(clutterMap);
		clutterMap.data.attributes["filename"] = filename;
	}
	catch (const std::runtime_error & e) {
		mout.warn() << "Failed reading cluttermap '" << filename << "'" << mout.endl;
	}

}

const Hi5Tree & ClutterOp::getClutterMap(const PolarODIM & odim) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	const SourceODIM srcODIM(odim.source);

	// if (clutterMap.getChildren().empty()){ // Load map
	drain::StringMapper filepath;
	//filepath.parse("cluttermaps/cluttermap-${NOD}.h5"); // consider a set of candidates, month-stamped?
	filepath.parse(this->pathSyntax);
	const std::string filename = filepath.toStr(srcODIM);
	mout.note() << "clutter map: '" << filename << "'" << mout.endl;
	setClutterMap(filename); // Note: load new only when needed

	if (clutterMap.getChildren().empty()){
		mout.warn() << "no clutterMap available, problems ahead..." << mout.endl;
		return clutterMap;
	}

	ODIMPath path;
	DataSelector selector(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
	selector.quantity = "CLUTTER";

	/*
	ODIMPathList paths;
	selector.getPaths(clutterMap, paths, ODIMPathElem::DATA | ODIMPathElem::QUALITY);
	for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); ++it){
		mout.warn() << "path candidate " << *it << mout.endl;
	}
	*/

	if (selector.getPath3(clutterMap, path)){ //, ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
		mout.debug() << "found " << path << mout.endl;
		return clutterMap(path);
	}
	else {
		mout.debug() << "using default path " << path << mout.endl;
		return clutterMap["dataset1"]["data1"];
		//return clutterMap.begin()->second.begin()->second;  // "dataset1/data1"
	}


}

void ClutterOp::processDataSet(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & aux) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	const Data<PolarSrc> & srcData = src.getFirstData();
	const size_t cols = srcData.data.getWidth();
	const size_t rows = srcData.data.getHeight();

	//const Data<PolarSrc> & srcMap = clutterDataSet.getFirstData();
	const Data<PolarSrc> srcMap(getClutterMap(srcData.odim));

	mout.note() << "using quantity: " << srcMap.odim.quantity << mout.endl;

	if (srcMap.data.isEmpty()){
		//clutterMap.dumpContents(std::cerr);
		//hi5::Hi5Base::writeText(clutterMap);
		mout.warn() << clutterMap << mout.endl;

		// mout.note() << clutterDataSet << mout.endl;
		mout.warn() << "input clutter map missing, giving up." << mout.endl;
		return;
	}




	//const
	dstProb.odim.quantity = "CLUTTER";
	getQuantityMap().setQuantityDefaults(dstProb, "PROB");
	dstProb.data.setGeometry(cols, rows);

	bool GAMMA = (gamma != 1.0);
	double g = 1.0/gamma;
	double x;

	double altitude;
	double coeff;
	//double distance;
	//double distanceMax = 0;
	//unsigned short i2;
	const double halfCoeff = ::log(decay) / 1000;
	for (unsigned short i=0; i<cols; ++i){
		altitude = Geometry::heightFromEtaGround(srcData.odim.elangle * drain::DEG2RAD, srcData.odim.getBinDistance(i));
		coeff = ::exp(altitude * halfCoeff); //
		//distance = Geometry::groundFromEtaBeam(srcData.odim.elangle, srcData.odim.getBinDistance(i));
		//i2 = srcMap.odim.getBinIndex(distance);
		//if (i2 < cols_map){
		if (GAMMA){
			for (unsigned short j=0; j<rows; ++j){
				x = srcMap.odim.scaleForward(srcMap.data.get<double>(i, j));
				dstProb.data.put(i,j, dstProb.odim.scaleInverse( coeff * ::pow(x, g)));
			}
		}
		else {
			for (unsigned short j=0; j<rows; ++j){
				dstProb.data.put(i,j, dstProb.odim.scaleInverse( coeff * srcMap.odim.scaleForward(srcMap.data.get<double>(i, j)) ));
			}
		}
		//}
	}

	//@ dstProb.updateTree();

	writeHow(dstProb);

	/*
	const Data<PolarSrc> & srcDBZH = src.getData("DBZH");


	const drain::image::Geometry & geometry = srcTH.data.getGeometry();

	if (srcDBZH.data.getGeometry() != geometry){
		mout.warn() << "different geometry in TH and DBZH (unsupported), giving up." << mout.endl;
		return;
	}


	//PlainDataDst & dstProb = dst;
			//dst.getQualityData("AClutterMapD");
	//dstProb.data.setGeometry(geometry);
	const double QMAX = dstProb.odim.scaleInverse(1.0);

	/// Main loop
	double dbzh, th;
	drain::FuzzyPeak<double, unsigned char> fuzzy(0.0, reflHalfWidth, QMAX);
	Image::const_iterator ith   = srcTH.data.begin();
	Image::const_iterator idbzh = srcDBZH.data.begin();
	Image::const_iterator it = dstProb.data.begin();
	while (ith != srcTH.data.end()){
		th = *ith;
		if (th == srcTH.odim.nodata){
			*it = 0;
		}
		else if (th != srcTH.odim.undetect){
			dbzh = *idbzh;
			if (dbzh != srcDBZH.odim.nodata){
				if (dbzh == srcDBZH.odim.undetect)
					dbzh = -32.0;
				else
					dbzh = srcDBZH.odim.scaleForward(dbzh);
				th = srcTH.odim.scaleForward(th);
				*it = QMAX - fuzzy(dbzh - th);
			}
		}
		else {
			*it = 0;
		}
		++ith;
		++idbzh;
		++it;
	}

	writeHow(dstProb);
	*/
}

}

// Rack
