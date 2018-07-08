/**

    Copyright 2010-2013   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include <algorithm>

#include <drain/util/FunctorPack.h>
#include <drain/util/Fuzzy.h>

//#include <drain/imageops/SegmentAreaOp.h>
//#include <drain/imageops/RunLengthOp.h>
//#include <drain/imageops/MathOpPack.h>
//#include <drain/imageops/HighPassOp.h>
//#include <drain/imageops/SlidingWindowMedianOp.h>
#include <drain/imageops/DistanceTransformOp.h>
//#include <drain/imageops/SegmentAreaOp.h>
#include <drain/imageops/RunLengthOp.h>
//#include <drain/imageops/BasicFunctors.h>
#include <drain/imageops/DistanceTransformOp.h>
//#include <drain/imageops/FuzzyOp.h>
#include <drain/imageops/FunctorOp.h>

//#include <drain/imageops/MarginalStatisticOp.h>
//#include <drain/imageops/FuzzyPeakOp.h>
//#include <drain/imageops/FuzzyThresholdOp.h>

// debugging
#include <drain/image/File.h>

#include "LineOp.h"

// using namespace std;

using namespace drain;
using namespace drain::image;

namespace rack {

/*  Essentially, difference of images processed with median in two directions.
 *
 *
 */
//void LineOp::processImage(const PolarODIM &odimIn, const Image &src, Image &dst) const {
void LineOp::processData(const PlainData<PolarSrc> &srcData, PlainData<PolarDst> &dstData) const {


	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << "start" << mout.endl;

	//const Image & src = srcData.data;
	//Image & dst = dstData.data;

	//const double srcMin = drain::Type::getMin<double>(src.getType());

	/// Scale parameters to pixel intensities and coordinates.
	const double thresholdDistance = static_cast<double>(lengthMin)*1000.0/srcData.odim.rscale; // double, but essentially in pixel coordinates (i)
	const double widthMaxBins =  widthMax/360.0*static_cast<double>(srcData.odim.nbins);

	Image srcElong;
	DistanceTransformExponentialOp(thresholdDistance/2.5, 1).process(srcData.data, srcElong);

	//File::write(srcElong, "Line0-srcElong.png");
	//_mout.writeImage(10, srcData.data, "src-elong");

	Image rleVert(typeid(unsigned char));

	RunLengthVertOp(srcData.odim.scaleInverse(reflMin)).process(srcElong, rleVert);  //
	//_mout.writeImage(10, rleVert, "rle-vert1");
	UnaryFunctorOp<RemappingFunctor> remap;
	remap.functor.fromValue = 0;
	remap.functor.toValue = 0;
	remap.process(rleVert, rleVert);
	//RemapOp(0,255).process(rleVert, rleVert);
	//FuzzyBellOp  fuzzyHorz(0, widthMaxBins); //.process(rleVert, rleVert);
	UnaryFunctorOp<FuzzyBell<double> > fuzzyHorz;
	fuzzyHorz.functor.set(0, widthMaxBins); //.process(rleVert, rleVert);
	fuzzyHorz.process(rleVert, rleVert);
	//_mout.writeImage(10, rleVert, "rle-vert-remap");

	if (reflMin2 != std::numeric_limits<double>::min()){
		//std::cerr.precision(10);
		//std::cerr << "\n*** reflMin2" << reflMin2 << std::endl;
		//std::cerr << "\n*** reflMin3" << reflMin3 << std::endl;
		Image tmp;
		//MaximumOp maxOp;
		BinaryFunctorOp<MaximumFunctor> maxOp;
		RunLengthVertOp(srcData.odim.scaleInverse(reflMin2)).process(srcElong, tmp);
		//rle.threshold = srcData.odim.scaleInverse(reflMin2);  //
		//rle.process(srcElong, tmp);
		UnaryFunctorOp<RemappingFunctor> remap;
		remap.functor.fromValue = 0;
		remap.functor.toValue = 255;
		remap.process(tmp, tmp);
		//RemapOp(0,255).process(tmp, tmp);
		fuzzyHorz.process(tmp, tmp);
		//_mout.writeImage(10, tmp, "rle-vert2");

		maxOp.traverseChannel(tmp.getChannel(0), rleVert.getChannel(0), rleVert.getChannel(0));
		if (reflMin3 != std::numeric_limits<double>::min()){
			tmp.clear();
			RunLengthVertOp(srcData.odim.scaleInverse(reflMin3)).process(srcElong, tmp);
			//rle.threshold = srcData.odim.scaleInverse(reflMin3);  //
			//rle.process(srcElong, tmp);
			//UnaryFunctorOp<RemappingFunctor> remap;
			//RemapOp(0,255).process(tmp, tmp);
			//remap.functor.fromValue = 0;
			//remap.functor.toValue = 255;
			remap.traverseChannel(tmp.getChannel(0), tmp.getChannel(0));
			fuzzyHorz.traverseChannel(tmp.getChannel(0),tmp.getChannel(0));
			//_mout.writeImage(10, tmp, "rle-vert3");
		}
		maxOp.traverseChannel(tmp.getChannel(0), rleVert.getChannel(0), rleVert.getChannel(0));
	}

	//_mout.writeImage(10, rleVert, "rle-vertF"); // ? final


	/// Horizontal run lengths are computed "on top of" the vertical run lengths; using src would give too long lines (inside clouds)
	Image rleHorz;
	RunLengthHorzOp(128).process(rleVert, rleHorz);
	//_mout.writeImage(10, rleHorz, "rle-horz"); // ? final

	UnaryFunctorOp<FuzzyStep<double> > fuzzyStep;
	fuzzyStep.functor.set(thresholdDistance, thresholdDistance/4.0 );
	//FuzzyStepOp(thresholdDistance, thresholdDistance/4.0 ).process(rleHorz, rleHorz);
	fuzzyStep.traverseChannel(rleHorz.getChannel(0), rleHorz.getChannel(0));
	//_mout.writeImage(10, rleHorz, "rle-horz-fuzzy"); // ? final

	BinaryFunctorOp<MultiplicationFunctor>().traverseChannel(rleVert.getChannel(0), rleHorz.getChannel(0), dstData.data.getChannel(0));
	// MultiplicationOp().process(rleVert,rleHorz, dstData.data);
	//_mout.writeImage(10, dstData.data, "dst"); // ? final


	/*
	/// Using srcData.odim.gain is stable
	HighPassOp highPass(width, height, 10.0*srcData.odim.gain, -30.0);
	highPass.process(srcData.data, tmp);  // Actually does not need extra tmp.
	if (mout.isDebug(10)){
		tmp.properties["highPass"] = highPass.getParameters().getValues();
		File::write(tmp, "Emitter1-highpass.png");
	}

	/// process out small and/or weak specks
	SlidingWindowMedianOp median(5, 1, sensitivity);
	median.process(tmp, dst);
	if (mout.isDebug(10)){
		dst.properties["median"] = median.getParameters().getValues();
		File::write(dst, "Emitter3-median.png");
	}

	/// Spread   TODO scale
	DistanceTransformExponentialOp(4, 2).process(dst, dst);
	if (mout.isDebug(10)){
		File::write(dst, "Emitter5-dist.png");
	}
	*/

}


}
