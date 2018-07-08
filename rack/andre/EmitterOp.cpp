/**

    Copyright 2010-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

//#include "radar/ODIM.h"
#include "EmitterOp.h"

#include <drain/util/FunctorPack.h>
#include <drain/util/Fuzzy.h>
// debugging
#include <drain/image/File.h>
#include <drain/imageops/DistanceTransformOp.h>
#include <drain/imageops/HighPassOp.h>
#include <drain/imageops/MarginalStatisticOp.h>
#include <drain/imageops/SlidingWindowMedianOp.h>



// using namespace std;

using namespace drain;
using namespace drain::image;

namespace rack {

/*  Essentially, difference of images filtered with median in two directions.
 *
 *
 */

void EmitterOp::processData(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << *this << mout.endl;

	dst.data.setPhysicalScale(0.0, 1.0);

	/// Minimum segment width in pixels.
	const int w = src.odim.getBeamBins(lengthMin*1000.0); // (static_cast<int>(lengthMin)*1000)/src.odim.rscale;


	/// Maximum segment height in pixels.
	const int h = src.odim.getAzimuthalBins(thicknessMax); // (thicknessMax*src.odim.nrays)/360;

	/// Maximum segment thickness in degrees, relative to full circle (360deg)
	const double hD = static_cast<double>(thicknessMax)/360.0;

	const size_t imageWidth  = src.data.getWidth();
	const size_t imageHeight = src.data.getHeight();

	const double reflMin = -10; // dBZ, consider as parameter?

	mout.debug(3) << "min length and max thickness," << w << ',' << h << " (pixels)" << mout.endl;

	storeDebugData(2, src.data, "SRC");

	Image tmp;
	// Compute margin peaks
	Image marginAvg(typeid(unsigned char), 1, imageHeight);
	marginAvg.setCoordinatePolicy(PolarProductOp::polarCoordPolicy);

	double dBZ;
	std::vector<unsigned short int> count(imageHeight);
	std::vector<unsigned short int> countData(imageHeight);
	//drain::FuzzyPeak<double, unsigned char> fuzzyPeak1(10.0, 10.0, 255);
	drain::FuzzyStepsoid<double> fuzzyStepsoid(0.5, 0.5, marginAvg.getMax<double>());
	//drain::FuzzyStep<double, double> fuzzyStep(0.25, 0.50, 255.0);
	for (size_t j=0; j<imageHeight; ++j){
		unsigned short int & c = count[j];
		unsigned short int & d = countData[j];
		c = 0;
		d = 0;
		for (size_t i=0; i<imageWidth; ++i){
			dBZ = src.data.get<double>(i,j);
			if (dBZ != src.odim.nodata){
				++c;
				if (dBZ != src.odim.undetect)
					++d;
					//s += src.odim.scaleForward(d);
			}
		}
		// d = fuzzyPeak1(s/static_cast<double>(c));
		//mout.note() << "d/c =  " << d << '/' << c << "\t fuzzy=" <<  fuzzyStepsoid(static_cast<double>(d)/static_cast<double>(c)) << mout.endl;
		marginAvg.put(j, fuzzyStepsoid(static_cast<double>(d)/static_cast<double>(c)));
	}

	storeDebugData(2, marginAvg, "MARG");

	HighPassOp hop(1, 3*h, 5.0);
	hop.process(marginAvg, marginAvg);  // 2.0 from hat  marginAvg, marginAvg
	mout.debug() << hop << mout.endl;
	mout.debug(1) << marginAvg << mout.endl;
	storeDebugData(2, marginAvg, "MARG_HP");

	DistanceTransformExponentialOp(1.0, 3.0*static_cast<double>(h)).process(marginAvg, marginAvg);
	storeDebugData(2, marginAvg, "MARG_HP_DIST");

	Image srcElong;
	srcElong.setScaling(1.0);
	/// Essentially, morphological closing, horizontally.
	DistanceTransformExponentialOp(3.0*1000.0/src.odim.rscale, 1).process(src.data, srcElong);  //  REQUIRE_NORMALIZED_DATA
	storeDebugData(2, srcElong, "DIST-HORZ");


	Image rleVert;
	rleVert.setPhysicalScale(0.0, 1.0);
	RunLengthVertOp(src.odim.scaleInverse(reflMin)).process(srcElong, rleVert);  //
	storeDebugData(2, rleVert, "RLE-VERT");

	UnaryFunctorOp<RemappingFunctor> remap;
	remap.functor.fromValue = 0.0;
	remap.functor.toValue =   1.0;
	remap.process(rleVert, rleVert); // note: scale=1.0
	storeDebugData(2, rleVert, "RLE-VERT-REMAP");

	UnaryFunctorOp<FuzzyBell<double> > fuzzyBell;
	fuzzyBell.functor.set(0,hD);
	fuzzyBell.process(rleVert, rleVert);
	storeDebugData(2, rleVert, "RLE-VERT-FUZZY");

	/// Horizontal run lengths are computed on the vertical run lengths; using src would give too long lines (inside clouds)
	Image rleHorz;
	rleHorz.setPhysicalScale(0.0, 1.0);
	RunLengthHorzOp(0.5).process(rleVert, rleHorz);
	storeDebugData(2, rleHorz, "RLE-HORZ");

	UnaryFunctorOp<drain::FuzzyStep<double> > fuzzyStep;
	fuzzyStep.functor.set(0,hD);
	fuzzyStep.traverseChannel(rleHorz, rleHorz);
	storeDebugData(2, rleHorz, "RLE-HORZ-FUZZY");

	// All physical: 1.0  dst.data.setOptimalScale(0.0, 1.0);
	BinaryFunctorOp<MultiplicationFunctor>().traverseChannel(rleVert, rleHorz, dst.data);
	storeDebugData(2, dst.data, "FINAL");

	BinaryFunctorOp<MultiplicationFunctor>(2.0 * sensitivity).traverseChannel(dst.data, marginAvg, dst.data); // 4.0 * sensitivity*
	storeDebugData(2, dst.data, "FINAL-MARGIN-WEIGHTED");
	//_mout.writeImage(10, dst.data, "dst.data"); // ? final


}


}  // rack::
