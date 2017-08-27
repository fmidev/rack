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

//#include "Coordinates.h"

#include <drain/util/Debug.h>
#include <drain/image/BasicOps.h>
//#include <drain/image/Coordinates.h>
#include "PolarSmoother.h"
#include "Analysis.h"
#include "PolarWindow.h"


using namespace drain::image;

namespace rack
{

/// PlainData<PolarDst>
//void polarGaussian(const PlainData<PolarSrc> & src, Image & dst, int radius){
void PolarSmoother::filter(const PolarODIM & odimSrc, const drain::image::Image & src, drain::image::Image & dst, double radiusMetres){

	drain::MonitorSource mout("name", __FUNCTION__);
	CoordinatePolicy polar(CoordinatePolicy::POLAR, CoordinatePolicy::WRAP, CoordinatePolicy::LIMIT,CoordinatePolicy::WRAP);

	Image srcWeighted;
	Image tmp;
	Image tmpWeighted;
	Image dstWeighted;
	srcWeighted.setGeometry(src.getGeometry());
	tmp.setCoordinatePolicy(polar);
	tmp.setGeometry(src.getGeometry());
	tmpWeighted.setGeometry(src.getGeometry());
	dstWeighted.setGeometry(src.getGeometry());

	RadarFunctorOp<BinaryThresholdFunctor> weightMakerOp(true);
	//drain::image::UnaryFunctorOp<RadarDataFunctor<BinaryThresholdFunctor> > weightMakerOp(true);
	weightMakerOp.odimSrc = odimSrc;
	weightMakerOp.nodataValue   = 0.0;
	weightMakerOp.undetectValue = 1.0;
	weightMakerOp.functor.threshold  = -10.0;
	weightMakerOp.functor.replace = 0;
	weightMakerOp.functor.replaceHigh = 255;
	weightMakerOp.filter(src, srcWeighted);

	// File::write(src, "fuzzyCellArea0.png");
	// File::write(srcWeighted, "fuzzyCellArea0w.png");

	dst.setGeometry(src.getGeometry());
	dst.setCoordinatePolicy(polar);
	mout.warn() << srcWeighted << mout.endl;

	if (radiusMetres <=  odimSrc.rscale){
		mout.warn() << "too small radiusMetres==" << radiusMetres << ", setting to 3" << mout.endl;
		radiusMetres = odimSrc.rscale * 3;
	}

	const int radiusBins =  odimSrc.getBeamBins(radiusMetres); //radiusMetres/odimSrc.rscale;



	GaussianStripeHorzWeighted window1(static_cast<double>(radiusBins), radiusBins*2);
	window1.setSrc(src);
	window1.setSrcWeight(srcWeighted);
	window1.setDst(tmp);
	window1.setDstWeight(tmpWeighted);
	// mout.debug(1) << window1 << mout.endl;
	//std::cerr << __FUNCTION__ << '\n';
	window1.traverse();
	//std::cerr << __FUNCTION__ << "OK" << std::endl;
	//mout.warn() << tmp << '\n' << tmpWeighted << mout.endl;
	// File::write(tmp, "fuzzyCellArea1.png");
	// File::write(tmpWeighted, "fuzzyCellArea1w.png");

	GaussianStripeVertPolarWeighted window2(radiusBins, radiusBins*2);
	//window2.setRangeNorm(static_cast<double>(odimSrc.rscale* odimSrc.nrays) / (2.0*M_PI));
	//window2.setRangeNorm(static_cast<double>(odimSrc.rscale* odimSrc.nrays) / (2.0*M_PI));
	window2.setRangeNorm(odimSrc);
	//std::cerr << __FUNCTION__ << "OK" << std::endl;
	//static_cast<double>(odimSrc.nrays) * (M_PI/180.0) / static_cast<double>(odimSrc.rscale);
	window2.setSrc(tmp);
	window2.setSrcWeight(tmpWeighted);
	window2.setDst(dst);
	window2.setDstWeight(dstWeighted);
	mout.debug(2) << window2 << mout.endl;
	mout.debug() << ", rangeNorm=" << window2.getRangeNorm() << mout.endl;
	//mout.warn() << "startar: " << window2 << mout.endl;
	window2.traverse();
	// File::write(dst, "fuzzyCellArea2.png");
	mout.warn() << "success" << mout.endl;

	BinaryFunctorOp<SubtractionFunctor> sub;
	sub.functor.setScale(5.0, 0.0);
	sub.LIMIT = true;
	sub.filter(src, dst, dst);
	// File::write(dst, "fuzzyCellArea3.png");
}



}

// Rack
