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

#include "VerticalODIM.h"

namespace rack {


void VerticalCrossSectionODIM::init(){

	//std::set<std::string> & rootAttributes = getRootAttributes();
	std::set<std::string> & datasetAttributes = getDatasetAttributes();
	//std::set<std::string> & dataAttributes = getDatasetAttributes();

	// compiler test...
#ifdef INT64_C
	declare(datasetAttributes, "where:xsize", xsize = INT64_C(0));
	declare(datasetAttributes, "where:ysize", ysize = INT64_C(0));
#else
	declare(datasetAttributes, "where:xsize", xsize = 0L);
	declare(datasetAttributes, "where:ysize", ysize = 0L);
#endif

	declare(datasetAttributes, "where:xscale", xscale = 0.0);
	declare(datasetAttributes, "where:yscale", yscale = 0.0);


	declare(datasetAttributes, "where:minheight",  minheight = 0.0);
	declare(datasetAttributes, "where:maxheight",  maxheight = 0.0);

	declare(datasetAttributes, "how:NI",  NI = 0.0);

	/*declare(datasetAttributes, "how:minRange", minRange, 0.0);
	declare(datasetAttributes, "how:maxRange", maxRange, 0.0);

	declare(datasetAttributes, "where:startaz", startaz, 0.0);
	declare(datasetAttributes, "where:stopaz", stopaz,  0.0);
	declare(datasetAttributes, "where:azSlots", azSlots, 1L);
    */
}


void VerticalProfileODIM::init(){

	object = "VP";

	std::set<std::string> & rootAttributes = getRootAttributes();
	std::set<std::string> & datasetAttributes = getDatasetAttributes();
	//std::set<std::string> & dataAttributes = getDatasetAttributes();

	//ODIM::_init();

	declare(rootAttributes, "where:lon", lon = 0.0);
	declare(rootAttributes, "where:lat", lat = 0.0);
	declare(rootAttributes, "where:height", height = 0.0);


	declare(datasetAttributes, "where:levels",  levels = 0L);
	declare(datasetAttributes, "where:interval",  interval = 0.0);

	declare(datasetAttributes, "how:minRange", minRange = 0.0);
	declare(datasetAttributes, "how:range", range = 0.0);

	declare(datasetAttributes, "where:startaz", startaz = 0.0);
	declare(datasetAttributes, "where:stopaz", stopaz = 0.0);
	declare(datasetAttributes, "where:azSlots", azSlots = 1L);


}


void RhiODIM::init(){

	object = "XSEC";

	std::set<std::string> & rootAttributes = getRootAttributes();
	std::set<std::string> & datasetAttributes = getDatasetAttributes();
	//std::set<std::string> & dataAttributes = getDatasetAttributes();


	declare(rootAttributes, "where:lon", lon = 0.0);
	declare(rootAttributes, "where:lat", lat = 0.0);

	declare(datasetAttributes, "where:minRange", minRange = 0.0);
	declare(datasetAttributes, "where:range", range = 0.0);

	declare(datasetAttributes, "where:az_angle", az_angle = 0.0);
	declare(datasetAttributes, "where:angles", angles = 0.0);

	/*
	declare(datasetAttributes, "where:startaz", startaz, 0.0);
	declare(datasetAttributes, "where:stopaz", stopaz,  0.0);
	declare(datasetAttributes, "where:azSlots", azSlots, 1L);
    */
}


}  // namespace rack



// Rack
