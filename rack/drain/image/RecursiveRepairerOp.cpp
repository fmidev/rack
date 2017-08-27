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

#include "RecursiveRepairerOp.h"
#include "CopyOp.h"
#include "FastAverageOp.h"
#include "QualityOverrideOp.h"

#include "FilePng.h"

namespace drain
{

namespace image
{

/** Image restoration utility applying recursive interpolation from neighboring pixels. 
 * 	
 */

/// The main functionality called by filter() after image compatibility check and tmp allocations
void RecursiveRepairerOp::traverse(const Image &src, const Image &srcAlpha, Image &dst, Image &dstAlpha) const {


	MonitorSource mout(iMonitor, name, __FUNCTION__);

	if (!dst.isSame(src))
		dst.copyDeep(src);

	/// This is important with multi-channel targets in order to reset the shared alpha channel. (Otherwise 1st channel would complete it.)
	if (!dstAlpha.isSame(srcAlpha))
		dstAlpha.copyDeep(srcAlpha);

	//FilePng::write(src, name+"-d.png");
	//FilePng::write(srcAlpha, name+"-q.png");
	//FilePng::write(dstAlpha, name+"-Q.png");

	FastAverageOp avg(width,height);
	mout.debug(2) << avg << mout.endl;

	QualityOverrideOp q; //q(decay);
	mout.debug(2) << q << mout.endl;

	Image blurred(src);
	Image blurredAlpha(srcAlpha);

	for (int i=0; i<loops; ++i){

		mout.debug(1) << " loop=" << i << mout.endl;

		/*
		if (i==0){
			mout.debug(3) << "avg" << mout.endl;
			avg.traverse(src, srcAlpha,  blurred, blurredAlpha);
			mout.debug(3) << "q" << mout.endl;
			q.traverse(src, srcAlpha, blurred, blurredAlpha,  dst, dstAlpha);
		}
		else {
		*/

		// More elegant this way
		mout.debug(3) << "avg" << mout.endl;
		avg.traverse(dst, dstAlpha,  blurred, blurredAlpha);

		mout.debug(3) << "q" << mout.endl;
		q.traverse(blurred, blurredAlpha,  dst, dstAlpha);
		//}

		if (mout.isDebug(15)){
			std::stringstream sstr;
			sstr << name;
			sstr.width(2);
			sstr.fill('0');
			sstr << i;
			//std::cerr << sstr.str() << std::endl;
			FilePng::write(blurred, sstr.str()+"-sd.png");
			FilePng::write(blurredAlpha, sstr.str()+"-sq.png");
			FilePng::write(dst, sstr.str()+"-td.png");
			FilePng::write(dstAlpha, sstr.str()+"-tq.png");
		}

	}



}


}  // image::

}  // drain::

// Drain
