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

#include "FastOpticalFlowOp.h"

#include "File.h"

//#include "FastAverageOp.h"
#include "DoubleSmootherOp.h"
#include "util/Fuzzy.h"
#include "FunctorOp.h"
#include "GradientOp.h"
#include "PixelVectorOp.h"
#include "QuadraticSmootherOp.h"
#include "RecursiveRepairerOp.h"

namespace drain
{

namespace image
{




void FastOpticalFlowOp::writeDebugImage(const Image & src, const std::string & filename, double scale, double bias) const {

	MonitorSource mout(iMonitor, name, __FUNCTION__);
	mout.warn() << filename << '\t' << src  << mout.endl;

	if (src.isIntegerType() && (scale==1.0) && (bias==0.0)){
		File::write(src, filename);
	}
	else {
		Image tmp;
		tmp.setType<unsigned char>();
		//mout.warn() << " CONVERTING " << mout.endl;
		//FunctorOp<ScalingFunctor>(scale, bias).filter(src, tmp);
		UnaryFunctorOp<ScalingFunctor> op;
		op.functor.setScale(scale, bias);
		op.filter(src, tmp);
		//ScaleOp(scale, bias).filter(src, tmp);
		File::write(tmp, filename);
	}
}


/// Computes a differential image with channels dx, dy, dt and w (quality of gradients).
// User may wish to redefine this.
///
void FastOpticalFlowOp::computeDerivativeImage(const Image & src1Smooth, const Image & src2Smooth, Image & dst) const {

	//MonitorSource mout(iMonitor,"FastOpticalFlowOp::computeDerivativeImage");
	MonitorSource mout(iMonitor, name, __FUNCTION__);

	// TODO: concentrate on the "middle image". Skip grad stability, let oflow use

	const size_t imageWidth  = src1Smooth.getWidth();
	const size_t imageHeight = src1Smooth.getHeight();

	/*
	Image src1Smooth;
	Image src2Smooth;

	mout.note() << "Smoothing" << mout.endl;
	// Notice that weighted smoothing applies, if applicable
	smoothen(src1, src1Smooth);
	smoothen(src2, src2Smooth);
	*/
	if (mout.isDebug(3)){
		writeDebugImage(src1Smooth, "src1Smooth.png");
		writeDebugImage(src2Smooth, "src2Smooth.png");
	}

	/// NOTE! dst == src2 ?
	dst.setType<double>();
	dst.setGeometry(imageWidth,imageHeight,3,1);

	Image grad;
	grad.setView(dst, 0, 2);  // dx & dy
	Image & dx = dst.getChannel(0);
	Image & dy = dst.getChannel(1);
	Image & dt = dst.getChannel(2);
	Image &  w = dst.getChannel(3); // alpha


	mout.debug() << " diff: dst=" << dst << mout.endl;
	mout.debug() << " diff: &dt=" << dt  << mout.endl;

	mout.note() << "Computing time derivative" << mout.endl;
	// Time derivative, dt
	//BinaryFunctorOp<SubtractionFunctor> subOp;
	BinaryFunctorOp<SubtractionFunctor>().filter(src2Smooth.getChannel(0), src1Smooth.getChannel(0), dt);
	// SubtractionOp().filter(src2Smooth.getChannel(0), src1Smooth.getChannel(0), dt);
	if (mout.isDebug(3)){
		writeDebugImage(dt, "diff-dt.png", 0.5, 0.5);
		//writeDebugImage(dt, "diff-dt2.png", 0.15, 0.5);
		//writeDebugImage(dt, "diff-dt0.png");
	}

	mout.info() << "Computing gradients" << mout.endl;
	mout.debug(3) << " - smooth1: " << src1Smooth << mout.endl;
	mout.debug(3) << " - smooth2: " << src2Smooth << mout.endl;
	mout.debug(3) << " - dst: "     << dst << mout.endl;


	if (true){ // TODO: move to optional funct?
		/// Gradient quality = gradient stability * gradient magnitude
		// (part 1: gradient unchangedness)
		//
		//mout.debug(20);

		mout.info() << "First, determining gradient quality (stability between g2 and g1)" << mout.endl;
		mout.debug(3) << "g1 (gradients of src" << mout.endl;
		computeGradient(src1Smooth.getChannel(0), dx, dy);
		dx.setLimits(0.0, 255.0);
		dy.setLimits(0.0, 255.0);
		if (mout.isDebug(3)){
			writeDebugImage(dx, "diff-dx.png", 0.5, 0.5);
			writeDebugImage(dy, "diff-dy.png", 0.5, 0.5);
		}

		mout.debug(3) << "g2 (gradients of src" << mout.endl;
		Image grad2(grad.getType(), imageWidth, imageHeight, 2);
		//grad2.setLimits(0.0, 255.0);
		Image & dx2 = grad2.getChannel(0);
		dx2.setLimits(0.0, 255.0);
		Image & dy2 = grad2.getChannel(1);
		dy2.setLimits(0.0, 255.0);
		computeGradient(src2Smooth.getChannel(0), dx2, dy2);
		//GradientHorizontalOp().filter(src2Smooth.getChannel(0), dx2); //.getChannel(0));
		//GradientVerticalOp().filter(  src2Smooth.getChannel(0), dy2); //.getChannel(1));
		if (mout.isDebug(3)){
			writeDebugImage(dx2, "diff-dx2.png", 0.5, 0.5);
			writeDebugImage(dy2, "diff-dy2.png", 0.5, 0.5);
		}

		DistanceOp distOp;
		distOp.functorName   = "FuzzyStep";
		distOp.functorParams = "0,25";
		distOp.filter(grad, grad2, w);
		w.setLimits(0, 255.0);
		if (mout.isDebug(3)){
			writeDebugImage(w, "diff-grad-w.png");  // , 0.5, 0.5
		}
	}


	mout.note() << "Computing intermediate source image, for 'final' gradients" << mout.endl;
	// Intensity gradients should not be computed for the 1st or 2nd image, but "between" them.
	// So the images will be mixed here. //// To save memory, src2Smooth is recycled.
	Image srcMixed; // = src2Smooth;
	BinaryFunctorOp<MixerFunctor> mixOp;
	mixOp.functor.coeff = 0.5;
	mixOp.filter(src2Smooth, src1Smooth, srcMixed);
	//MixerOp(0.5).filter(src2Smooth, src1Smooth, srcMixed);
	if (mout.isDebug(3)){
		writeDebugImage(srcMixed, "src-mixed.png");
	}
	computeGradient(srcMixed.getChannel(0), dx, dy);
	if (mout.isDebug(3)){
		writeDebugImage(dx, "diff-dx-mixed.png", 0.5, 0.5);
		writeDebugImage(dy, "diff-dy-mixed.png", 0.5, 0.5);
	}

	/*
	mout.note() << "Computing gradient magnitude" << mout.endl;
	// Update Gradient stability as well (emphasize strong gradients)
	Image gradMagnitude;
	gradMagnitude.setLimits(0, 255);
	MagnitudeOp("linear", 2).filter(grad, gradMagnitude);
	if (mout.isDebug(3)){
		writeDebugImage(gradMagnitude, "diff-grad-magn.png", 0.2, 0.0);
	}
	MultiplicationOp(255.0).filter(w, gradMagnitude, w);
	if (mout.isDebug(3)){
		writeDebugImage(w, "diff-grad-magn-wd.png", 1.0, 0.0);
	}
	*/


}



}
}


// Drain
