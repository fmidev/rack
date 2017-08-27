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

//#include "radar/ODIM.h"
#include "EmitterOp.h"

//#include <drain/image/SegmentAreaOp.h>
//#include <drain/image/RunLengthOp.h>
#include <drain/image/BasicOps.h>


#include <drain/image/DistanceTransformOp.h>
//#include <drain/image/FuzzyPeakOp.h>
//#include <drain/image/FuzzyThresholdOp.h>
#include <drain/image/SlidingWindowMedianOp.h>
//#include "image/GammaOp.h"
#include <drain/image/HighPassOp.h>
#include <drain/image/MarginalStatisticOp.h>

// debugging
#include <drain/image/File.h>

#include <drain/util/Fuzzy.h>

// using namespace std;

using namespace drain;
using namespace drain::image;

namespace rack {

/*  Essentially, difference of images filtered with median in two directions.
 *
 *
 */

void EmitterOp::processData(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const {

	drain::MonitorSource mout(name, __FUNCTION__);
	mout.debug() << *this << mout.endl;

	const int h     = (widthMax*src.odim.nrays)/360;
	/// Maximum segment thickness, relative to full circle
	const double hD = static_cast<double>(widthMax)/360.0;
	const int w = (static_cast<int>(lengthMin)*1000)/src.odim.rscale;
	/// Minimum segment length, relative to image width
	//const double wD = static_cast<double>(lengthMin)*1000.0/static_cast<double>(src.odim.rscale*src.odim.nbins);

	const size_t imageWidth  = src.data.getWidth();
	const size_t imageHeight = src.data.getHeight();

	const double reflMin = -10; // dBZ

	mout.debug(3) << "physical widthMax,lengthMin = " << h << ',' << w << mout.endl;

	storeDebugData(2, src.data, "SRC");


	Image tmp;
	// Compute margin peaks
	Image marginAvg;
	marginAvg.setType<unsigned char>();
	//marginAvg.useAbsoluteScale();
	marginAvg.setGeometry(1, imageHeight);

	double dBZ;
	std::vector<unsigned short int> count(imageHeight);
	std::vector<unsigned short int> countData(imageHeight);
	//drain::FuzzyPeak<double, unsigned char> fuzzyPeak1(10.0, 10.0, 255);
	drain::FuzzyStepsoid<double> fuzzyStepsoid(0.25, 0.125, marginAvg.getMax<double>());
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
		marginAvg.put(j, fuzzyStepsoid(static_cast<double>(d)/static_cast<double>(c)));
	}


	storeDebugData(2, marginAvg, "MARG");



	HighPassOp(1, 3*h, 5.0).filter(marginAvg, marginAvg);  // 2.0 from hat  marginAvg, marginAvg
	storeDebugData(2, marginAvg, "MARG_HP");


	DistanceTransformExponentialOp(1, 3*h).filter(marginAvg, marginAvg);
	storeDebugData(2, marginAvg, "MARG_HP_DIST");


	Image srcElong; //(typeid(unsigned char));
	//srcElong.setType<unsigned char>();
	srcElong.useAbsoluteScale();
	/// Essentially, morphological closing, horizontally.
	DistanceTransformExponentialOp(3.0*1000.0/src.odim.rscale, 1).filter(src.data, srcElong);  //  REQUIRE_NORMALIZED_DATA
	storeDebugData(2, srcElong, "DIST-HORZ");


	Image rleVert(typeid(unsigned char));
	//rleVert.setType<unsigned char>();
	//rleVert.useAbsoluteScale(); // true lengths

	RunLengthVertOp(src.odim.scaleInverse(reflMin)).filter(srcElong, rleVert);  //
	storeDebugData(2, rleVert, "RLE-VERT");

	UnaryFunctorOp<RemappingFunctor> remap;
	remap.functor.fromValue = 0.0;
	remap.functor.toValue =   1.0;
	remap.filter(rleVert, rleVert); // note: scale=1.0
	storeDebugData(2, rleVert, "RLE-VERT-REMAP");
	//rleVert.useAbsoluteScale(); // true lengths
	//mout.warn() << "rle-vert-remap: " << rleVert << mout.endl;
	//FuzzyBellOp(0, hD).filter(rleVert, rleVert);  // note: scale=1.0EmitterOp
	UnaryFunctorOp<FuzzyBell<double> > fuzzyBell;
	fuzzyBell.functor.set(0,hD);
	fuzzyBell.filter(rleVert, rleVert);
	storeDebugData(2, rleVert, "RLE-VERT-FUZZY");

	/// Horizontal run lengths are computed on the vertical run lengths; using src would give too long lines (inside clouds)
	Image rleHorz(typeid(unsigned char));
	//rleHorz.setType<unsigned char>();
	//rleHorz.useAbsoluteScale(); // true lengths
	RunLengthHorzOp(128).filter(rleVert, rleHorz);
	storeDebugData(2, rleHorz, "RLE-HORZ");
	//_mout.writeImage(10, rleHorz, "rle-horz"); // ? final

	//FuzzyStepOp(lengthMin/255.0, 1.0+w).filter(rleHorz, rleHorz);
	//FuzzyStepOp(0.75*wD, wD).filter(rleHorz, rleHorz);
	UnaryFunctorOp<drain::FuzzyStep<double> > fuzzyStep;
	fuzzyStep.functor.set(0,hD);
	fuzzyStep.filter(rleVert, rleVert);
	storeDebugData(2, rleHorz, "RLE-HORZ-FUZZY");


	BinaryFunctorOp<MultiplicationFunctor>().filter(rleVert,rleHorz, dst.data);
	storeDebugData(2, dst.data, "FINAL");

	BinaryFunctorOp<MultiplicationFunctor>(2.0 * sensitivity).filter(dst.data, marginAvg, dst.data); // 4.0 * sensitivity*
	storeDebugData(2, dst.data, "FINAL-MARGIN-WEIGHTED");
	//_mout.writeImage(10, dst.data, "dst.data"); // ? final


}


}  // rack::

// Rack
