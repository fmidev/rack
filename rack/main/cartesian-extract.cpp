/**


    Copyright 2006 - 2010   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

   Created on: Sep 30, 2010
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


