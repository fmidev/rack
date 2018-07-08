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

#include "BeamAltitudeOp.h"
//#include "DataSelector.h"



namespace rack
{

using namespace drain::image;


void BeamAltitudeOp::processData(const Data<PolarSrc> & src, Data<PolarDst> &dst) const {

	drain::Logger mout(name,__FUNCTION__);

	//if (odim.type.empty())
	//	mout.error() << "Unset data storage type."  << mout.endl;

	// dst.odim.updateFromMap(src.odim);

	/*
	//dst.odim = src.odim; // elangle, starttime etc.
	dst.odim.gain = 123.456;
	mout.warn() << "dst.odim" << dst.odim << mout.endl;
	mout.error() << "dst.odim.gain" << dst.odim.gain << mout.endl;
	*/
	/*
	const std::type_info &t = drain::Type::getType(odim.type.at(0));
	dst.data.setType(t);
	dst.odim.setTypeDefaults(t);
	dst.odim.gain     = odim.gain;
	dst.odim.offset   = odim.offset;
	//dst.odim.nodata   = odim.nodata;
	//dst.odim.undetect = odim.undetect;
	dst.odim.quantity = "HGHT";
	dst.odim.nrays = 1;
	dst.odim.rscale = (static_cast<double>(src.odim.nbins) * src.odim.rscale + src.odim.rstart) / static_cast<double>(dst.odim.nbins);
	 */

	//dst.odim.elangle = src.odim.elangle;

	//mout.debug(1) << "target nbins:" << dst.odim.nbins << " rscale:" << dst.odim.rscale << mout.endl;

	//dst.data.setGeometry(dst.odim.nbins, 1);
	const double max = drain::Type::call<drain::typeMax, double>(dst.data.getType());  //dst.data.scaling.getMax<double>();

	const double gainMetres = 1000*odim.gain;
	const double eta = src.odim.elangle * drain::DEG2RAD;
	double h;
	for (long int i = 0; i < dst.odim.nbins; ++i) {
		//std::cerr << i << '\t' << ground << " m\t h=" << h << " >" << h/odim.gain << " m\n";
		h = Geometry::heightFromEtaGround(eta, i*dst.odim.rscale)/gainMetres;
		if (h < max)
			dst.data.put(i, h);
		else
			dst.data.put(i, dst.odim.undetect); // quality?
	}


}


}
