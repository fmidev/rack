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
#include "HydroClassBasedOp.h"
// temp
// #include <drain/image/File.h>
// #include "radar/Geometry.h"

using namespace drain::image;

namespace rack {

// void HydroClassBasedOp::filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const {
void HydroClassBasedOp::runDetector(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const {
//void HydroClassBasedOp::filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	//mout.debug(parameters );

	const double probScaled = nonMet * dstProb.odim.scaleInverse(1.0);

	for (unsigned int i = 0; i < srcData.data.getWidth(); ++i)
		for (unsigned int j = 0; j < srcData.data.getHeight(); ++j)
			if (srcData.data.get<unsigned int>(i,j) == NOMET)
				dstProb.data.put(i,j, probScaled);
			//else
			//dst.put(i,j, src.get<float>(i,j)*10.0);


	if (mout.isDebug(10)){
		//File::write(tmpFuzzyDBZ,"HydroClassBasedOp.png");
	}



}

}

// Rack
