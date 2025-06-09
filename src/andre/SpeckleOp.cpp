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



#include <drain/image/ImageFile.h>
#include <drain/util/Fuzzy.h>

#include <drain/imageops/SegmentAreaOp.h>
#include "radar/Analysis.h"
#include "SpeckleOp.h"

//using namespace drain::image;

namespace rack {


//void SpeckleOp::filterImage(const RadarODIM &odimIn, const Image &src, Image &dst) const {
void SpeckleOp::runDetector(const PlainData<PolarSrc> &src, PlainData<PolarDst> &dst) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.debug(parameters );

	mout.debug2(src.data.getCoordinatePolicy() );


	//const double codeValueMin = src.odim.getMin<double>();
	const double thresholdMin = src.odim.getMin(); //src.odim.scaleForward(src.odim.getMin()); // skips 0, 1 if undetect/nodata

	double thresholdFinal = this->threshold;
	if (thresholdFinal < thresholdMin){
		mout.warn("src    " , src.data.getConf() );
		mout.warn("src[0] " , src.data.getChannel(0).getConf() );
		mout.warn("src.odim " , src.odim.scaling );
		mout.warn("threshold (", thresholdFinal , "dBZ) lower than supported min (" , thresholdMin , "), adjusting it." );
		thresholdFinal = thresholdMin;
	}

	// Warn if below min dBZ?
	//const double min = std::max(src.data.getMin<double>()+2.0, src.odim.scaleInverse(reflMin));
	//const double max = src.data.getMax<double>()-2.0;
	drain::FuzzyBell2<double> fuzzyBell;
	// Peak at 1.0 (one pixel, minimum speck size)
	// Area: half-width at 25...50 pixels
	// Gain: around 250
	// Offset: often 1.0, because 0.0 reserved for no-data?
	double offset = dst.odim.scaleInverse(0.0);

	if (invertPolar){
		/// Distance [bins] at which a bin is (nearly) square, ie. when beam-perpendicular and beam-directional steps are equal.
		const double r = static_cast<double>(src.odim.area.height) / (2.0*M_PI);
		fuzzyBell.set(1.0, static_cast<double>(area)*r, dst.odim.scaleInverse(1.0)-offset, offset);
		SegmentAreaOp<float,unsigned short,PolarSegmentProber> op2(fuzzyBell, thresholdFinal);
		mout.debug("internal operator: " , op2 );
		op2.process(src.data, dst.data);
	}
	else {
		fuzzyBell.set(1.0, area, dst.odim.scaleInverse(1.0)-offset, offset);
		SegmentAreaOp<float,unsigned short> op(fuzzyBell, thresholdFinal); // dBZ!	//"min,max,mapping,mSlope,mPos"
		mout.debug("internal operator: " , op );
		op.process(src.data, dst.data);
	}




	//if (mout.isDebug(10)) File::write(src.data,"SegmentAreaOp_src.png");


	//if (mout.isDebug(10)) File::write(dst.data,"SegmentAreaOp_dst.png");

}


}
