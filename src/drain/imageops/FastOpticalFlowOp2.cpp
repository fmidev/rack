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
#include "drain/util/Log.h"

#include "DistanceTransformOp.h"
#include "drain/util/FunctorPack.h"
#include "FunctorOp.h"
#include "GaussianAverageOp.h"
#include "ResizeOp.h"

#include "ImageModifierPack.h"


#include "FastOpticalFlowOp2.h"

namespace drain
{

namespace image
{


void OpticalFlowCore2::setSrcFrames(const ImageTray<const Channel> & srcTray){

	Logger mout(getImgLog(), "SlidingOpticalFlow2", __FUNCTION__);

	const size_t N = getDiffChannelCount();

	if (srcTray.size() < getDiffChannelCount())
		mout.error() << "src has less channels than needed = " << N << mout.endl;

	// For general coord handling etc, native src must be set!
	//Window<OpticalFlowConfig>::setSrcFrames(srcTray.get());
	mout.debug() << "setting accumulation channels" << mout.endl;

	GXX.setView(srcTray.get(0));
	GXY.setView(srcTray.get(1));
	GYY.setView(srcTray.get(2));
	GXT.setView(srcTray.get(3));
	GYT.setView(srcTray.get(4));
	//mout.warn() << "GXX:" << GXX << mout.endl;
	//File::write(GXX, "GXX1.png");

	//mout.warn() << "setting alpha, if found" << mout.endl;
	/// Set quality (weight) channel
	if (srcTray.hasAlpha()){
		mout.info() << "set alpha from src.alpha() " << mout.endl;
		//srcWeight.setView(srcTray.getAlpha());
		setSrcFrameWeight(srcTray.getAlpha());
	}
	else if (srcTray.size() > N) {
		mout.note() << "set alpha from src.get(" << N << ") " << mout.endl;
		//srcWeight.setView(srcTray.get(3));
		setSrcFrameWeight(srcTray.get(N));
	}
	else {
		mout.warn() << "no alpha to set?" << mout.endl;
	}

	if ((srcTray.size()+srcTray.alpha.size()) > (N+1))
		mout.note() << "src has more than " << (N+1) << " channels?" << mout.endl;
}


void FastOpticalFlow2Op::preprocess(const Channel & srcImage, const Channel & srcWeight, Image & dstImage, Image & dstWeight) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	drain::image::ImageFrame const *srcImagePtr  = &srcImage;
	drain::image::ImageFrame const *srcWeightPtr = &srcWeight;

	dstImage.adoptScaling(srcImage.getConf());
	dstWeight.adoptScaling(srcWeight.getConf());

	mout.debug() << "src:  " << srcImage  << mout.endl;
	mout.debug() << "srcW: " << srcWeight << mout.endl;
	mout.debug() << "dst:  " << dstImage  << mout.endl;
	mout.debug() << "dstW: " << dstWeight << mout.endl;


	Image mask; //srcData.createSimpleQualityData(mask, 1.0, 0.0, 1.0);

	if (optResize()){
		mout.debug() << "rescale: " << resize << mout.endl;
		drain::image::ResizeOp op;
		if (resize > 1.0){
			op.setWidth(resize);
			op.setHeight((resize*srcImage.getHeight())/srcImage.getWidth());
			if (resize < 10.0){
				mout.warn() << "resized geometry suspiciously small " << op.getParameters() << mout.endl;
			}
		}
		else { // shrink 0%...100%
			op.setWidth(resize * srcImage.getWidth());
			op.setHeight(resize * srcImage.getHeight());
		}
		mout.info() << "run: " << op.getName() << ':' << op.getParameters() << mout.endl;
		op.process(*srcImagePtr,  dstImage);
		op.process(*srcWeightPtr, dstWeight);
		srcImagePtr  = & dstImage;
		srcWeightPtr = & dstWeight;

	}

	if (optThreshold()){
		UnaryFunctorOp<ThresholdFunctor> op;
		srcImage.getConf().requestPhysicalMin();
		op.functor.threshold = threshold;
		op.functor.replace = srcImage.getConf().requestPhysicalMin();
				//srcImage.getScaling().inv(srcImage.getMin<double>());
		//mout.warn() << "running: " << op.getParameters() << mout.endl;
		mout.info() << "run: " << op.getName() << ':' << op.getParameters() << mout.endl;
		op.process(*srcImagePtr, dstImage);
		srcImagePtr   = & dstImage;
	}

	if (optSpread()|| optSmoother()){
		UnaryFunctorOp<ThresholdFunctor> op;
		op.functor.threshold = 0.2;
		mout.info() << "creating mask: " << op.getName() << ':' << op.getParameters() << mout.endl;
		op.process(*srcWeightPtr, mask);
		//File::write(mask, "mask.png");
	}

	if (optSmoother()){
		drain::image::GaussianAverageOp op;
		op.setSize(conf.frame.width, conf.frame.height);
		mout.info() << "run: " << op.getName() << ':'<< op.getParameters() << mout.endl;
		op.process(*srcImagePtr, *srcWeightPtr, dstImage, dstWeight);
		srcImagePtr  = & dstImage;
		srcWeightPtr = & dstWeight;
	}

	if (optSpread()){
		//drain::image::DistanceTransformFillExponentialOp op;
		drain::image::DistanceTransformExponentialOp op;
		op.setRadius(conf.frame.width, conf.frame.height);
		mout.info() << "run: " << op.getName() << ':' << op.getParameters() << mout.endl;
		op.process(*srcImagePtr,  dstImage);
		op.process(*srcWeightPtr, dstWeight);
		srcImagePtr  = & dstImage;
		srcWeightPtr = & dstWeight;
	}

	if (optSmoother() || optSpread()){
		BinaryFunctorOp<MultiplicationFunctor>().process(mask,  dstWeight);
		//File::write(srcQualityMod.data, "data.png");
		//File::write(mask, "mask.png");
	}

	//Image tmp, tmpQuality;
	//const size_t w = bean.conf.width/4;
	//const size_t h = bean.conf.height/4;

}


/// Computes a differential image with channels dx, dy, dt and w (quality of gradients).
void FastOpticalFlow2Op::computeDifferentials(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	// Source images (intensity)
	if (src.size() < 2) {
		mout.error() << "src image tray must have at least 2 channels, now ("<< src.size() << ")" << mout.endl;
	}
	else if (src.size() > 2) {
		mout.warn() << "src image set has more than 2 channels ("<< src.size() << "), using [0] and [1]" << mout.endl;
	}

	const Channel & src1 = src.get(0);
	const Channel & src2 = src.get(1);

	// Use source alphas, if provided
	const int a = src.getGeometry().channels.getAlphaChannelCount();
	const bool USE_ALPHA  = (a>0);
	const bool USE_ALPHA2 = (a>1);

	const Channel & alpha1 = USE_ALPHA  ? src.getAlpha(0) : src.get(0); // = dummy
	const Channel & alpha2 = USE_ALPHA2 ? src.getAlpha(1) : alpha1    ; // = shared

	if (USE_ALPHA){

		if (USE_ALPHA2){
			mout.note() << "using separate alpha channels for both inputs, ok" << mout.endl;
		}
		else {
			mout.note() << "using shared alpha channel, ok" << mout.endl;
		}

		if (a>2) {
			mout.warn() << "several alpha channels ("<< a << "), using [0] and [1] only..." << mout.endl;
		}

		mout.note() << src.alpha << mout.endl;

	}
	else {
		mout.debug() << " no alpha channel (quality weight) in src data, ok" << mout.endl;
	}

	// Source images (differentials)
	if (dst.size() < getDiffChannelCount()) {
		mout.error() << "dst image tray does not have enough channels ("<< getDiffChannelCount() << ")" << mout.endl;
	}

	// ! Scale should be set already in getSrc().
	const double scale = src.get().getConf().requestPhysicalMax(10.0); // easily returns 255.0 for unsigned char images
	mout.debug2() << " src: " << src1 << mout.endl;

	for (size_t i = 0; i < dst.size(); ++i) {
		Channel & d = dst.get(i);
		if (!d.getScaling().isPhysical()){
			mout.warn() << "would like to use physical scaling +/-" << scale << " for diff channel #" << i << mout.endl;
			//  => =>  d.setOptimalScale(-scale, scale); // pix?
		}
		mout.debug() << "diff channel #" << i << "scaling: " << d.getScaling() << mout.endl;
	}

	if (mout.isDebug(20)){
		ImageFile::write(src.get(0), "src1.png");
		ImageFile::write(src.get(0), "src2.png");
	}


	Channel & GXX = dst.get(0);
	Channel & GXY = dst.get(1);
	Channel & GYY = dst.get(2);
	Channel & GXT = dst.get(3);
	Channel & GYT = dst.get(4);
	Channel &   Q = dst.getAlpha(); // quality, always available (in OpticalWindowCore)

	// Clear (important !)
	dst.getAlpha().clear(); // esp. this!
	for (ImageTray<Channel>::iterator it = dst.begin(); it != dst.end(); ++it){
		it->second.clear();
	}

	/* speedup
  	drain::Point2D p;
	std::vector<double> empty(dst.size(), 0);
	for (std::size_t k=0; k<OFFSET; ++k){
		p.setLocation(0,k);
		for (p.x=0; p.x<src1.getWidth(); ++p.x){
			dst.putPixel(p, empty);
			this->dstWeight.put(p, 0);
		}
	}
	 */

	data_t w;

	data_t dx1;
	data_t dy1;
	data_t dt;
	data_t dx2;
	data_t dy2;

	data_t dx;
	data_t dy;


	const int OFFSET = 1;
	const double SPAN = static_cast<double>(2*OFFSET);
	const std::size_t iLimit = src1.getWidth() -OFFSET;
	const std::size_t jLimit = src1.getHeight()-OFFSET;
	// pixel index
	std::size_t address;
	const int DX = OFFSET;
	const int DY = OFFSET*src1.getWidth(); //row address

	drain::FuzzyBell2<double> scaleW(0.0, 10.0);

	w = 1.0; // constant, if ! USE_ALPHA
	for (std::size_t j=OFFSET; j<jLimit; ++j){
		for (std::size_t i=OFFSET; i<iLimit; ++i){

			// TODO check weight!
			address = src1.address(i,j);

			if (USE_ALPHA){
				if (!checkQuality(alpha1, address, DX, DY)){
					Q.put(address, 0.0);
					continue;
				}
				if (USE_ALPHA2 && !checkQuality(alpha2, address, DX, DY)){
					Q.put(address, 0.0);
					continue;
				}
				w = alpha1.getScaled(i,j)*alpha2.getScaled(i,j);
			}

			dx1 = (src1.get<data_t>(address+DX) - src1.get<data_t>(address-DX)) / SPAN;
			dy1 = (src1.get<data_t>(address+DY) - src1.get<data_t>(address-DY)) / SPAN;

			dx2 = (src2.get<data_t>(address+DX) - src2.get<data_t>(address-DX)) / SPAN;
			dy2 = (src2.get<data_t>(address+DY) - src2.get<data_t>(address-DY)) / SPAN;

			dt  = (src2.get<data_t>(address)    - src1.get<data_t>(address));


			dx = (dx1+dx2)/2.0;
			dy = (dy1+dy2)/2.0;

			GXX.put(address, w*dx*dx);
			GXY.put(address, w*dx*dy);
			GYY.put(address, w*dy*dy);

			GXT.put(address, w*dx*dt);
			GYT.put(address, w*dy*dt);

			/*
			if ((i == j)&& ((i&7) == 0)){
				std::cerr << '[' << i << ',' << j  << "]  \t"
						<< w*dx*dx << '\t'
						<< w*dx*dy << '\t'
						<< w*dy*dy << '\t'
						<< w*dx*dt << '\t'
						<< w*dy*dt << '\t'
					<< "# " << dx << ',' << dy << '\n';
			}
			 */

			/// Gradient stability measure

			dx = (dx2-dx1);
			dy = (dy2-dy1);
			Q.put(address, w*scaleW((dx*dx + dy*dy)));
			//W.put(address, w);

		}
	}

	/*
	File::write(GXX, "GXX.png");
	File::write(GXY, "GXY.png");
	File::write(GYY, "GYY.png");
	File::write(GXT, "GXT.png");
	File::write(GYT, "GYT.png");
	File::write(Q,   "W.png");
	 */

}



}
}


// Drain
