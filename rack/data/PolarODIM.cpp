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

#include "PolarODIM.h"

namespace rack {



void PolarODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	if (initialize & ROOT){
		reference("what:object", object = "PVOL");  // // or SCAN...
		reference("where:lon", lon = 0.0);
		reference("where:lat", lat = 0.0);
		reference("where:height", height = 0.0);
		reference("how:freeze", freeze = 10.0);
	}

	if (initialize & DATASET){
		reference("where:nbins",  nbins = 0L);
		reference("where:nrays",  nrays = 0L);
		reference("where:rscale", rscale = 0.0);
		reference("where:elangle", elangle = 0.0);
		reference("where:rstart", rstart = 0.0);
		reference("where:a1gate", a1gate = 0L);
		reference("where:startaz", startaz = 0.0);
		reference("where:stopaz",   stopaz = 0.0);
		reference("how:wavelength", wavelength = 0.0);
		reference("how:highprf", highprf = 0.0);
		reference("how:lowprf", lowprf = 0.0);
	}

	if (initialize & DATA){
	}

}






}  // namespace rack


