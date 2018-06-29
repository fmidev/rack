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
*//**

    Copyright 2001 - 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)

    This file is part of AnoRack, radar data processing utilities for C++.

 *  Created on: Mar 7, 2011
 *      Author: mpeura
 */



#include "SpeckleOp.h"

#include <drain/imageops/SegmentAreaOp.h>


#include <drain/image/File.h>
#include <drain/util/Fuzzy.h>

using namespace drain::image;

namespace rack {


//void SpeckleOp::filterImage(const RadarODIM &odimIn, const Image &src, Image &dst) const {
void SpeckleOp::processData(const PlainData<PolarSrc> &src, PlainData<PolarDst> &dst) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << parameters << mout.endl;

	// Warn if below min dBZ?
	const double min = std::max(src.data.getMin<double>()+2.0, src.odim.scaleInverse(reflMin));
	const double max = src.data.getMax<double>()-2.0;
	drain::FuzzyBell<double> fuzzyBell;
	fuzzyBell.set(0.0, area, dst.data.getMax<double>());
	SegmentAreaOp<SegmentProber<float, unsigned short> > op(fuzzyBell, min, max); 	//"min,max,mapping,mSlope,mPos"

	mout.debug(1) << op << mout.endl;
	mout.debug(2) << src.data.getCoordinatePolicy() << mout.endl;

	//if (mout.isDebug(10)) File::write(src.data,"SegmentAreaOp_src.png");

	op.process(src.data, dst.data);

	//if (mout.isDebug(10)) File::write(dst.data,"SegmentAreaOp_dst.png");

}


}
