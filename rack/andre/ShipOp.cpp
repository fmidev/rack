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

#include "ShipOp.h"

#include <drain/image/HighPassOp.h>
#include <drain/util/Fuzzy.h>



#include <drain/image/SlidingWindowMedianOp.h>


#include <drain/image/FastAverageOp.h>
#include <drain/image/SegmentStatisticsOp.h>
#include <drain/image/SegmentAreaOp.h>
#include <drain/image/RunLengthOp.h>


#include <drain/image/DistanceTransformOp.h>

#include <drain/image/GammaOp.h>

#include <drain/util/DataScaling.h>

// temp debugging
#include <drain/image/File.h>

#include "radar/Analysis.h"
#include "radar/Geometry.h"


using namespace drain::image;


namespace rack {

/**
 *
 *   \param srcData - the raw measurement data (DBZH only), with ODIM metadata, in polar coords
 *   \param dstData - the resulting probability field, with ODIM metadata, in polar coords
 *
 *   This function is called for each desired elevation, typically just the lowest 1-2.
 */
void ShipOp::processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const {

	/// For verbose mode (monitoring and debugging the process)
	drain::MonitorSource mout(name, __FUNCTION__);
	mout.debug() << "start" << mout.endl;

	/// Use natural (SI) units for neighbourhood window dimension, not bins/pixels.
	//  This is for portablitity between radars.
	const int width  = static_cast<int>(windowWidth / srcData.odim.rscale + 0.5);
	const int height = static_cast<int>((windowHeight/360.0) * srcData.odim.nrays + 0.5);

	mout.debug(2) << "window: " << width << ',' << height << mout.endl;

	/// Temp image for peaks
	Image tmpHighPass;

	/// Temp image (probability field, fuzzy membership of "high reflectivity")
	Image tmpFuzzyDBZ;
	tmpFuzzyDBZ.setType<unsigned char>();

	/// Step 1: compute fuzzy membership of "high reflectivity"
	RadarFunctorOp<drain::FuzzyStepsoid<double> > dbzFuzzifier; // Fuzzy step, sigmoid-like, continuous
	dbzFuzzifier.odimSrc = srcData.odim; // Radar ops use metadata for scaling the pixels and the values
	dbzFuzzifier.functor.set(reflMin, 10.0);  // minimum reflectivity (10dBZ if not set)
	dbzFuzzifier.filter(srcData.data, tmpFuzzyDBZ);  // compute "high reflectivity"
	// Debugging
	storeDebugData(2, tmpFuzzyDBZ, "SHIP_DBZ"); // for debugging only

	/// Step 2b: detect high peaks
	HighPassOp highpass;
	highpass.width  = width;
	highpass.height = height;
	highpass.offset = -0.2; // cuts off low values
	highpass.scale = (-highpass.offset + 0.5) / ((reflDev / srcData.odim.gain)/srcData.data.getMax<double>()); // => x==reflDev => prob = 0.5
	mout.debug(4) << highpass << mout.endl;
	//mout.warn() << srcData.data << mout.endl;
	highpass.filter(srcData.data, tmpHighPass);
	storeDebugData(2, tmpHighPass, "SHIP_HP"); // for debugging only

	/// Step 2b: imitate sidelobes (expontential, "IRF style"), adding them to the peaks
	DistanceTransformExponentialOp dist;
	dist.horz = 1.0;
	dist.vert = (5.5 * srcData.odim.nrays) / 360.0;
	mout.debug(4) << dist << mout.endl;
	dist.filter(tmpHighPass, tmpHighPass);
	storeDebugData(2, tmpHighPass, "SHIP_HP_DIST"); // for debugging only

	/// Step 3: combine the two evidence field by multiplying them (fuzzy-AND operation)
	BinaryFunctorOp<MultiplicationFunctor> mulOp;
	mulOp.functor.scale = 2.0;
	mulOp.filter(tmpFuzzyDBZ, tmpHighPass, dstProb.data);
	storeDebugData(3, dstProb.data, "SHIP_FINAL"); // for debugging only

}

}

// Rack
