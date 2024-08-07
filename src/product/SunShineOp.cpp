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
#include <drain/image/ImageFile.h>
#include <drain/util/Geo.h>
//#include "radar/Geometry.h"
#include "radar/RadarProj.h"

#include "radar/Sun.h"
//#include "radar/RadarDataPicker.h"


#include "SunShineOp.h"




namespace rack
{

using namespace drain::image;


void SunShineOp::processData(const Data<PolarSrc> & src, Data<PolarDst> & dst) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("Start"  );

	if (odim.type.empty())
		mout.error("Unset data storage type."  );

	//ProductOp::applyODIM();
	mout.note(" odim: " , odim );
	//mout.note("dodim: " , dst.odim );
	setGeometry(odim, dst);
	//ProductOp::applyODIM(dst.odim, odim);

	//dst.odim.rscale = (static_cast<double>(src.odim.geometry.width) * src.odim.rscale + src.odim.rstart) / static_cast<double>(dst.odim.geometry.width);
	mout.debug2("target nbins:" , dst.odim.area.width , " rscale:" , dst.odim.rscale );

	RadarProj4 proj(src.odim.lon, src.odim.lat);
	proj.setProjectionDst("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_def");

	mout.debug(proj );

	//std::string timestamp = src.odim.date + src.odim.time.substr(0,4);
	//mout.warn("timestamp " , timestamp , " <- " , src.odim.date , ':' ,  src.odim.time );

	Sun sun(timestamp);
	dst.odim.setTime(timestamp);
	dst.odim.updateLenient(odim);
	// dst.odim.updateLenient() //.setTypeDefaults
	// mout.warn("odim " , dst.odim.date , '-' , dst.odim.time );

	double a;  // azm of the beam
	double r;  // range of the bin
	double lon;
	double lat;

	// proj.projectInv(0.0, 0.0, lon, lat);
	// mout.warn("proj\t" , (180.0*lon/M_PI) , ',' , (180.0*lat/M_PI) );

	mout.debug2("main" );


	for (unsigned int j = 0; j < dst.odim.area.height; ++j) {

		a = dst.odim.getAzimuth(j);

		for (unsigned int i = 0; i < dst.odim.area.width; ++i) {
			//std::cerr << i << '\t' << ground << " m\t h=" << h << " >" << h/odim.scaling.scale << " m\n";
			r = dst.odim.getBinDistance(i);
			proj.projectFwd(r*sin(a), r*cos(a), lon, lat);
			sun.setLocation(lon, lat);
			if (sun.elev > 0.0)
				dst.data.put(i, j, dst.odim.scaleInverse(sin(sun.elev)));
			else
				dst.data.put(i, j, dst.odim.undetect); // This could be conditional
		}
	}

	//mout.note("dst odim: " , dst.odim );

	//dst.odim.product = "MIKA"; // odim.product;
	//dst.odim.undetect = 0.123456789; // odim.product;
	//dst.odim.prodpar = odim.prodpar;
	//dst.updateTree2();
	//mout.note("dst odim: " , dst.odim );

}


}

// Rack
