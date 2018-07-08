/*

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

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
