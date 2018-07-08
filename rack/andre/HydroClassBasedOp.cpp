/**

    Copyright 2010-2012   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack library for C++.

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
#include "HydroClassBasedOp.h"
// temp
// #include <drain/image/File.h>
// #include "radar/Geometry.h"

using namespace drain::image;

namespace rack {

// void HydroClassBasedOp::filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const {
void HydroClassBasedOp::processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const {
//void HydroClassBasedOp::filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const {

	drain::Logger mout(name, __FUNCTION__);
	//mout.debug() << parameters << mout.endl;

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
