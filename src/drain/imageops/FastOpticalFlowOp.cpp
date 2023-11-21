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

#include <drain/image/ImageFile.h>
#include "drain/util/Fuzzy.h"
#include "FastOpticalFlowOp.h"
#include "FunctorOp.h"
#include "DifferentialOp.h"
#include "PixelVectorOp.h"
#include "BlenderOp.h"


#include "ImageModifierPack.h"

namespace drain
{

namespace image
{


void OpticalFlowCore::setDstFrames(ImageTray<Channel> & dstTray){

	Logger mout(getImgLog(), "SlidingOpticalFlow", __FUNCTION__);

	if (dstTray.size() < 2)
		mout.error("dst has less than 2 channels" );

	// Set main dst channels: horizontal (uField) and vertical (vField) motion.
	//Window<OpticalFlowConfig>::setDstFrames(dstTray.get()); // MUST BE HERE, also for dst
	this->uField.setView(dstTray.get(0));
	this->vField.setView(dstTray.get(1));

	/// Set quality (weight) channel
	if (dstTray.hasAlpha()){
		mout.info("using dst.alpha() as quality" );
		dstWeight.setView(dstTray.getAlpha());
		//this->setDstFrameWeight(dstTray.getAlpha());
	}
	else if (dstTray.size() >= 2){
		mout.note("using dst[2] as quality" );
		//this->setDstFrameWeight(dstTray.get(2));
		dstWeight.setView(dstTray.get(2));
	}
	else {
		mout.warn("no alpha to set" );
	}

	if (dstTray.size() > 3)
		mout.note("dst has more than 3 channels?" );

}

void OpticalFlowCore1::setSrcFrames(const ImageTray<const Channel> & srcTray){

	Logger mout(getImgLog(), "SlidingOpticalFlow", __FUNCTION__);

	if (srcTray.size() < getDiffChannelCount())
		mout.error("src has less than ", getDiffChannelCount() , " channels (Dx,Dy,Dt needed)" );

	Dx.setView(srcTray.get(0));
	Dy.setView(srcTray.get(1));
	Dt.setView(srcTray.get(2));

	/// Set quality (weight) channel
	if (srcTray.hasAlpha()){
		mout.info("set alpha from src.alpha() " );
		//srcWeight.setView(srcTray.getAlpha());
		// setSrcFrameWeight(srcTray.getAlpha());
		srcWeight.setView(srcTray.getAlpha());
	}
	else if (srcTray.size() >= 4) {
		mout.note("set alpha from src.get(3) " );
		//srcWeight.setView(srcTray.get(3));
		srcWeight.setView(srcTray.getAlpha(3));
		// setSrcFrameWeight(srcTray.getAlpha(3));
	}
	else {
		mout.warn("no alpha to set?" );
	}

	if ((srcTray.size()+srcTray.alpha.size()) > 4)
		mout.note("src has more than 4 channels?" );
}


/// Computes a differential image with channels dx, dy, dt and w (quality of gradients).
void FastOpticalFlowOp::computeDifferentials(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);
	// TODO: concentrate on the "middle image". Skip grad stability, let oflow use

	// Source images (intensity)
	if (src.size() < 2) {
		mout.error("src image tray must have at least 2 channels, now (", src.size() , ")" );
	}
	else if (src.size() > 2) {
		mout.warn("src image set has more than 2 channels (", src.size() , "), using [0] and [1]" );
	}

	const Channel & src1 = src.get(0);
	const Channel & src2 = src.get(1);

	// Dst images (differentials)
	if (dst.size() == 3) {
	}
	else {
		mout.error("dst image tray does not have enough channels (3)" );
	}

	const double scale = src1.getConf().requestPhysicalMax(10.0); // easily returns 255.0 for unsigned char images
	mout.debug2() << " src: " << src1 << mout.endl;

	for (size_t i = 0; i < dst.size(); ++i) {
		Channel & d = dst.get(i);
		if (!d.getScaling().isPhysical()){
			mout.warn("would like to use physical scaling +/-" , scale , " for diff channel #" , i );
			//  => =>  d.setOptimalScale(-scale, scale); // pix?
		}
		mout.debug("diff channel #" , i , "scaling: " , d.getScaling() );
	}

	if (mout.isDebug(20)){
		ImageFile::write(src1, "src1.png");
		ImageFile::write(src2, "src2.png");
	}

	Channel & dt = dst.get(2);
	Channel &  w = dst.getAlpha(); // alpha

	ImageTray<const Channel> srcTray1;
	srcTray1.setChannels(src1);
	ImageTray<const Channel> srcTray2;
	srcTray2.setChannels(src2);

	mout.debug2() << " diff: dst=" << dst << mout.endl;
	mout.debug2() << " diff: &dt=" << dt  << mout.endl;

	mout.debug("Computing time derivative" );
	BinaryFunctorOp<SubtractionFunctor>().traverseChannel(src2, src1, dt);
	if (mout.isDebug(10)){
		ImageFile::write(dt, "diff-dt.png");
		//writeDebugImage(dt, "diff-dt.png", 0.5, 0.5);
	}

	mout.debug("Computing gradients" );
	mout .debug3() << " - smooth1: " << src1 << mout.endl;
	mout .debug3() << " - smooth2: " << src2 << mout.endl;
	mout .debug3() << " - dstTray: "     << dst << mout.endl;

	GradientOp gradientOp;
	gradientOp.LIMIT = false; // todo: decide on dst storage type?

	mout .debug3() << "g1 (gradients of src" << mout.endl;

	ImageTray<Channel> gradTray1(dst);
	gradientOp.traverseChannels(srcTray1, gradTray1);
	if (mout.isDebug(10)){
		ImageFile::write(gradTray1.get(0), "diff-dx.png");
		ImageFile::write(gradTray1.get(1), "diff-dy.png");
	}

	mout .debug3() << "g2 (gradients of src)" << mout.endl;
	Image grad2(typeid(OpticalFlowCore1::data_t)); // , dst.getGeometry()  //
	grad2.setName("Grad2");
	grad2.setPhysicalRange(-scale, scale, true);
	// grad2.setOptimalScale();

	grad2.setGeometry(dst.getGeometry());
	ImageTray<Channel> gradTray2;
	gradTray2.setChannels(grad2);
	//gradientOp.traverseFrame(src2.getChannel(0), grad2);
	gradientOp.traverseChannels(srcTray2, gradTray2);
	if (mout.isDebug(18)){
		ImageFile::write(gradTray2.get(0), "diff-dx2.png");
		ImageFile::write(gradTray2.get(1), "diff-dy2.png");
	}

	mout.debug2() << "Determining gradient quality (stability between g2 and g1)" << mout.endl;
	/// Gradient quality = gradient stability * gradient magnitude
	// (part 1: gradient unchangedness)

	DistanceOp distOp;  // Consider new BinaryOp that would compute q AND means of dx and dy ?
	distOp.functorDef   = "FuzzyStep/0.0:100.0";
	//distOp.functorParams = "0,0.25";
	//distOp.functorParams = "0,100.0"; // monotone is enough, not very parameter-sensitive

	ImageTray<Channel> weightTray;
	weightTray.setChannels(w);

	distOp.traverseChannels(gradTray1, gradTray2, weightTray);

	if (mout.isDebug(3)){
		mout.warn(w );
		mout.warn(weightTray.get() );
		mout.debug2() << scale << mout.endl;
		Histogram histogram(256);
		histogram.setRange(0.0, scale);
		//hist.setParameter
		ImageHistogram histOp;
		histogram.compute(w, w.getType());
		//histOp.computeHistogram(w, histogram);
		mout.warn(histogram );
		// TODO:
		ImageFile::write(w, "diff-grad-w.png");  // actually, w
		//File::write(weightTray.get(), "diff-grad-w.png");  // actually, w

	}


	mout.debug("Computing intermediate source image, for 'final' gradients" );
	// Intensity gradients should not be computed for the 1st or 2nd image, but "between" them.
	// So the images will be mixed here. //// To save memory, src2 is recycled.
	Image srcMixed; // = src2;
	BinaryFunctorOp<MixerFunctor> mixOp;
	mixOp.functor.coeff = 0.5;
	mixOp.makeCompatible(src1.getConf(), srcMixed);
	mixOp.traverseChannel(src1, src2, srcMixed.getChannel(0));
	if (mout.isDebug(3)){
		ImageFile::write(srcMixed, "src-mixed.png");
	}

	ImageTray<const Channel> srcMixedTray;
	srcMixedTray.setChannels(srcMixed);
	gradientOp.traverseChannels(srcMixedTray, gradTray1);
	if (mout.isDebug(4)){
		ImageFile::write(gradTray1.get(0), "diff-dx-mixed.png");
		ImageFile::write(gradTray2.get(1), "diff-dy-mixed.png");
		//exit(0);
	}

	/*
		mout.note("Computing gradient magnitude" );
		// Update Gradient stability as well (emphasize strong gradients)
		Image gradMagnitude;
		gradMagnitude.setLimits(0, 255);
		MagnitudeOp("linear", 2).process(grad, gradMagnitude);
		if (mout.isDebug(3)){
			writeDebugImage(gradMagnitude, "diff-grad-magn.png", 0.2, 0.0);
		}
		MultiplicationOp(255.0).process(w, gradMagnitude, w);
		if (mout.isDebug(3)){
			writeDebugImage(w, "diff-grad-magn-wd.png", 1.0, 0.0);
		}
	 */


}



}
}


// Drain
