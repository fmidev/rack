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



//#include <drain/util/Fuzzy.h>

#include <drain/image/File.h>
//#include <drain/imageops/DistanceTransformFillOp.h>
//#include <drain/imageops/RecursiveRepairerOp.h>


#include "data/DataCoder.h"
#include "hi5/Hi5.h"
//#include "hi5/Hi5Read.h"

#include "radar/Coordinates.h"
#include "radar/Composite.h"

#include "radar/Sun.h"


#include "cartesian-extract.h"


namespace rack {



void CartesianExtract::extract(const std::string & channels) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	RackResources & resources = getResources();


	// Append results - why not, but Cartesian was typically used for subcompositing
	// resources.cartesianHi5.clear();
	resources.setSource(resources.cartesianHi5, *this);

	//ODIMPath path("dataset1");
	ODIMPath path;

	ODIMPathElem parent(ODIMPathElem::DATASET, 1);
	if (ProductBase::appendResults.is(ODIMPathElem::DATASET))
		DataSelector::getNextChild(resources.cartesianHi5, parent);
	else if (ProductBase::appendResults.is(ODIMPathElem::DATA)){
		DataSelector::getLastChild(resources.cartesianHi5, parent);
		if (parent.index == 0){
			parent.index = 1;
		}
	}
	else
		resources.cartesianHi5.clear(); // don't append, overwrite...

	path << parent; // ?
	mout.debug() << "dst path: " << path << mout.endl;

	Hi5Tree & dstGroup = resources.cartesianHi5(path);
	DataSet<CartesianDst> dstProduct(dstGroup);

	mout.debug(3) << "update geodata " << mout.endl;
	resources.composite.updateGeoData(); // TODO check if --plot cmds don't need

	// NEW 2020/06
	RootData<CartesianDst> dstRoot(resources.cartesianHi5);
	//CartesianODIM odim; // needed? yes, because Extract uses (Accumulator &), not Composite.
	dstRoot.odim.updateFromMap(resources.composite.odim);

	//mout.warn() << resources.composite.odim << mout.endl;

	ProductBase::completeEncoding(dstRoot.odim, resources.composite.getTargetEncoding());


	if (!resources.targetEncoding.empty()){
		ProductBase::completeEncoding(dstRoot.odim, resources.targetEncoding);
		// odim.setValues(resources.targetEncoding, '=');
		resources.targetEncoding.clear();
	}


	//
	//mout.warn() << "composite: " << resources.composite.odim << mout.endl;
	//mout.warn() << "composite: " << resources.composite << mout.endl;
	//mout.note() << "dst odim: " << odim << mout.endl;
	mout.debug(1) << "extracting..." << mout.endl;

	resources.composite.extract(dstRoot.odim, dstProduct, channels);

	//mout.warn() << "extracted data: " << dstProduct << mout.endl; // .getFirstData().data
	// CONSIDER
	drain::VariableMap & how = dstRoot.getHow();

	// CHECK success (order counts) ?
	// ODIM::copyToH5<ODIMPathElem::ROOT>(odim, resources.cartesianHi5);

	//drain::VariableMap & how = resources.cartesianHi5["how"].data.attributes;

	how["software"]   = __RACK__;
	how["sw_version"] = __RACK_VERSION__;
	// Non-standard
	how["tags"] = resources.composite.nodeMap.toStr(':');

	// Non-standard
	//drain::VariableMap & where = resources.cartesianHi5["where"].data.attributes; // dstGroup
	drain::VariableMap & where = dstRoot.getWhere();
	where["BBOX"].setType(typeid(double));
	where["BBOX"] = resources.composite.getBoundingBoxD().toStr(); // todo get vector?

	where["BBOX_data"].setType(typeid(double));
	const drain::Rectangle<double> & bboxDataD = resources.composite.getDataExtentD();
	where["BBOX_data"] = bboxDataD.toStr();

	drain::Rectangle<int> bboxDataPix;
	resources.composite.deg2pix(bboxDataD.lowerLeft, bboxDataPix.lowerLeft);
	resources.composite.deg2pix(bboxDataD.upperRight, bboxDataPix.upperRight);
	where["BBOX_data_pix"].setType(typeid(short int));
	where["BBOX_data_pix"] = bboxDataPix.toStr(); // clumsy
	// where["BBOX_data_pix"] = bboxDataPix;

	where["BBOX_overlap"].setType(typeid(double));
	where["BBOX_overlap"] = resources.composite.getDataOverlapD().toStr();


	DataTools::updateCoordinatePolicy(resources.cartesianHi5, RackResources::limit);
	DataTools::updateInternalAttributes(resources.cartesianHi5);

	resources.currentHi5 = &resources.cartesianHi5;
	resources.currentImage = NULL;
	resources.currentGrayImage = NULL;

	//resources.composite.odim.quantity
	if (dstProduct.has(resources.composite.odim.quantity)){
		Data<CartesianDst> & dstData = dstProduct.getData(resources.composite.odim.quantity); // OR: by odim.quantity
		resources.currentImage = & dstData.data;
		resources.currentGrayImage = resources.currentImage;
		if (resources.currentImage->isEmpty()){
			mout.warn() << "empty product data: " << dstData << mout.endl; // .getFirstData().data
		}
	}
	else {
		mout.warn() << "dstProduct does not have claimed quantity: " << resources.composite.odim.quantity << mout.endl; // .getFirstData().data
	}

	mout.debug() << "extracted quantity: " << dstProduct << mout.endl; // .getFirstData().data

	//dstGroup
	//

	/// For successfull file io:
	resources.errorFlags.unset(RackResources::INPUT_ERROR); // resources.inputOk = false;
	resources.errorFlags.unset(RackResources::DATA_ERROR); // resources.dataOk = false;
	//mout.warn() << "created" << mout.endl;

	VariableMap & statusMap = getRegistry().getStatusMap(); // getStatusMap(true);
	//statusMap["what:quantity"] = ;
	statusMap.updateFromMap(dstRoot.odim);
	//resources.getUpdatedStatusMap();

}




void CartesianSun::exec() const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	RackResources & resources = getResources();

	resources.cartesianHi5.clear();

	//RootData<CartesianDst> root(resources.cartesianHi5);
	DataSet<CartesianDst> dst(resources.cartesianHi5[ODIMPathElem::DATASET]);
	PlainData<CartesianDst> & dstData = dst.getData("SUNSHINE");
	getQuantityMap().setQuantityDefaults(dstData.odim, "PROB");
	dstData.odim.quantity = "SUNSHINE";

	const size_t width  = resources.composite.getFrameWidth();
	const size_t height = resources.composite.getFrameHeight();
	dstData.setGeometry(width, height);


	Sun sun(timestamp);
	dstData.odim.setTime(timestamp);

	resources.composite.updateScaling();

	mout.debug(1) << "main" << mout.endl;
	double lat, lon;

	for (size_t j = 0; j < height; ++j) {

		for (size_t i = 0; i < width; ++i) {

			resources.composite.pix2rad(i,j, lon,lat);
			sun.setLocation(lon, lat);
			if (sun.elev > 0.0)
				dstData.data.put(i, j, dstData.odim.scaleInverse(sin(sun.elev)));
			else
				dstData.data.put(i, j, dstData.odim.undetect); // This could be conditional
		}
	}


	// Still "borrowing" composite, yet not one.
	dstData.odim.updateGeoInfo(resources.composite);

	ODIM::copyToH5<ODIMPathElem::ROOT>(dstData.odim, resources.cartesianHi5); // od

	resources.currentHi5 = &resources.cartesianHi5;
	DataTools::updateInternalAttributes(resources.cartesianHi5);
	// resources.cartesianHi5[ODIMPathElem::WHAT].data.attributes["source"] = "fi";

}


}  // namespace rack::

