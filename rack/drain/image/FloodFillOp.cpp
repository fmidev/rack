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

#include <sstream>
#include <ostream>
#include "ImageOp.h"
#include "Coordinates.h"
#include "FilePng.h"

#include "FloodFillOp.h"

namespace drain
{
namespace image
{


FloodFillOp::FloodFillOp(int i, int j, double min, double max, double value) : ImageOp("FloodFillOp",
		"Fills an area starting at (i,j) having intensity in [min,max], with a value.") {
	parameters.reference("i", this->i0 = i);
	parameters.reference("j", this->j0 = j);
	parameters.reference("min", this->min = min);
	parameters.reference("max", this->max = max);
	parameters.reference("value", this->value = value);
}

/*
void FloodFillOp::makeCompatible(const Image & src,Image & dst) const  {

	MonitorSource mout(iMonitor, name, __FUNCTION__);

	mout.debug(1) << dst << mout.endl;

	if (!dst.typeIsSet())
		dst.setType(src.getType());

	dst.setGeometry(src.getWidth(), src.getHeight(),
			std::max(size_t(1),dst.getImageChannelCount()), dst.getAlphaChannelCount());

	mout.debug(1) << dst << mout.endl;

}
*/

void FloodFillOp::traverse(const Image & src,Image & dst) const {

	MonitorSource mout(iMonitor, name, __FUNCTION__);

	mout.debug() << *this << mout.endl;

	//makeCompatible(src,dst);

	if (src.isFloatType() || dst.isFloatType()) {
		mout.debug(1) << "type: double" << mout.endl;
		SegmentProber<double,double> fill(src,dst);
		fill.probe(i0, j0, value, min, max);
	}
	else {
		mout.debug(1) << "type: integral" << mout.endl;
		SegmentProber<int,int> fill(src,dst);
		fill.probe(i0, j0, static_cast<int>(value), static_cast<int>(min), static_cast<int>(max));
	}

}

}
}


// Drain
