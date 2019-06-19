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
// Algorithm is based on the study made by Brandon Hickman from The University Of Helsinki

#include <drain/util/Fuzzy.h>
#include <drain/util/FunctorPack.h>
//#include <drain/image/SlidingWindowMedianOp.h>
#include <drain/image/File.h>

//#include "hi5/Hi5Write.h"

#include "data/ODIM.h"
#include "data/Data.h"
#include "data/QuantityMap.h"

#include "radar/Geometry.h"
#include "radar/Analysis.h"
#include "main/rack.h"
#include "radar/Precipitation.h"

#include "RainRateDPOp.h"


//#include <drain/image/SegmentAreaOp.h>
//#include <drain/image/MathOpPack.h>



//using namespace drain::image;

namespace rack {



void RainRateDPOp::processDataSet(const DataSet<PolarSrc> & sweepSrc, DataSet<PolarDst> & dstProduct) const {

	/// For monitoring cmd execution as text dump. Use --debug (or \c --verbose \c \<level\> to define verbosity.
	drain::Logger mout(name, __FUNCTION__);
	mout.info() << "start" <<  mout.endl;

	// IDEA: FUZZY => PROB quality2/... so that finally class could be extracted?

	/// Retrieve dBZ data
	const PlainData<PolarSrc> & srcDBZH = sweepSrc.getData("DBZH");

	if (srcDBZH.data.isEmpty()){
		mout.warn() << "DBZH missing, giving up..." <<  mout.endl;
		return;
	}

	const drain::image::AreaGeometry & geometry = srcDBZH.data.getGeometry();
	const QuantityMap & qmap = getQuantityMap();

	RadarFunctorOp<drain::FuzzyStep<double> > step;
	RadarFunctorOp<drain::FuzzyStepsoid<double> > stepsoid;


	// Reserve main quantity (RATE) first
	PlainData<PolarDst> & rateDst = dstProduct.get(odim.quantity);
	rateDst.setGeometry(geometry);

	// RHOHV
	const PlainData<PolarSrc> & srcRHOHV = sweepSrc.getData("RHOHV");
	PlainData<PolarDst>  & metRHOHV = dstProduct.get("RHOHV_FUZZY");
	qmap.setQuantityDefaults(metRHOHV, "PROB");

	if (srcRHOHV.data.isEmpty()){
		mout.warn() << "RHOHV missing, fuzzifying to: " << 0.5 <<  mout.endl;
		metRHOHV.setGeometry(geometry);
		metRHOHV.data.fill(metRHOHV.odim.scaleInverse(0.5));
	}
	else {
		stepsoid.odimSrc = srcRHOHV.odim;
		stepsoid.functor.set(rhohv, 0.15);
		stepsoid.process(srcRHOHV.data, metRHOHV.data);
	}

	// DBZH
	stepsoid.odimSrc = srcDBZH.odim;
	// DBZH / heavy rain
	PlainData<PolarDst> & heavyDBZH = dstProduct.get("DBZH_HEAVY");
	qmap.setQuantityDefaults(heavyDBZH, "PROB");
	stepsoid.functor.set(dbzRange.min, +5.0);
	stepsoid.process(srcDBZH.data, heavyDBZH.data);
	heavyDBZH.data.setPhysicalScale(0.0, 1.0);
// DBZH / hail
	PlainData<PolarDst> & hailDBZH = dstProduct.get("DBZH_HAIL");
	qmap.setQuantityDefaults(hailDBZH, "PROB");
	stepsoid.functor.set(dbzRange.max, +5.0);
	stepsoid.process(srcDBZH.data, hailDBZH.data);

	// ZDR
	const PlainData<PolarSrc> & srcZDR = sweepSrc.getData("ZDR");
	PlainData<PolarDst>  & heavyZDR = dstProduct.get("ZDR_FUZZY");
	qmap.setQuantityDefaults(heavyZDR, "PROB");

	if (srcZDR.data.isEmpty()){
		mout.warn() << "ZDR missing, fuzzifying to:" << 0.5 <<  mout.endl;
		heavyZDR.setGeometry(geometry);
		heavyZDR.data.fill(heavyZDR.odim.scaleInverse(0.5));
	}
	else {
		stepsoid.odimSrc = srcZDR.odim;
		stepsoid.functor.set(zdr, 0.1);
		stepsoid.process(srcZDR.data, heavyZDR.data);
	}

	// KDP
	const PlainData<PolarSrc> & srcKDP = sweepSrc.getData("KDP");
	PlainData<PolarDst>  & heavyKDP = dstProduct.get("KDP_FUZZY");
	qmap.setQuantityDefaults(heavyKDP, "PROB");

	if (srcKDP.data.isEmpty()){
		mout.warn() << "KDP missing, fuzzifying to:" << 0.5 <<  mout.endl;
		heavyKDP.setGeometry(geometry);
		heavyKDP.data.fill(heavyKDP.odim.scaleInverse(0.5));
	}
	else {
		stepsoid.odimSrc = srcKDP.odim;
		stepsoid.functor.set(kdp, 0.25);
		stepsoid.process(srcKDP.data, heavyKDP.data);
		heavyKDP.data.setPhysicalScale(0.0, 1.0);
	}

	// KDP x DBZH
	PlainData<PolarDst> & heavyKDPxDBZH = dstProduct.get("KDPxDBZH_FUZZY");
	qmap.setQuantityDefaults(heavyKDPxDBZH, "PROB");
	heavyKDPxDBZH.setGeometry(geometry);
	drain::image::BinaryFunctorOp<MultiplicationFunctor> mul;
	mout.warn() << heavyDBZH.data     <<  mout.endl;
	mout.warn() << heavyKDP.data.getChannel(0)      <<  mout.endl;
	mout.warn() << heavyKDPxDBZH.data <<  mout.endl;
	mul.traverseChannel(heavyDBZH.data, heavyKDP.data.getChannel(0), heavyKDPxDBZH.data);

}



}

// Rack
