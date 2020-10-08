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

#include "hi5/Hi5.h"

#include "resources.h"
#include "data/SourceODIM.h"

using namespace drain;
using namespace drain::image;

namespace rack {

const CoordinatePolicy RackResources::polarLeft(CoordinatePolicy::POLAR, CoordinatePolicy::WRAP, CoordinatePolicy::LIMIT, CoordinatePolicy::WRAP);

const CoordinatePolicy RackResources::limit(CoordinatePolicy::LIMIT, CoordinatePolicy::LIMIT, CoordinatePolicy::LIMIT,CoordinatePolicy::LIMIT);

const drain::Flags::value_t RackResources::INPUT_ERROR     = 1;
const drain::Flags::value_t RackResources::DATA_ERROR      = 2;
const drain::Flags::value_t RackResources::METADATA_ERROR  = 4;
const drain::Flags::value_t RackResources::OUTPUT_ERROR    = 8;
const drain::Flags::value_t RackResources::PARAMETER_ERROR = 16;

//drain::Logger RackResources::mout("racklet"); inputOk(true), dataOk(true),
RackResources::RackResources() : currentHi5(&inputHi5), currentPolarHi5(&inputHi5), currentImage(NULL),
		currentGrayImage(NULL), scriptExec(scriptParser.script), errorFlags(errorFlagValue, errorFlagDict, ',') { //inputSelect(0),
	polarAccumulator.setMethod("WAVG");
	andreSelect = "dataset1,count=1";
	errorFlagDict.add("INPUT",     INPUT_ERROR);
	errorFlagDict.add("METADATA",  METADATA_ERROR);
	errorFlagDict.add("DATA",      DATA_ERROR);
	errorFlagDict.add("OUTPUT",    OUTPUT_ERROR);
	errorFlagDict.add("PARAMETER", PARAMETER_ERROR);
	errorFlags.reset();
}

void RackResources::setSource(Hi5Tree & dst, const drain::Command & cmd){

	drain::Logger mout("RackResources", __FUNCTION__);

	typedef std::map<void *, const drain::Command *> sourceMap;
	static sourceMap m;

	if (m[&dst] != &cmd){
		mout.debug() << "Cleared dst for " << cmd.getName() << mout.endl;
		dst.clear();
	}

	m[&dst] = &cmd;

}

drain::VariableMap & RackResources::getUpdatedStatusMap() {

	drain::Logger mout(__FUNCTION__, __FILE__);

	VariableMap & statusMap = getRegistry().getStatusMap(true); // comes with updated commands (NEW)

	/// Step 1: copy current H5 metadata (what, where, how)
	//DataSelector selector("data[0-9]+");
	DataSelector selector(ODIMPathElem::DATA);
	// selector.pathMatcher << ODIMPathElemMatcher(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
	// selector.pathMatcher.setElems(ODIMPathElem::DATA);
	selector.setParameters(select);
	selector.count = 1; // warn if not 1?
	ODIMPath path;
	//selector.getPathNEW(*currentHi5, path, ODIMPathElem::DATA | ODIMPathElem::QUALITY);
	selector.getPath3(*currentHi5, path);

	if (path.empty()){
		mout.note() << "no data groups found with selector '" << select << "'" << mout.endl;
		//mout.debug(4) << " currentHi5:\n" << *currentHi5 << mout.endl;
	}

	//else {
	mout.debug() << "using path=" << path << mout.endl;
	//mout.note() << (*currentHi5)(path)[ODIMPathElem::ARRAY].data.dataSet << mout.endl;
	DataTools::getAttributes(*currentHi5, path, statusMap);
	// mout.debug() << statusMap << mout.endl;
	//}

	/// Split what:source to separate fields
	//mout.warn() << "eka"  << mout.endl;
	const SourceODIM sourceODIM(statusMap["what:source"].toStr());

	//mout.warn() << "importMap"  << mout.endl;
	statusMap.importCastableMap(sourceODIM);

	//mout.warn() << "PolarODIM" << mout.endl;
	const PolarODIM odim(statusMap);
	//mout.warn() << odim << mout.endl;
	statusMap["how:NI"] = odim.getNyquist();

	/// Miscellaneous
	statusMap["version"] = __RACK_VERSION__;
	//statusMap["inputOk"] = static_cast<int>(inputOk);
	statusMap["errorFlags"] << errorFlags;
	// statusMap["accumulator"] = acc.toStr();
	statusMap["composite"] = composite.toStr();
	statusMap["andreSelect"] = andreSelect;

	getImageInfo(&colorImage, statusMap["img:colorImage"]);
	getImageInfo(&grayImage, statusMap["img:grayImage"]);
	getImageInfo(currentImage, statusMap["img:currentImage"]);
	getImageInfo(currentGrayImage, statusMap["img:currentGrayImage"]);

	return statusMap;
}

void RackResources::initComposite() {

	drain::Logger mout(__FUNCTION__, __FILE__);

	RackResources & resources = *this; //getResources();

	/// Set data selector
	if (!resources.select.empty()){
		resources.composite.dataSelector.setParameters(resources.select);
		resources.select.clear();
		// resources.composite.odim.quantity.clear();
	}

	if ((resources.composite.getFrameWidth() == 0) || (resources.composite.getFrameHeight() == 0)){
		resources.composite.setGeometry(500,500); // frame only
		mout.warn() << "size unset, applying " << resources.composite.getFrameWidth() << ',' << resources.composite.getFrameHeight() << mout.endl;
	}
	resources.composite.allocate();

	if (!resources.targetEncoding.empty()){
		resources.composite.setTargetEncoding(resources.targetEncoding);
		mout.debug() << "target encoding: " << resources.composite.getTargetEncoding() << mout.endl;
		resources.targetEncoding.clear();
	}


}




/*
void RackResources::getImageInfo(const char *label, const drain::image::Image *ptr, VariableMap & statusMap){
	std::stringstream sstr;
	if (ptr){
		ptr->toOStr(sstr);
	}
	else {
		sstr << "NULL";
	}
	statusMap[label] = sstr.str();
}
*/

void RackResources::getImageInfo(const drain::image::Image *ptr, Variable & entry) const {
	std::stringstream sstr;
	if (ptr){
		ptr->toOStr(sstr);
	}
	else {
		sstr << "NULL";
	}
	entry = sstr.str();
}

bool RackResources::setCurrentImage(const DataSelector & imageSelector){

	drain::Logger mout(__FUNCTION__, __FILE__);

	// NOTE  ODIMPathElem::ARRAY ie. "/data" cannot be searched, so it is added under DATA or QUALITY path.

	ODIMPath path;

	if (imageSelector.getPath3(*currentHi5, path)){

		mout.info() << "derived path: '" << path << "'" << mout.endl;

		drain::image::Image & img = (*currentHi5)(path)[ODIMPathElem::ARRAY].data.dataSet;
		mout.info() << img.getProperties().get("what:quantity", "?") << ", scaling: " << img.getScaling() << "  " << img << mout.endl;

		if (!img.isEmpty()){
			DataTools::getAttributes(*currentHi5, path, img.properties); // may be unneeded (or for image processing ops?)
		}
		else {
			mout.debug() << "empty image: " << img.properties << mout.endl;
			mout.warn()  << "empty image: " << img << mout.endl;
		}

		// Hence, image may also be empty.
		currentImage     = & img;
		currentGrayImage = & img;

		return true;

	}
	else {
		// if (path.empty()){
		mout.warn() << "no paths found with " << imageSelector << ", skipping..." << mout.endl;
		return false;
	}



}

bool RackResources::guessDatasetGroup(ODIMPathElem & pathElem) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	pathElem.set(ODIMPathElem::DATASET, 1);
	//ODIMPathElem parent(ODIMPathElem::DATASET, 1);
	if (ProductBase::appendResults.is(ODIMPathElem::DATASET)){
		DataSelector::getNextChild(*currentHi5, pathElem);
		//path << parent;

		if (pathElem == currentPath.front()){
			mout.note() << "this path could have been set automatically: " << currentPath << mout.endl;
		}
		return true;
	}
	else if (ProductBase::appendResults.is(ODIMPathElem::DATA)){
		DataSelector::getLastChild(*currentHi5, pathElem);
		if (pathElem.index == 0){
			pathElem.index = 1;
		}
		//path << parent;
		if (pathElem == currentPath.front()){
			mout.note() << "this path could have been set automatically: " << currentPath << mout.endl;
		}
		return true;
	}
	else {
		// path << parent; // DATASET1
		//path = currentPath;
		return false;
	}
}


/// Default instance
RackResources & getResources() {
	static RackResources resources;
	return resources;
}


} // rack::

