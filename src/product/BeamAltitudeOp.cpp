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
#include "radar/Geometry.h"

#include "BeamAltitudeOp.h"
//#include "DataSelector.h"



namespace rack
{


BeamAltitudeOp::BeamAltitudeOp() : // const std::string & type="S", double gain=100.0, long int nbins=0, double rscale=0) :
				PolarProductOp(__FUNCTION__,"Computes the altitude at each bin")
{

	parameters.link("aboveSeaLevel", this->aboveSeaLevel = true, "false=radar site|true=sea level");

	odim.product  = "ALTITUDE";
	odim.quantity = "HGHT";
	odim.area.height = 1;

	dataSelector.setQuantities("");
	dataSelector.setMaxCount(1);

	this->allowedEncoding.link("type",   odim.type);
	this->allowedEncoding.link("gain",   odim.scaling.scale);
	this->allowedEncoding.link("offset", odim.scaling.offset);

	odim.completeEncoding("");

	/*
	this->allowedEncoding.link("type",   odim.type = "C");
	this->allowedEncoding.link("gain",   odim.scaling.scale = 0.1);
	this->allowedEncoding.link("offset", odim.scaling.offset = 0.0);
	*/

	// allowedEncoding.link("type", odim.type, "S");
	// allowedEncoding.link("gain", odim.scaling.scale, 0.001);

};

void BeamAltitudeOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	//mout.debug2("target nbins:" , dst.odim.geometry.width , " rscale:" , dst.odim.rscale );

	//dst.data.setGeometry(dst.odim.geometry.width, 1);
	const double max = drain::Type::call<drain::typeMax, double>(dstData.data.getType());  //dst.data.scaling.getMax<double>();

	const double scaleMetres = ODIM::versionFlagger.isSet(ODIM::KILOMETRES) ? 0.001 : 1.0;

	const double eta = srcData.odim.elangle * drain::DEG2RAD;
	double h, hEncoded;
	for (unsigned int i = 0; i < dstData.odim.area.width; ++i) {
		//std::cerr << i << '\t' << ground << " m\t h=" << h << " >" << h/odim.scale << " m\n";

		if (aboveSeaLevel){
			h = scaleMetres * (srcData.odim.height +  Geometry::heightFromEtaGround(eta, dstData.odim.getBinDistance(i)));
		}
		else {
			h = scaleMetres * Geometry::heightFromEtaGround(eta, dstData.odim.getBinDistance(i));
		}
		hEncoded = dstData.odim.scaling.inv(h);

		// h = Geometry::heightFromEtaGround(eta, i*dst.odim.rscale)/gainMetres;
		if (hEncoded < max){
			dstData.data.put(i, hEncoded);
		}
		else {
			dstData.data.put(i, dstData.odim.undetect); // quality?
		}
		//std::cerr << i << '\t' << dst.data.get<short>(i) << '\t' << dst.odim.scaleForward(dst.data.get<double>(i)) << '\n';
	}

	dstData.odim.elangle = srcData.odim.elangle;

}


}

// Rack
