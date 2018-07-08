/*


    Copyright 2011-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include <drain/util/Log.h>

#include "VerticalODIM.h"

namespace rack {


void VerticalCrossSectionODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	if (initialize & ROOT){

		// compiler test...
#ifdef INT64_C
		reference("where:xsize", xsize = INT64_C(0));
		reference("where:ysize", ysize = INT64_C(0));
#else
		reference("where:xsize", xsize = 0L);
		reference("where:ysize", ysize = 0L);
#endif

		reference("where:xscale", xscale = 0.0);
		reference("where:yscale", yscale = 0.0);

		reference("where:minheight",  minheight = 0.0);
		reference("where:maxheight",  maxheight = 0.0);

		reference("how:NI",  NI = 0.0);
	}

	if (initialize & DATASET){
	}


	if (initialize & DATA){
	}

}




void  VerticalProfileODIM::init(group_t initialize){ // n::referenceRootAttrs(){

	object = "XSEC";

	if (initialize & ROOT){
		reference("where:lon", lon = 0.0);
		reference("where:lat", lat = 0.0);
		reference("where:height", height = 0.0);
	}

	if (initialize & DATASET){
		reference("where:levels",  levels = 0L);
		reference("where:interval",  interval = 0.0);

		reference("how:minRange", minRange = 0.0);// where or how??
		reference("how:range", range = 0.0);// where or how??

		reference("where:startaz", startaz = 0.0);
		reference("where:stopaz", stopaz = 0.0);
		reference("where:azSlots", azSlots = 1L);
	}

	if (initialize & DATA){
	}

}



void RhiODIM::init(group_t initialize){ //referenceRootAttrs(){

	object = "XSEC";

	if (initialize & ROOT){
		reference("where:lon", lon = 0.0);
		reference("where:lat", lat = 0.0);
	}

	if (initialize & DATASET){
		reference("where:minRange", minRange = 0.0);  // where or how??
		reference("where:range", range = 0.0);// where or how??
		reference("where:az_angle", az_angle = 0.0);
		reference("where:angles", angles = 0.0);
	}

	if (initialize & DATA){
	}

}




}  // namespace rack


