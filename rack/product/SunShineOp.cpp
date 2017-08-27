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
#include <drain/image/File.h>

//#include "andre/AndreOp.h"
#include "radar/Geometry.h"

#include "radar/Sun.h"
//#include "radar/RadarDataPicker.h"


#include "SunShineOp.h"




namespace rack
{

using namespace drain::image;


void SunShineOp::processData(const Data<PolarSrc> & src, Data<PolarDst> & dst) const {

	drain::MonitorSource mout(name, __FUNCTION__);

	mout.debug() << "Start"  << mout.endl;

	if (odim.type.empty())
		mout.error() << "Unset data storage type."  << mout.endl;

	//ProductOp::applyODIM();
	mout.note() << "odim: " << odim << mout.endl;
	setGeometry(odim, dst);
	mout.warn() << dst << mout.endl;

	//ProductOp::applyODIM(dst.odim, odim);

	/*
	dst.odim.updateFromMap(src.odim);

	// const std::type_info &t = drain::Type::getType(odim.type.at(0));
	// dst.data.setType(t);
	dst.odim.quantity = odim.quantity;
	dst.data.setType<unsigned char>();
	dst.odim.setTypeDefaults();
	dst.odim.gain     = odim.gain;
	dst.odim.offset   = odim.offset;

	dst.odim.nbins  = odim.nbins;
	dst.odim.nrays  = odim.nrays;
	dst.odim.rscale = odim.rscale;

	dst.data.setGeometry(dst.odim.nbins, dst.odim.nrays);

	*/

	//dst.odim.rscale = (static_cast<double>(src.odim.nbins) * src.odim.rscale + src.odim.rstart) / static_cast<double>(dst.odim.nbins);
	mout.debug(1) << "target nbins:" << dst.odim.nbins << " rscale:" << dst.odim.rscale << mout.endl;

	// const double max = dst.data.getMax<double>();
	// const double gainMetres = 1000*odim.gain;
	// const double eta = src.odim.elangle * DEG2RAD;
	// double h;


	RadarProj4 proj(src.odim.lon, src.odim.lat);
	proj.setProjectionDst("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_def");

	mout.debug() << proj << mout.endl;

	//std::string timestamp = src.odim.date + src.odim.time.substr(0,4);
	//mout.warn() << "timestamp " << timestamp << " <- " << src.odim.date << ':' <<  src.odim.time << mout.endl;

	Sun sun(timestamp);
	//double azm;
	//double elev;

	double a;  // azm of the beam
	double r;  // range of the bin
	double lon;
	double lat;

	// proj.projectInv(0.0, 0.0, lon, lat);
	// mout.warn() << "proj\t" << (180.0*lon/M_PI) << ',' << (180.0*lat/M_PI) << mout.endl;

	mout.debug(1) << "main" << mout.endl;


	for (long int j = 0; j < dst.odim.nrays; ++j) {

		a = dst.odim.getAzimuth(j);

		for (long int i = 0; i < dst.odim.nbins; ++i) {
			//std::cerr << i << '\t' << ground << " m\t h=" << h << " >" << h/odim.gain << " m\n";
			r = dst.odim.getBinDistance(i);
			//proj.projectInv(r*sin(a), r*cos(a), lon, lat);
			proj.projectFwd(r*sin(a), r*cos(a), lon, lat);
			sun.setLocation(lon, lat);
			//Sun::getSunPos(timestamp, lon, lat, azm, elev);
			//if ((i==j) && ((i&7) ==0))
			//	mout.warn() << i << '\t' << (lon*180/M_PI) << ',' << (lat*180/M_PI) << '\t' << (azm*180/M_PI) << ',' << sin(elev) << mout.endl;
			if (sun.elev > 0.0)
				dst.data.put(i, j, dst.odim.scaleInverse(sin(sun.elev)));
			else
				dst.data.put(i, j, dst.odim.undetect);
		}
	}

	dst.odim.product = odim.product;
	dst.odim.prodpar = odim.prodpar;
	dst.updateTree();

	//mout.warn() << timestamp << mout.endl;

}


}

// Rack
