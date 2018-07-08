/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <sstream>
#include <ostream>
#include "image/Coordinates.h"
#include "image/FilePng.h"

#include "ImageOp.h"
#include "FloodFillOp.h"

namespace drain
{
namespace image
{


FloodFillOp::FloodFillOp(int i, int j, double min, double max, double value) : ImageOp(__FUNCTION__,
		"Fills an area starting at (i,j) having intensity in [min,max], with a value.") {
	parameters.reference("i", this->i0 = i);
	parameters.reference("j", this->j0 = j);
	parameters.reference("min", this->min = min);
	parameters.reference("max", this->max = max);
	parameters.reference("value", this->value = value);
}


void FloodFillOp::traverseChannel(const Channel & src, Channel & dst) const {

	Logger mout(getImgLog(), name, __FUNCTION__);

	mout.debug() << *this << mout.endl;

	if (src.isFloatType() || dst.isFloatType()) {
		mout.debug(1) << "type: double" << mout.endl;
		SegmentProber<double,double> fill(src, dst);
		src.adjustCoordinateHandler(fill.handler);
		fill.probe(i0, j0, value, min, max);
	}
	else {
		mout.debug(1) << "type: integral" << mout.endl;
		SegmentProber<int,int> fill(src, dst);
		src.adjustCoordinateHandler(fill.handler);
		fill.probe(i0, j0, static_cast<int>(value), static_cast<int>(min), static_cast<int>(max));
	}

}

}
}

