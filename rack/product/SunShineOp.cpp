/*

    Copyright 2010-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

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

	drain::Logger mout(name, __FUNCTION__);

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

	//@ dst.updateTree();

	//mout.warn() << timestamp << mout.endl;

}


}
