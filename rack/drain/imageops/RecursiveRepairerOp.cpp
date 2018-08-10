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

#include "image/FilePng.h"

#include "RecursiveRepairerOp.h"
#include "CopyOp.h"
#include "FastAverageOp.h"
#include "QualityOverrideOp.h"


#include "ImageOpBank.h"

namespace drain
{

namespace image
{


/** Image restoration utility applying recursive interpolation from neighboring pixels. 
 * 	
 */

//void RecursiveRepairerOp::traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {

void RecursiveRepairerOp::traverseChannels(const ImageTray<const Channel> &src, ImageTray<Channel> &dst) const {


	Logger mout(getImgLog(), name, __FUNCTION__);

	// mout.warn() << "src: " << src << mout.endl;
	// mout.warn() << "dst: " << dst << mout.endl;
	mout.debug() << "src:\n" << src << mout.endl;
	mout.debug() << "dst:\n" << dst << mout.endl;


	if (mout.isDebug(15)){
		FilePng::write(src.getAlpha(), name+"-sq.png");
		FilePng::write(dst.getAlpha(), name+"-dq.png");
		//FilePng::write(src, name+"-d.png");
		//FilePng::write(srcAlpha, name+"-q.png");
	}
	//FilePng::write(dstAlpha, name+"-Q.png");
	int loops = this->loops;
	if (loops == 0){
		mout.note() << "zero loops, so just copying data" << mout.endl;
		CopyOp copy;
		copy.traverseChannels(src, dst);
		//copy.traverseChannel(src, dst);
		//copy.traverseChannel(srcAlpha, dstAlpha);
		return;
	}

	std::string smoother("average");

	if (!this->smoother.empty()){
		switch (this->smoother.at(0)) {
			case 'a':
				smoother = "average";
				break;
			case 'g':
				smoother = "gaussianAverage";
				break;
			case 'd':
				smoother = "distanceTransformFill";
				if (loops > 1){
					mout.note() << "discarding further loops (" << loops << "), inapplicable with " << smoother << mout.endl;
					loops = 1;
				}
				break;
			case 'D':
				smoother = "distanceTransformFillExp";
				if (loops > 1){
					mout.note() << "discarding further loops (" << loops << "), inapplicable with " << smoother << mout.endl;
					loops = 1;
				}
				break;
			default:
				mout.warn() << "invalid smoother(" << smoother << "), using average"<< mout.endl;
				break;
		}
	}


	//FastAverageOp avg(width, height);
	ImageOpBank & bank = getImageOpBank();
	if (!bank.has(smoother)){
		mout.note() << bank << mout.endl;
		mout.error() << "invalid smoother: " << smoother << mout.endl;
		return;
	}

	ImageOp & smootherOp = getImageOpBank().get(smoother).get();
	smootherOp.setParameter("width", width);
	smootherOp.setParameter("height", height);

	mout.debug() << smootherOp << mout.endl;

	QualityOverrideOp qualityOverrideOp; //qualityOverrideOp(decay);
	mout.debug(2) << qualityOverrideOp << mout.endl;

	Image blurred(src.get().getType(), src.getGeometry());
	//Image blurredAlpha(src.getAlpha());
	blurred.setName("blurred");
	mout.debug() << blurred << mout.endl;
	//blurredAlpha.setName("blurredAlpha");
	//mout.warn() << blurredAlpha << mout.endl;

	ImageTray<Channel> blur;
	blur.setChannels(blurred);
	//blur.appendAlpha(blurredAlpha);

	//ImageTray<const Channel> blurC;
	//blurC.setChannels(blurred);

	mout.debug() << '\n' << blur  << mout.endl;
	//mout.debug() << '\n' << blurC << mout.endl;

	//makeCompatible(src, blurred);
	//makeCompatible(srcAlpha, blurredAlpha);

	mout.debug(3) << "avg" << mout.endl;
	//smootherOp.traverseChannel(src, srcAlpha,  blurred, blurredAlpha);
	smootherOp.traverseChannels(src, blur);
	mout.debug(3) << "qualityOverrideOp" << mout.endl;
	//FilePng::write(blurred, name+"-b.png");
	//FilePng::write(dst.get(1), name+"-D.png");


	qualityOverrideOp.traverseChannels(blur, src, dst);

	// FilePng::write(dst.get(1), name+"-d.png");
	// traverseChannel(src, srcAlpha, blurred, blurredAlpha,  dst, dstAlpha);

	//ImageTray<const Channel> dstC;
	//dstC.setChannels(dst);
	//mout.debug(1) << dstC << mout.endl;


	for (int i=1; i<loops; ++i){

		mout.debug(1) << " loop=" << i << mout.endl;

		if (mout.isDebug(15)){
			std::stringstream sstr;
			sstr << name;
			sstr.width(2);
			sstr.fill('0');
			sstr << i;
			//std::cerr << sstr.str() << std::endl;

			FilePng::write(blurred.getChannel(1), sstr.str()+"-bd.png");
			FilePng::write(blurred.getAlphaChannel(), sstr.str()+"-bq.png");
			//FilePng::write(blurredAlpha, sstr.str()+"-bq.png");
			FilePng::write(dst.get(1), sstr.str()+"-td.png");
			FilePng::write(dst.getAlpha(), sstr.str()+"-tq.png");

		}

		// More elegant this way
		mout.debug(3) << "smoother" << mout.endl;
		smootherOp.traverseChannels(dst, blur);
		//FilePng::write(blur.(1), name+"-b.png");

		mout.debug(3) << "qualityOverrideOp" << mout.endl;
		//qualityOverrideOp.traverseChannels(blurC, dstC, dst);
		qualityOverrideOp.traverseChannels(blur, dst);


	}



}


}  // image::

}  // drain::

// Drain
