/**

    Copyright 2006 - 2014 Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)

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

    Created on: Nov 17, 2014
    Author: mpeura
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
