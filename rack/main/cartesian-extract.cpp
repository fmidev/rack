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



#include <drain/util/Fuzzy.h>

#include <drain/image/File.h>
#include <drain/imageops/DistanceTransformFillOp.h>
#include <drain/imageops/RecursiveRepairerOp.h>


#include "data/DataCoder.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"

#include "radar/Coordinates.h"
#include "radar/Composite.h"
//#include "radar/Extractor.h"


#include "cartesian-extract.h"


namespace rack {



void CartesianExtract::extract(const std::string & channels) const {

	drain::Logger mout(name, __FUNCTION__);

	RackResources & resources = getResources();


	// Append results - why not, but Cartesian was typically used for subcompositing
	// resources.cartesianHi5.clear();
	resources.setSource(resources.cartesianHi5, *this);

	//ODIMPath path("dataset1");
	ODIMPath path;

	ODIMPathElem parent(BaseODIM::DATASET, 1);
	if (ProductBase::appendResults.is(BaseODIM::DATASET))
		DataTools::getNextChild(resources.cartesianHi5, parent);
	else if (ProductBase::appendResults.is(BaseODIM::DATA))
		DataTools::getLastChild(resources.cartesianHi5, parent);

	path << parent;

	//if (ProductBase::appendResults.is(BaseODIM::DATA))
	//	DataTools::getNextChild(resources.cartesianHi5[parent], parent);


	//if (!DataTools::getNextDescendant(resources.cartesianHi5, ProductBase::appendResults.getType(), path))
	//	path.push_back(BaseODIM::DATASET);



	/*
	std::string path = "dataset1";
	if (ProductBase::appendResults == "dataset"){
		if (DataSelector::getNextOrdinalPath(resources.cartesianHi5, "dataset([0-9]+)$", path))
			mout.note() << "appending, path=" << path << mout.endl;
	}
	else if (ProductBase::appendResults == "data"){
		if (DataSelector::getLastOrdinalPath(resources.cartesianHi5, "dataset([0-9]+)$", path))
			mout.note() << "appending, path=" << path << mout.endl;
	}
	*/


	HI5TREE & dstGroup = resources.cartesianHi5(path);
	DataSet<CartesianDst> dstProduct(dstGroup);


	resources.composite.updateGeoData(); // TODO check if --plot cmds don't need

	CartesianODIM odim; // needed? yes, because Extract uses (Accumulator &), not Composite.
	odim.updateFromMap(resources.composite.odim);

	//mout.warn() << resources.composite.odim << mout.endl;

	ProductBase::handleEncodingRequest(odim, resources.composite.getTargetEncoding());


	if (!resources.targetEncoding.empty()){
		ProductBase::handleEncodingRequest(odim, resources.targetEncoding);
		// odim.setValues(resources.targetEncoding, '=');
		resources.targetEncoding.clear();
	}


	//
	//mout.warn() << "composite: " << resources.composite.odim << mout.endl;
	//mout.warn() << "composite: " << resources.composite << mout.endl;
	//mout.note() << "dst odim: " << odim << mout.endl;
	//mout.debug() << "extracting..." << mout.endl;
	resources.composite.extract(odim, dstProduct, channels);

	//mout.warn() << "extracted data: " << dstProduct.getFirstData().data << mout.endl;

	ODIM::copyToH5<ODIM::ROOT>(odim, resources.cartesianHi5); // odim.copyToRoot(resources.cartesianHi5);

	drain::VariableMap & how = dstGroup["how"].data.attributes;
	how["software"] = __RACK__;
	// Non-standard
	how["tags"] = resources.composite.nodeMap.toStr(':');

	// Non-standard
	drain::VariableMap & where = resources.cartesianHi5["where"].data.attributes; // dstGroup
	where["BBOX"].setType(typeid(double));
	where["BBOX"] = resources.composite.getBoundingBoxD().toStr();
	where["BBOX_data"].setType(typeid(double));
	where["BBOX_data"] = resources.composite.getDataExtentD().toStr();
	where["BBOX_overlap"].setType(typeid(double));
	where["BBOX_overlap"] = resources.composite.getDataOverlapD().toStr();


	DataTools::updateCoordinatePolicy(resources.cartesianHi5, RackResources::limit);
	DataTools::updateAttributes(resources.cartesianHi5);

	resources.currentHi5 = &resources.cartesianHi5;
	resources.currentImage = NULL;
	resources.currentGrayImage = NULL;

	/// For successfull file io:
	resources.inputOk = true;
	resources.dataOk = true;
	//mout.warn() << "created" << mout.endl;
}






}  // namespace rack::



// Rack
