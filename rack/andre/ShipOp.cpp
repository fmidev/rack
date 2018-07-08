/*

    Copyright 2009-2013   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include "ShipOp.h"

#include <drain/util/FunctorPack.h>
#include <drain/util/Fuzzy.h>

#include <drain/imageops/SlidingWindowMedianOp.h>
#include <drain/imageops/FastAverageOp.h>
#include <drain/imageops/HighPassOp.h>
#include <drain/imageops/DifferentialOp.h>
#include <drain/imageops/SegmentAreaOp.h>
#include <drain/imageops/RunLengthOp.h>
#include <drain/imageops/DistanceTransformOp.h>
#include <drain/imageops/GammaOp.h>

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
	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << "odim: " << srcData.odim << mout.endl;

	/// Use natural (SI) units for neighbourhood window dimension, not bins/pixels.
	//  This is for portablitity between radars.
	const int width  = srcData.odim.getBeamBins(windowWidth);
			//static_cast<int>(windowWidth / srcData.odim.rscale + 0.5);
	const int height = srcData.odim.getAzimuthalBins(windowHeight);
			// static_cast<int>((windowHeight/360.0) * srcData.odim.nrays + 0.5);

	mout.debug(2) << "window: " << width << ',' << height << mout.endl;


	/// Temp image (probability field, fuzzy membership of "high reflectivity")
	drain::image::Image tmpFuzzyDBZ;
	tmpFuzzyDBZ.setPhysicalScale(0.0, 1.0);

	/// Step 1: compute fuzzy membership of "high reflectivity"
	RadarFunctorOp<drain::FuzzyStepsoid<double> > dbzFuzzifier; // Fuzzy step, sigmoid-like, continuous
	dbzFuzzifier.odimSrc = srcData.odim; // Radar ops use metadata for scaling the pixels and the values
	dbzFuzzifier.functor.set(reflMin, 10.0);  // minimum reflectivity (10dBZ if not set)
	mout.debug() << "fuzzy: " << dbzFuzzifier.functor << mout.endl;
	dbzFuzzifier.process(srcData.data, tmpFuzzyDBZ);  // compute "high reflectivity"
	tmpFuzzyDBZ.setPhysicalScale(0.0, 1.0);
	// Debugging
	storeDebugData(2, tmpFuzzyDBZ, "SHIP_DBZ"); // for debugging only
	mout.debug(1) << tmpFuzzyDBZ << mout.endl;

	/// Step 2b: detect high peaks
	/// Temp image for peaks
	/*
		if (! drain::Type::call<drain::typeIsSmallInt>(srcData.data.getType())){  // well, still standardized data.
			mout.warn() << "input data not 8/16 bit, probably problems ahead with undetect: " <<  srcData.data << mout.endl;
		}
	 */

	drain::image::Image tmpHighPass;
	/*
	Image srcQuality;
	srcData.createSimpleQualityData(srcQuality, 1, 0, 0);
	storeDebugData(2, srcQuality, "SHIP_Q");
	HighPassOp highpass(width, height);
	//highpass.offset = -0.2; // cuts off low values
	//highpass.scale = (-highpass.offset + 0.5) / (reflDev / srcData.odim.gain) * srcData.data.getMax<double>(); // => x==reflDev => prob = 0.5
	mout.debug(4) << highpass << mout.endl;
	highpass.process(tmpFuzzyDBZ, srcQuality, tmpHighPass, srcQuality);
	tmpHighPass.setPhysicalScale(0.0, 1.0);
	storeDebugData(2, tmpHighPass, "SHIP_HP"); // for debugging only
	*/

	drain::image::Image tmpLap(tmpFuzzyDBZ);
	//drain::image::Image tmpLap(srcData.data.getWidth(), srcData.data.getHeight(), 3);
	tmpLap.setPhysicalScale(0.0, 1.0);
	LaplaceHorzOp lapHorz(2); // not enough, detects edeges.
	mout.debug(4) << lapHorz << mout.endl;
	lapHorz.traverseChannel(tmpFuzzyDBZ, tmpLap[0]);
	storeDebugData(2, tmpLap[0], "SHIP_HPH"); // for debugging only

	/*
	LaplaceVertOp lapVert(2); // not enough, detects edeges.
	mout.debug(4) << lapVert << mout.endl;
	lapVert.traverseChannel(tmpFuzzyDBZ, tmpLap[1]);
	storeDebugData(2, tmpLap[1], "SHIP_HPV"); // for debugging only
	BinaryFunctorOp<MultiplicationFunctor>().traverseChannel(tmpLap[0], tmpLap[1], tmpLap[2]);
	storeDebugData(2, tmpLap[2], "SHIP_HPX"); // for debugging only
	//storeDebugData(2, tmpLap, "SHIP_HPX_RGB"); // for debugging only
	*/

	drain::image::Image tmpPeaks(typeid(unsigned char));

	drain::FuzzyBell<double> fuzzyBell(5.0, width*height, tmpPeaks.getMax<double>());
	SegmentAreaOp<SegmentProber<double,unsigned char> > segArea(fuzzyBell, srcData.odim.scaleInverse(reflMin));
	segArea.process(srcData.data, tmpPeaks);
	tmpPeaks.setPhysicalScale(0.0, 1.0);
	storeDebugData(2, tmpPeaks, "SHIP_SEG");

	/// Step 2b: imitate sidelobes (expontential, "IRF style"), adding them to the peaks
	DistanceTransformExponentialOp dist;
	dist.setRadius(1.0, srcData.odim.getAzimuthalBins(15.0));
	mout.debug(4) << dist << mout.endl;
	dist.process(tmpPeaks, tmpPeaks);
	storeDebugData(2, tmpPeaks, "SHIP_HP_DIST"); // for debugging only
	mout.warn() << tmpPeaks << mout.endl;


	/// Step 3: combine the two evidence field by multiplying them (fuzzy-AND operation)
	BinaryFunctorOp<MultiplicationFunctor> mulOp;
	mulOp.functor.setScale(1); // = 2.0;
	mulOp.LIMIT = true;
	mulOp.initializeParameters(tmpLap[0], tmpPeaks);
	mulOp.makeCompatible(tmpPeaks, dstProb.data);
	dstProb.setPhysicalRange(0.0, 1.0);
	mulOp.traverseChannel(tmpLap[0], tmpPeaks, dstProb.data);
	storeDebugData(3, dstProb.data, "SHIP_FINAL"); // for debugging only
	mout.debug(2) << dstProb.data << mout.endl;

}

}
