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


#include "ThresholdOp.h"


using namespace drain::image;


namespace rack {


//void FThresholdOp::filterImage(const PolarODIM &srcData.odim, const Image &src, Image &dst) const {
void FThresholdOp::processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const {

	const int width  = srcData.data.getWidth();
	const int height = srcData.data.getHeight();

	//const drain::VariableMap &attributes = srcData.data.properties;

	const drain::DataScaling scaleDBZ(srcData.odim.gain, srcData.odim.offset);

	//const float rscale = attributes.get("where:rscale",500.0f);
	//const float eta    = attributes.get("where:elangle",0.5) * M_PI/180.0;
	//const float nodata     = attributes.get("what:nodata", srcData.data.getMax<float>());
	//const float undetect   = attributes.get("what:undetect", 0.0f);

	/// Detector parameters
	//const float maxDBZ = getParameter("maxDBZ",10.0);   //image::Intensity::limit<T>((getParameter("max",10.0)-offset)/gain);
	//const float dBZWidth = getParameter("dBZWidth",10.0f);


	/// Descending fuzzy step, located at (max) altitude.
	//const drain::FuzzyStepsoid<double,float> tAltitude(maxAltitude, -altitudeWidth);

	/// Descending fuzzy step, located at max intensity
	const drain::FuzzyStepsoid<float> tReflectivity(maxDBZ, -dBZWidth);

	float s;
	//float c1; // fuzzy altitude
	float c2; // fuzzy reflectivity
	const float maxD = 250.0; //dstData.getMax<float>()-1.0f;
	for (int i = 0; i < width; ++i) {
		//tAltitude.filter(Geometry::heightFromEtaBeam(eta,i*rscale),c1);
		for (int j = 0; j < height; ++j) {
			s = srcData.data.get<float>(i,j);
			if ((s == srcData.odim.undetect) || (s == srcData.odim.nodata))
				continue;
			c2 = tReflectivity(scaleDBZ.forward(s));
			dstProb.data.put(i, j, maxD*c2);
		}
	}

	//if (modrain::Debug > 4)
	//	File::write(dstProb.data, "ThresholdOp.png");

}

}  // rack::

// Rack
