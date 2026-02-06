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

#include <drain/Log.h>
#include "VerticalODIM.h"

namespace rack {


void VerticalCrossSectionODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	if (initialize & ODIMPathElem::ROOT){

		// compiler test...
#ifdef INT64_C
		link("where:xsize", xsize = INT64_C(0));
		link("where:ysize", ysize = INT64_C(0));
#else
		link("where:xsize", xsize = 0L);
		link("where:ysize", ysize = 0L);
#endif

		link("where:xscale", xscale = 0.0);
		link("where:yscale", yscale = 0.0);

		//link("where:minheight",  minheight = 0.0);
		//link("where:maxheight",  maxheight = 0.0);
		link("where:minheight",  altitudeRange.min = 0.0);
		link("where:maxheight",  altitudeRange.max = 0.0);

		link("how:NI",  NI = 0.0);
	}

	if (initialize & ODIMPathElem::DATASET){
	}


	if (initialize & ODIMPathElem::DATA){
	}

}




void  VerticalProfileODIM::init(group_t initialize){ // n::referenceRootAttrs(){

	object = "XSEC";

	if (initialize & ODIMPathElem::ROOT){
		link("where:lon", lon = 0.0);
		link("where:lat", lat = 0.0);
		altitudeRange = 0.0;
		link("where:height", altitudeRange.min); // note height vs. altitude
	}

	if (initialize & ODIMPathElem::DATASET){
		link("where:levels",  levels = 0L);
		link("where:interval",  interval = 0.0);

		//link("how:minRange", minRange = 0.0);// where or how??
		//link("how:range", range = 0.0);// where or how??
		// Product will use range (Range object)
		link("how:minRange", distanceRange.min = 0.0);// where or how??
		link("how:range",    distanceRange.max = 0.0);// where or how??

		link("where:startaz", azmRange.min = 0.0);
		link("where:stopaz",  azmRange.max = 0.0);
		// link("where:startaz", startaz = 0.0);
		// link("where:stopaz", stopaz = 0.0);

		link("where:azSlots", azSlots = 1L);
	}

	if (initialize & ODIMPathElem::DATA){
	}

}



void RhiODIM::init(group_t initialize){ //referenceRootAttrs(){

	object = "XSEC";

	if (initialize & ODIMPathElem::ROOT){
		link("where:lon", lon = 0.0);
		link("where:lat", lat = 0.0);
	}

	if (initialize & ODIMPathElem::DATASET){
		link("where:minRange", range.min = 0.0);  // where or how??
		link("where:range",    range.max = 0.0);  // where or how??
		link("where:az_angle", az_angle = 0.0);
		// link("where:angles",   angles = -1.0);
		//link("where:angles", angles);
	}

	if (initialize & ODIMPathElem::DATA){
	}

}




}  // namespace rack



// Rack
