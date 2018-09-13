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
		currentGrayImage(NULL), inputSelect(0), scriptExec(scriptParser.script) {

}

void RackResources::setSource(HI5TREE & dst, const drain::Command & cmd){

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

	//RackResources & resources = getResources();

	// resources.select.clear(); don't clear, because status is used fro debugging.
	//CommandRegistry & reg = getRegistry();
	//VariableMap & statusMap = getRegistry().getStatusMap(true); // comes with updated commands (NEW)
	VariableMap & statusMap = getRegistry().getStatusMap(true); // comes with updated commands (NEW)

	/// Step 1: copy current H5 metadata (what, where, how)
	DataSelector selector("data[0-9]+");
	selector.setParameters(select);
	//std::list<std::string> l;
	std::string path;
	DataSelector::getPath(*currentHi5, selector, path);

	if (path.empty()){
		mout.note() << " currentHi5: no path for selector '" << select << "'" << mout.endl;
		mout.warn() << " currentHi5:\n" << *currentHi5 << mout.endl;
	}
	else {
		//const std::string & path = *l.begin();
		mout.debug(1) << "RackResources" << " path=" << path << mout.endl;
		DataTools::getAttributes(*currentHi5, path, statusMap);
		// mout.debug() << statusMap << mout.endl;
	}
	/// Split what:source to separate fields
	const SourceODIM sourceODIM(statusMap["what:source"].toStr());
	statusMap.importMap(sourceODIM); // was import

	const PolarODIM odim(statusMap);
	//mout.warn() << odim << mout.endl;
	statusMap["how:NI"] = odim.getNyquist();

	/// Miscellaneous
	statusMap["version"] = __RACK_VERSION__;
	statusMap["inputOk"] = static_cast<int>(inputOk);
	// statusMap["accumulator"] = acc.toStr();
	statusMap["composite"] = composite.toStr();
	statusMap["andreSelect"] = andreSelect;


	return statusMap;
}



bool RackResources::setCurrentImage(const DataSelector & imageSelector){

	drain::Logger mout("RackResources", __FUNCTION__);

	std::list<std::string> l;
	DataSelector::getPaths(*currentHi5, imageSelector, l); // todo getFirstData

	if (!l.empty()){

		const std::list<std::string>::const_iterator it = l.begin();
		mout.info() << "selected: " << *it << mout.endl;
		drain::image::Image & img = (*currentHi5)(*it).data.dataSet;
		if (!img.isEmpty()){
			DataTools::getAttributes(*currentHi5, *it, img.properties); // may be unneeded
			currentImage = &img;
			return true;
		}
		else {
			mout.warn() << "empty data in path: " << *it << mout.endl;
			return false;
		}
	}
	else {
		// if EXIT_ON_DATA_FAIL
		mout.warn() << "skipping, no image data found with selector " << imageSelector << mout.endl;
		return false;
	}


}

/// Default instance
RackResources & getResources() {
	static RackResources resources;
	return resources;
}




} /* namespace rack */

// Rack
