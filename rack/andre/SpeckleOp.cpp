/**

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
