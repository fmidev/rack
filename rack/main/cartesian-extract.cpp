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

#include <drain/image/DistanceTransformFillOp.h>
#include <drain/image/File.h>
#include <drain/image/RecursiveRepairerOp.h>

#include "data/DataCoder.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"

#include "radar/Coordinates.h"
#include "radar/Composite.h"
//#include "radar/Extractor.h"


#include "cartesian-extract.h"


namespace rack {



void CartesianExtract::extract(const std::string & channels) const {

	drain::MonitorSource mout(name, __FUNCTION__);

	RackResources & resources = getResources();


	// Append results - why not, but Cartesian was typically used for subcompositing
	// resources.cartesianHi5.clear();
	resources.setSource(resources.cartesianHi5, *this);

	std::string path = "dataset1";
	if (ProductOp::appendResults){
		if (DataSelector::getNextOrdinalPath(resources.cartesianHi5, "dataset([0-9]+)$", path))
			mout.note() << "appended, path=" << path << mout.endl;
	}

	HI5TREE & dstGroup = resources.cartesianHi5[path];
	DataSetDst<CartesianDst> dstProduct(dstGroup);


	resources.composite.updateGeoData(); // TODO check if --plot cmds don't need

	CartesianODIM odim; // needed? yes, because Extract uses (Accumulator &), not Composite.
	odim.updateFromMap(resources.composite.odim);



	ProductOp::handleEncodingRequest(odim, resources.composite.getTargetEncoding());


	if (!resources.targetEncoding.empty()){
		ProductOp::handleEncodingRequest(odim, resources.targetEncoding);
		// odim.setValues(resources.targetEncoding, '=');
		resources.targetEncoding.clear();
	}


	//mout.warn() << "composite: " << resources.composite << mout.endl;
	//mout.note() << "dst odim: " << odim << mout.endl;
	//mout.debug() << "extracting..." << mout.endl;
	resources.composite.extract(odim, dstProduct, channels);

	//mout.warn() << "extracted data: " << dstProduct.getFirstData().data << mout.endl;

	odim.copyToRoot(resources.cartesianHi5);

	dstGroup["how"].data.attributes["software"] = __RACK__;
	// Non-standard
	dstGroup["how"].data.attributes["tags"] = resources.composite.nodeMap.toStr(':');
	dstGroup["where"].data.attributes["BBOX"]      = resources.composite.getBoundingBoxD().toStr();
	dstGroup["where"].data.attributes["BBOX_data"] = resources.composite.getDataExtentD().toStr();


	RackResources::updateCoordinatePolicy(resources.cartesianHi5, RackResources::limit);
	DataSelector::updateAttributes(resources.cartesianHi5);

	resources.currentHi5 = &resources.cartesianHi5;
	resources.currentImage = NULL;
	resources.currentGrayImage = NULL;

	resources.inputOk = true; // TODO
	//mout.warn() << "created" << mout.endl;
}






}  // namespace rack::



// Rack
