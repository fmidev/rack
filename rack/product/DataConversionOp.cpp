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
#include <drain/util/Debug.h>
#include <drain/util/Type.h>
#include "DataConversionOp.h"

namespace rack {


DataConversionOp::~DataConversionOp() {
}



const HI5TREE & DataConversionOp::getNormalizedData(const DataSetSrc<PolarSrc > & srcDataSet, DataSetDst<PolarDst > & dstDataSet, const std::string & quantity) {

	drain::MonitorSource mout("DataConversionOp", __FUNCTION__);

	const std::string quantityExt = std::string("~")+quantity;

	DataSetSrc<PolarSrc>::const_iterator it = srcDataSet.find(quantityExt);
	if (it != srcDataSet.end()){
		mout.debug(1) << "using cached quantity: " << quantityExt << mout.endl;
		return it->second.tree;
	}
	else {

		const PlainData<PolarSrc> & srcData = srcDataSet.getData(quantity);
		const EncodingODIM    & odimNorm = getQuantityMap().get(quantity)[srcData.odim.type.at(0)];

		if (EncodingODIM::haveSimilarEncoding(srcData.odim, odimNorm)){
			mout.debug(1) << "using original quantity (standard scaling):" << quantity << mout.endl;
			return srcData.tree;
		}
		else {
			mout.debug() << "converting and adding to cache: " << quantityExt << mout.endl;
			PlainData<PolarDst> & dstDataNew = dstDataSet.getData(quantityExt);
			dstDataNew.tree.data.noSave = true;
			DataConversionOp op;
			//op.setEncodingRequest(odimNorm);
			op.odim.importMap(odimNorm);
			// mout.warn() << "odimNorm: " << odimNorm << mout.endl;
			// mout.warn() << "op.odim: " << op.odim << mout.endl;
			dstDataNew.odim.quantity = quantity;
			op.processData(srcData, dstDataNew);
			dstDataNew.odim.quantity = quantityExt;
			dstDataNew.updateTree();
			// mout.warn() << "new odim: " << dstDataNew.odim << mout.endl;
			return dstDataNew.tree;
		}

	}
}



void DataConversionOp::processDataSet(const DataSetSrc<PolarSrc> & srcSweep, DataSetDst<PolarDst> & dstProduct) const {

	drain::MonitorSource mout(name, __FUNCTION__);

	//int index = 0;

	// Traverse quantities
	for (DataSetSrc<PolarSrc>::const_iterator it = srcSweep.begin(); it != srcSweep.end(); ++it){

		const std::string & quantity = it->first;


		if (quantity.empty()){
			mout.warn() << "empty quantity for data, skipping" << mout.endl;
			continue;
		}

		std::stringstream sstr; //quantity); //+"-tmp");
		//sstr << "data" << ++index << "tmp-"<< quantity;
		sstr << "data0." << quantity;

		mout.debug() << "quantity: " << quantity << mout.endl;

		const Data<PolarSrc> & srcData = it->second;
		Data<PolarDst>       & dstData = dstProduct.getData(quantity);

		mout.debug(1) << EncodingODIM(odim) << mout.endl;
		//mout.info() << "src " << (long int) &(srcData.data) << EncodingODIM(srcData.odim) << mout.endl;
		//mout.warn() << "dst " << (long int) &(dstData.data) << EncodingODIM(dstData.odim) << mout.endl;

		const drain::Type t(odim.type);

		const bool IN_PLACE = (&dstData.data == &srcData.data) && (t == srcData.data.getType2());

		if (IN_PLACE){

			if (ODIM::haveSimilarEncoding(srcData.odim, odim)){
				mout.info() << "already similar encoding, no need to convert" << mout.endl;
				continue; // to next quantity
			}

			mout.debug() << "in-place" << mout.endl;
			processData(srcData, dstData);
			dstData.updateTree();

		}
		else {
			//mout.warn() << "in-place" << mout.endl;
			mout.info() << "using tmp data (in-place computation not possible)" << mout.endl;

			if (true){ // SCOPE NEEDED FOR ERASING CLEANLY  dstProduct.tree[TMP_PATH]

				HI5TREE & tmpTree = dstProduct.tree[sstr.str()];
				tmpTree.data.noSave = ProductOp::outputDataVerbosity < 2;

				const std::set<std::string> & a = EncodingODIM::attributeGroups;
				for (std::set<std::string>::const_iterator it2 = a.begin(); it2 != a.end(); ++it2){
					if (dstData.tree.hasChild(*it2))
						tmpTree[*it2] = dstData.tree[*it2]; // ???
				}

				Data<PolarDst> dstDataNew(tmpTree);
				dstDataNew.odim.quantity = quantity;
				dstDataNew.odim.NI = srcData.odim.NI;
				ProductOp::handleEncodingRequest(dstDataNew.odim, encodingRequest);
					/*
				if (dstDataNew.odim.optimiseVRAD()){
					mout.warn() << "VRAD optimized: " << dstDataNew.odim << mout.endl;
					// ProductOp::handleEncodingRequest(dstDataNew.odim, encodingRequest);
				}
				*/

				dstDataNew.data.setGeometry(srcData.data.getGeometry());

				mout.debug() << "new scaling for " << quantity << ": " << EncodingODIM(dstDataNew.odim) << mout.endl;

				processData(srcData, dstDataNew);
				dstDataNew.updateTree();

				dstDataNew.tree.getChildren().swap(dstData.tree.getChildren());
				if (ProductOp::outputDataVerbosity == 0)
					//dstDataNew.tree.clear(); //getChildren().clear();
					// dstProduct.tree.getChildren().erase(TMP_PATH);
					dstProduct.tree.erase(sstr.str());
			}

		}

	}

}


void DataConversionOp::processData(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const {


	drain::MonitorSource mout(name, __FUNCTION__);

	mout.debug() << "start, " << dst << mout.endl;

	processImage(src.odim, src.data, dst.odim, dst.data);

}


void DataConversionOp::processImage(const PolarODIM & srcOdim, const drain::image::Image & srcImage, const PolarODIM & dstOdim, drain::image::Image & dstImage) const {


	drain::MonitorSource mout(name, __FUNCTION__);
	//mout.debug(1) << *this << mout.endl;
	mout.debug(1) << "start, type=" << odim.type << ", geom=" << srcImage.getGeometry() << mout.endl;

	mout.debug(1) << "dst:" << dstImage << mout.endl;

	const drain::Type t(odim.type);
	const drain::image::Geometry g(srcImage.getGeometry());
	if (srcImage.hasOverlap(dstImage)){
		if (t != srcImage.getType2()){
			mout.error() << "trying to change type when dst==src" << mout.endl;
			return;
		}
		if (g != dstImage.getGeometry()){
			mout.error() << "trying to change geometry when dst==src" << mout.endl;
			return;
		}
	}
	else {
		dstImage.setType(t);
		dstImage.setGeometry(g);
	}
	//if (&src != &dst){
	//dst.setType(odimOut.type.at(0));

	mout.debug(1) << "dst:" << dstImage << mout.endl;

	dstImage.setCoordinatePolicy(srcImage.getCoordinatePolicy());

	//const double ud = std::max(odimOut.undetect, dst.getMin<double>());
	//const double nd = std::min(odimOut.nodata, dst.getMax<double>());

	//mout.debug(2) << "input name: " << src.getName() << mout.endl;

	mout.debug(1) << "input odim: " << EncodingODIM(srcOdim) << mout.endl;
	mout.debug(4) << "input properties: " << srcImage.properties << mout.endl;
	//std::cerr << src.properties << std::endl;


	dstImage.properties = srcImage.properties;
	dstImage.properties["what:type"] = odim.type;  // ?
	dstImage.properties["what:gain"] = odim.gain;  // odimOut...
	dstImage.properties["what:offset"] = odim.offset;
	dstImage.properties["what:undetect"] = odim.undetect;
	dstImage.properties["what:nodata"] = odim.nodata;

	//dst.odim.set(odim);


	mout.debug(1) << "output properties: " << dstImage.properties << mout.endl;
	//std::cerr << dst.properties << std::endl;

	//const drain::DataScaling scaling(srcOdim.gain, srcOdim.offset, odim.gain, odim.offset);
	const drain::DataScaling scaling(srcOdim.gain, srcOdim.offset, dstOdim.gain, dstOdim.offset);

	mout.debug(2) << "scaling: " << scaling << mout.endl;

	Image::const_iterator s = srcImage.begin();
	Image::iterator d = dstImage.begin();
	// srcImage.getByteSize();
	mout.debug(2) << "src:    " << srcImage << mout.endl;
	mout.debug(2) << "target: " << dstImage << mout.endl;
	double x;
	while (s != srcImage.end()){
		x = *s;

		/// Checks 'undetect' first because 'undetect' and 'nodata' may be the same code
		if (x == srcOdim.undetect)
			*d = odim.undetect;
		else if (x == srcOdim.nodata)
			*d = odim.nodata;
		else {
			//  x = srcOdim.scaleForward(x);
			//  x = dst.odim.scaleInverse(x);
			// *d = dstImage.limit<double>( x );
			*d = dstImage.limit<double>( scaling.forward(x) );
		}
		//*d = dstImage.limit<double>( scaling.forward(x) );

		++s;
		++d;
	}

}




}

// Rack
