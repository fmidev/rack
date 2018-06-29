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

// Rack
