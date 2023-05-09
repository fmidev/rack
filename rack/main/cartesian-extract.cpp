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
#include "data/DataCoder.h"
#include "hi5/Hi5.h"

//#include "radar/Coordinates.h"
#include "radar/Composite.h"
#include "radar/Sun.h"

#include "cartesian-extract.h"


namespace rack {



void CartesianSun::exec() const {


	//RackResources & resources = getResources();
	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	ctx.cartesianHi5.clear();

	//RootData<CartesianDst> root(ctx.cartesianHi5);
	DataSet<CartesianDst> dst(ctx.cartesianHi5[ODIMPathElem::DATASET]);
	PlainData<CartesianDst> & dstData = dst.getData("SUNSHINE");
	getQuantityMap().setQuantityDefaults(dstData.odim, "PROB");
	dstData.odim.quantity = "SUNSHINE";

	const size_t width  = ctx.composite.getFrameWidth();
	const size_t height = ctx.composite.getFrameHeight();
	dstData.setGeometry(width, height);


	Sun sun(timestamp);
	dstData.odim.setTime(timestamp);

	mout.warn() << "Check bbox and scaling? " << ctx.composite << mout.endl;
	//ctx.composite.updateScaling();

	mout.debug2() << "main" << mout.endl;
	double lat, lon;

	for (size_t j = 0; j < height; ++j) {

		for (size_t i = 0; i < width; ++i) {

			ctx.composite.pix2rad(i,j, lon,lat);
			sun.setLocation(lon, lat);
			if (sun.elev > 0.0)
				dstData.data.put(i, j, dstData.odim.scaleInverse(sin(sun.elev)));
			else
				dstData.data.put(i, j, dstData.odim.undetect); // This could be conditional
		}
	}


	// Still "borrowing" composite, yet not one.
	dstData.odim.updateGeoInfo(ctx.composite);

	ODIM::copyToH5<ODIMPathElem::ROOT>(dstData.odim, ctx.cartesianHi5); // od

	ctx.currentHi5 = &ctx.cartesianHi5;
	DataTools::updateInternalAttributes(ctx.cartesianHi5);
	// ctx.cartesianHi5[ODIMPathElem::WHAT].data.attributes["source"] = "fi";

}


}  // namespace rack::

