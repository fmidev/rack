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
#include <data/Data.h>
#include <data/DataTools.h>
#include <data/PolarODIM.h>
#include <data/QuantityMap.h>
#include <data/SourceODIM.h>
#include <hi5/Hi5Read.h>
#include <image/Image.h>
#include <radar/Geometry.h>
#include <stddef.h>
#include <util/Log.h>
#include <util/StringMapper.h>
#include <util/Tree.h>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>

using namespace drain::image;

namespace rack {

void ClutterOp::setClutterMap(const std::string & filename) const {

	drain::Logger mout(name, __FUNCTION__);

	if (!clutterMap.getChildren().empty()){
		if (clutterMap.data.attributes["filename"].toStr() == filename){
			mout.info() << "required map '" << filename << "' already loaded, skipping reload"  << mout.endl;
			return;
		}
	}

	mout.info() << "reading " << filename << mout.endl;
	try {
		hi5::Reader::readFile(filename, clutterMap);
		DataTools::updateAttributes(clutterMap);
		clutterMap.data.attributes["filename"] = filename;
	}
	catch (const std::runtime_error & e) {
		mout.warn() << "Failed reading cluttermap '" << filename << "'" << mout.endl;
	}

}

const Data<PolarSrc> & ClutterOp::getClutterMap(const PolarODIM & odim) const {

	drain::Logger mout(name, __FUNCTION__);

	const SourceODIM srcODIM(odim.source);

	// if (clutterMap.getChildren().empty()){ // Load map
	drain::StringMapper filepath;
	filepath.parse("cluttermaps/cluttermap-${NOD}.h5"); // consider a set of candidates, month-stamped?
	const std::string filename = filepath.toStr(srcODIM);
	// mout.note() << "no clutterMap, trying to load '" << filename << "'" << mout.endl;
	setClutterMap(filename); // Note: does not reload
	/*
	}
	else {

	}
	*/

	if (clutterMap.getChildren().empty()){
		mout.warn() << "no clutterMap available, problems ahead..." << mout.endl;
	}

	DataSet<PolarSrc> src(clutterMap["dataset1"]);
	mout.debug(1) << src << mout.endl;

	if (src.empty()){
		mout.warn() << "no data groups in structure" << mout.endl;
	}


	if (src.find("CLUTTER") != src.end()){
		return src.getData("CLUTTER");
	}
	else {
		if (src.size())
			mout.warn() << "no CLUTTER quantity found, using 1st data" << mout.endl;
		return src.getFirstData();
	}


}

void ClutterOp::processDataSet(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & aux) const {

	drain::Logger mout(name, __FUNCTION__);

	const Data<PolarSrc> & srcData = src.getFirstData();
	const size_t cols = srcData.data.getWidth();
	const size_t rows = srcData.data.getHeight();

	/*
	if (clutterMap.getChildren().empty()){
		// mout.note() << clutterMap << mout.endl;
		mout.warn() << "no clutterMap loaded, skipping" << mout.endl;
		return;
	}
	*/

	//DataSet<PolarSrc> clutterDataSet(clutterMap["dataset1"]);  // or directly ? "data1"
	//const Data<PolarSrc> & srcMap = clutterDataSet.getFirstData();
	const Data<PolarSrc> & srcMap = getClutterMap(srcData.odim);

	mout.note() << "using 1st data, quantity: " << srcMap.odim.quantity << mout.endl;

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
		for (unsigned short j=0; j<rows; ++j){
			dstProb.data.put(i,j, dstProb.odim.scaleInverse( coeff * srcMap.odim.scaleForward(srcMap.data.get<double>(i, j)) ));
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
