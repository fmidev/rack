/*

    Copyright 2016 - 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack program for C++.

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

//#include "Coordinates.h"

#include <drain/util/Log.h>
#include <drain/util/FunctorPack.h>
//#include <drain/image/Coordinates.h>
#include "PolarSmoother.h"
#include "Analysis.h"
#include "PolarWindow.h"
#include <drain/imageops/FunctorOp.h>

using namespace drain::image;

namespace rack
{

/// PlainData<PolarDst>
//void polarGaussian(const PlainData<PolarSrc> & src, Image & dst, int radius){
void PolarSmoother::filter(const PolarODIM & odimSrc, const drain::image::Image & src, drain::image::Image & dst, double radiusMetres){

	drain::Logger mout("PolarSmoother", __FUNCTION__);
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
	weightMakerOp.process(src, srcWeighted);

	// File::write(src, "fuzzyCellArea0.png");
	// File::write(srcWeighted, "fuzzyCellArea0w.png");

	dst.setGeometry(src.getGeometry());
	dst.setCoordinatePolicy(polar);
	//mout.warn() << srcWeighted << mout.endl;

	if (radiusMetres <=  odimSrc.rscale){
		mout.warn() << "too small radiusMetres==" << radiusMetres << ", setting to 3" << mout.endl;
		radiusMetres = odimSrc.rscale * 3;
	}

	const int radiusBins =  odimSrc.getBeamBins(radiusMetres); //radiusMetres/odimSrc.rscale;


	//double radius = static_cast<double>(radiusBins);
	//GaussianStripeHorzWeighted window1(static_cast<double>(radiusBins), radiusBins*2);
	GaussianStripeWeighted<true> window1(radiusBins*2, static_cast<double>(radiusBins));
	window1.setSrcFrame(src);
	window1.setSrcFrameWeight(srcWeighted);
	window1.setDstFrame(tmp);
	window1.setDstFrameWeight(tmpWeighted);
	// mout.debug(1) << window1 << mout.endl;
	//std::cerr << __FUNCTION__ << '\n';
	window1.run();
	//std::cerr << __FUNCTION__ << "OK" << std::endl;
	//mout.warn() << tmp << '\n' << tmpWeighted << mout.endl;
	// File::write(tmp, "fuzzyCellArea1.png");
	// File::write(tmpWeighted, "fuzzyCellArea1w.png");

	GaussianStripeVertPolarWeighted window2(static_cast<double>(radiusBins), radiusBins*2);
	//GaussianStripeWeighted<false> window2(radiusBins*2, static_cast<double>(radiusBins));
	//window2.setRangeNorm(static_cast<double>(odimSrc.rscale* odimSrc.nrays) / (2.0*M_PI));
	//window2.setRangeNorm(static_cast<double>(odimSrc.rscale* odimSrc.nrays) / (2.0*M_PI));
	window2.setRangeNorm(odimSrc);
	//std::cerr << __FUNCTION__ << "OK" << std::endl;
	//static_cast<double>(odimSrc.nrays) * (M_PI/180.0) / static_cast<double>(odimSrc.rscale);
	window2.setSrcFrame(tmp);
	window2.setSrcFrameWeight(tmpWeighted);
	window2.setDstFrame(dst);
	window2.setDstFrameWeight(dstWeighted);
	mout.debug(2) << window2 << mout.endl;
	mout.debug() << ", rangeNorm=" << window2.getRangeNorm() << mout.endl;
	//mout.warn() << "startar: " << window2 << mout.endl;
	window2.run();
	// File::write(dst, "fuzzyCellArea2.png");
	mout.warn() << "success" << mout.endl;

	BinaryFunctorOp<SubtractionFunctor> sub;
	sub.functor.setScale(5.0, 0.0);
	sub.LIMIT = true;
	//sub.process(src, dst, dst);
	sub.process(src, dst);
	// File::write(dst, "fuzzyCellArea3.png");
}



}
