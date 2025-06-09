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
#include <drain/Log.h>
#include "data/Data.h"
#include "data/DataTools.h"
#include "data/PolarODIM.h"
#include "data/QuantityMap.h"
#include "data/SourceODIM.h"
#include <hi5/Hi5Read.h>
#include <drain/image/Image.h>
#include "radar/Geometry.h"
//#include <stddef.h>
#include <drain/util/StringMapper.h>
#include <drain/util/TreeOrdered.h>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>

using namespace drain::image;

namespace rack {

void ClutterOp::setClutterMap(const std::string & filename) const {

	drain::Logger mout(__FILE__, __FUNCTION__); //REPL __FILE__, __FUNCTION__);

	if (!clutterMap.getChildren().empty()){
		if (clutterMap.data.attributes["filename"].toStr() == filename){
			mout.note("required map '" , filename , "' already loaded, skipping reload"  );
			return;
		}
	}

	mout.debug("reading " , filename );
	try {
		hi5::Reader::readFile(filename, clutterMap);
		DataTools::updateInternalAttributes(clutterMap);
		clutterMap.data.attributes["filename"] = filename;
	}
	catch (const std::runtime_error & e) {
		mout.warn("Failed reading cluttermap '" , filename , "'" );
	}

}

const Hi5Tree & ClutterOp::getClutterMap(const PolarODIM & odim) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	SourceODIM srcODIM(odim.source);
	srcODIM["quantity"] = quantity;      // for file path
	srcODIM["what:quantity"] = quantity; // for data selector

	const std::string filePath = drain::StringMapper(this->file).toStr(srcODIM);
	mout.note("clutter map: '", filePath, "'");
	setClutterMap(filePath); // Note: loads new only when needed

	if (clutterMap.getChildren().empty()){
		mout.warn("no clutterMap available, problems ahead..." );
		return clutterMap;
	}

	ODIMPath path;
	DataSelector selector(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
	// selector.quantity = this->quantity; // "CLUTTER";
	// selector.setParameters("");
	// selector.setParameter("quantity", quantity);
	selector.setQuantities(quantity);
	mout.debug("selector " , selector ); // TODO protect quantity

	//selector.ensureDataGroup();
	if (selector.getPath(clutterMap, path)){ //, ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
		mout.info("found " , path );
		return clutterMap(path);
	}
	else {
		mout.warn("using default path " , path );
		return clutterMap["dataset1"]["data1"];
		//return clutterMap.begin()->second.begin()->second;  // "dataset1/data1"
	}


}

void ClutterOp::runDetection(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & aux) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	const Data<PolarSrc> & srcData = src.getFirstData();

	const Hi5Tree & clutterMap = getClutterMap(srcData.odim);

	if (clutterMap.empty()){
		mout.fail("clutterMap empty, skipping");
		return;
	}

	//const Data<PolarSrc> srcMap(getClutterMap(srcData.odim));
	const Data<PolarSrc> srcMap(clutterMap);

	mout.note("using clutter 'quality quantity': " , srcMap.odim.quantity );

	/*
	if (srcMap.data.isEmpty()){
		//clutterMap.dumpContents(std::cerr);
		//hi5::Hi5Base::writeText(clutterMap);
		mout.warn(clutterMap );

		// mout.note(clutterDataSet );
		mout.warn("input clutter map missing, giving up." );
		return;
	}*/

	const size_t cols = srcData.data.getWidth();
	const size_t rows = srcData.data.getHeight();


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

	// dstProb.updateTree();

	writeHow(dstProb);

}

}

// Rack
