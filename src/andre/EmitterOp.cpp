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

#include <algorithm>


#include <drain/util/FunctorPack.h>
#include <drain/util/Fuzzy.h>
// debugging
#include <drain/image/ImageFile.h>
#include <drain/imageops/DistanceTransformOp.h>
#include <drain/imageops/HighPassOp.h>
#include <drain/imageops/MarginalStatisticOp.h>
#include <drain/imageops/SlidingWindowMedianOp.h>

//#include "radar/ODIM.h"
#include "EmitterOp.h"

// using namespace std;

using namespace drain;
using namespace drain::image;

namespace rack {

/*  Essentially, difference of images filtered with median in two directions.
 *
 *
 */

void EmitterOp::runDetector(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.debug(*this );

	//dst.data.setPhysicalScale(0.0, 1.0);
	dst.data.setPhysicalRange(0.0, 1.0, true);
	/// Minimum segment width in pixels.
	//  const int w = src.odim.getBeamBins(lengthMin*1000.0); // (static_cast<int>(lengthMin)*1000)/src.odim.rscale;

	/// Maximum segment height in pixels.
	const int h = src.odim.getAzimuthalBins(thicknessMax); // (thicknessMax*src.odim.geometry.height)/360;

	/// Maximum segment thickness in degrees, relative to full circle (360deg)
	const double hD = static_cast<double>(thicknessMax)/360.0;

	const size_t imageWidth  = src.data.getWidth();
	const size_t imageHeight = src.data.getHeight();

	//const double reflMin = -10; // dBZ, consider as parameter?

	//mout.debug("min length and max thickness: " , w , ',' , h , " (pixels)" );

	storeDebugData(2, src.data, "SRC");

	// Image tmp;
	// Compute margin peaks
	Image marginAvg(typeid(unsigned char), 1, imageHeight);
	marginAvg.setCoordinatePolicy(PolarProductOp::polarCoordPolicy);
	//marginAvg.setPhysicalRange({0.0,1.0}, true);

	mout.special("Margin averages:" );
	double dBZ;
	std::vector<unsigned short int> count(imageHeight);
	std::vector<unsigned short int> countData(imageHeight);
	//drain::FuzzyPeak<double, unsigned char> fuzzyPeak1(10.0, 10.0, 255);
	drain::FuzzyStepsoid<double> fuzzyStepsoid(0.5, 0.5, marginAvg.getConf().getTypeMax<double>());
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
		//mout.note("d/c =  " , d , '/' , c , "\t fuzzy=" ,  fuzzyStepsoid(static_cast<double>(d)/static_cast<double>(c)) );
		if (c > 0)
			marginAvg.put(j, fuzzyStepsoid(static_cast<double>(d)/static_cast<double>(c)));
	}

	storeDebugData(2, marginAvg, "MARG");

	mout.special("HighPass op:" );

	HighPassOp hop(1, 3*h, 5.0);
	hop.process(marginAvg, marginAvg);  // 2.0 from hat  marginAvg, marginAvg
	mout.debug(hop );
	mout.debug2(marginAvg );
	storeDebugData(2, marginAvg, "MARG_HP");

	mout.special("DistanceTransformExponentialOp op:" );

	// TOPOLOGY=0
	//DistanceTransformExponentialOp(1.0, 3.0*static_cast<double>(h), 0).process(marginAvg, marginAvg);
	DistanceTransformExponentialOp(1.0, 3.0*static_cast<double>(h), DistanceModel::CONN4).process(marginAvg, marginAvg);
	storeDebugData(2, marginAvg, "MARG_HP_DIST");

	Image srcElong;
	//srcElong.setScaling(1.0);
	srcElong.setPhysicalRange(0.0, 1.0, true);
	/// Essentially, morphological closing, horizontally.
	DistanceTransformExponentialOp(3.0*1000.0/src.odim.rscale, 1).process(src.data, srcElong);  //  REQUIRE_NORMALIZED_DATA
	storeDebugData(2, srcElong, "DIST-HORZ");

	mout.special("RunLengthVertOp op:" );
	Image rleVert;
	rleVert.setPhysicalRange(0.0, 1.0, true);
	mout.special("srcElong.sc  " , srcElong.getScaling() , "  0.5->" , srcElong.getScaling().inv(0.5) );
	mout.special("srcElong.sc0 " , srcElong.getChannel(0).getScaling() , "  0.5->" , srcElong.getChannel(0).getScaling().inv(0.5) );

	//mout.warn("codeMin=" , src.odim.scaleInverse(reflMin) );
	//RunLengthVertOp(src.odim.scaleInverse(reflMin)).process(srcElong, rleVert);  //
	RunLengthVertOp(0.25).process(srcElong, rleVert);  //
	//rleVert.setPhysicalRange(0.0, 1.0, true);
	storeDebugData(2, rleVert, "RLE-VERT");

	mout.special("RemappingFunctor op:" );
	UnaryFunctorOp<RemappingFunctor> remap;
	remap.functor.fromValue = 0.0;
	remap.functor.toValue =   1.0;
	remap.process(rleVert, rleVert); // note: scale=1.0
	storeDebugData(2, rleVert, "RLE-VERT-REMAP");

	mout.special(" op:" );
	UnaryFunctorOp<FuzzyBell<double> > fuzzyBell;
	fuzzyBell.functor.set(0,hD);
	fuzzyBell.process(rleVert, rleVert);
	storeDebugData(2, rleVert, "RLE-VERT-FUZZY");

	/// Horizontal run lengths are computed on the vertical run lengths; using src would give too long lines (inside clouds)
	/*
	mout.special("RunLengthHorzOp op:" );
	Image rleHorz;
	//rleHorz.setPhysicalScale(0.0, 1.0);
	rleHorz.setPhysicalRange(0.0, 1.0, true);
	RunLengthHorzOp(0.5).process(rleVert, rleHorz);
	storeDebugData(2, rleHorz, "RLE-HORZ");

	mout.special("FuzzyStep op:" );
	UnaryFunctorOp<drain::FuzzyStep<double> > fuzzyStep;
	fuzzyStep.functor.set(0,hD);
	fuzzyStep.traverseChannel(rleHorz, rleHorz);
	storeDebugData(2, rleHorz, "RLE-HORZ-FUZZY");

	// All physical: 1.0  dst.data.setOptimalScale(0.0, 1.0);
	mout.special("MultiplicationFunctor op:" );
	BinaryFunctorOp<MultiplicationFunctor>().traverseChannel(rleVert, rleHorz, dst.data);
	storeDebugData(2, dst.data, "FINAL");
	*/

	mout.special("MultiplicationFunctor op2:" );
	mout.attention("towards END");
	BinaryFunctorOp<MultiplicationFunctor> mop;
	mop.functor.setScale(2.0 * sensitivity);
	rleVert.getChannel(0).setPhysicalRange(0,1, true);
	marginAvg.getChannel(0).setPhysicalRange(0,1, true);
	dst.data.getChannel(0).setPhysicalRange(0,1, true);
	mop.traverseChannel(rleVert, marginAvg, dst.data); // 4.0 * sensitivity*
	//mop.traverseChannel(dst.data, marginAvg, dst.data); // 4.0 * sensitivity*
	storeDebugData(2, dst.data, "FINAL-MARGIN-WEIGHTED");
	//_mout.writeImage(10, dst.data, "dst.data"); // ? final
	mout.attention("END");

}


}  // rack::

// Rack
