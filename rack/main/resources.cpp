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

//drain::Logger RackResources::mout("racklet");
RackResources::RackResources() : inputOk(true), dataOk(true), currentHi5(&inputHi5), currentPolarHi5(&inputHi5), currentImage(NULL),
		currentGrayImage(NULL), scriptExec(scriptParser.script) { //inputSelect(0),
	polarAccumulator.setMethod("WAVG");
	andreSelect = "dataset=1,count=1";
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

	drain::Logger mout("RackResources", __FUNCTION__);

	VariableMap & statusMap = getRegistry().getStatusMap(true); // comes with updated commands (NEW)

	/// Step 1: copy current H5 metadata (what, where, how)
	//DataSelector selector("data[0-9]+");
	DataSelector selector;
	selector.setParameters(select);
	selector.count = 1; // warn if not 1?
	ODIMPath path;
	selector.getPathNEW(*currentHi5, path, ODIMPathElem::DATA | ODIMPathElem::QUALITY);

	if (path.empty()){
		mout.note() << "no data groups found with selector '" << select << "'" << mout.endl;
		//mout.debug(4) << " currentHi5:\n" << *currentHi5 << mout.endl;
	}

	//else {
	mout.debug(1) << "using path=" << path << mout.endl;
	DataTools::getAttributes(*currentHi5, path, statusMap);
	// mout.debug() << statusMap << mout.endl;
	//}

	/// Split what:source to separate fields
	const SourceODIM sourceODIM(statusMap["what:source"].toStr());
	statusMap.importMap(sourceODIM);

	const PolarODIM odim(statusMap);
	//mout.warn() << odim << mout.endl;
	statusMap["how:NI"] = odim.getNyquist();

	/// Miscellaneous
	statusMap["version"] = __RACK_VERSION__;
	statusMap["inputOk"] = static_cast<int>(inputOk);
	// statusMap["accumulator"] = acc.toStr();
	statusMap["composite"] = composite.toStr();
	statusMap["andreSelect"] = andreSelect;

	getImageInfo("img:colorImage",   &colorImage, statusMap);
	getImageInfo("img:grayImage",    &grayImage,  statusMap);
	getImageInfo("img:currentImage",     currentImage,     statusMap);
	getImageInfo("img:currentGrayImage", currentGrayImage, statusMap);

	return statusMap;
}

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


bool RackResources::setCurrentImage(const DataSelector & imageSelector){

	drain::Logger mout("RackResources", __FUNCTION__);

	/* COPIED
	cmdImage.imageSelector.setParameters(resources.select);
	mout.debug() << "determining current gray image" << mout.endl;
	mout.debug(2) << cmdImage.imageSelector << mout.endl;
	ODIMPath path;
	cmdImage.imageSelector.getPathNEW(*resources.currentHi5, path, ODIMPathElem::DATA | ODIMPathElem::QUALITY);
	mout.debug(1) << "path: '" << path << "'" << mout.endl;
	resources.currentGrayImage = & (*resources.currentHi5)(path)[odimARRAY].data.dataSet;  // CREATES???
	resources.currentImage     =   resources.currentGrayImage;
	*/

	// NOTE  ODIMPathElem::ARRAY ie. "/data" cannot be searched, so it is added under DATA or QUALITY path.

	drain::Flags flags(ODIMPathElem::getDictionary());
	flags = ODIMPathElem::DATA | ODIMPathElem::QUALITY;
	ODIMPath path;
	bool result = imageSelector.getPathNEW(*currentHi5, path, flags);

	if (result){
		path << odimARRAY;
		currentPath = path;
	}
	else {
		mout.debug() << "no image data found with image selector: " << imageSelector << ", flags='" << flags << "'" << mout.endl;
		// EXIT_ON_DATA_FAIL  here?
	}

	mout.debug() << "derived path: '" << path << "'" << mout.endl;

	drain::image::Image & img = (*currentHi5)(path).data.dataSet;

	if (!img.isEmpty()){
		DataTools::getAttributes(*currentHi5, path, img.properties); // may be unneeded (or for image processing ops?)
	}

	// Hence, image may also be empty.
	currentImage     = & img;
	currentGrayImage = & img;

	return result;

	/*
	if (imageSelector.getPathNEW(*currentHi5, path, ODIMPathElem::DATA | ODIMPathElem::QUALITY)){

		path << ODIMPathElem(ODIMPathElem::ARRAY);
		//const ODIMPathList::const_iterator it = paths.begin();
		mout.info() << "selected: " << path << mout.endl;
		drain::image::Image & img = (*currentHi5)(path).data.dataSet;
		if (!img.isEmpty()){
			// mout.warn() << "selected: " << img.properties << mout.endl;
			DataTools::getAttributes(*currentHi5, path, img.properties); // may be unneeded
			currentImage     = & img;
			currentGrayImage = & img;
			//img.getCoordinatePolicy().
			//img.properties["coordinatePolicy"] = 3; //="1,2,3,4";
			mout.debug(1) << "selected: " << *currentImage << mout.endl;
			return true;
		}
		else {
			// consider
			dataOk = false;
			mout.warn() << "empty data in path: " << path << mout.endl;
			return false;
		}
	}
	else {
		// if EXIT_ON_DATA_FAIL
		mout.note() << "selector: " << imageSelector << mout.endl;
		mout.warn() << "no image data found in " << path << mout.endl;
		return false;
	}
	*/

}

bool RackResources::guessDatasetGroup(ODIMPathElem & pathElem) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

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

