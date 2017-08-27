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


#include "QualityOverrideOp.h"

namespace drain
{

namespace image
{

//makeCompatible(src,dst);
//makeCompatible(srcWeight,dstWeight);


void QualityOverrideOp::traverse(const Image &src, const Image &srcWeight, Image &dst, Image &dstWeight) const {

	MonitorSource mout(iMonitor,__FILE__, __FUNCTION__);

	mout.debug(4) << src << ", " << srcWeight << ", " << dst << ", " << dstWeight << mout.endl;
	if (decay != 1.0)
		mout.warn() << "Deprecating param 'decay' (" << decay << ") " << mout.endl;

	for (size_t k=0; k<src.getChannelCount(); k++){

		// (Decayed) weight
		float sF;
		Image::const_iterator s    = src.getChannel(k).begin();
		Image::const_iterator sEnd = src.getChannel(k).end();
		Image::const_iterator sw  = srcWeight.begin();
		Image::iterator d = dst.getChannel(k).begin();
		Image::iterator dw = dstWeight.begin();
		while (s != sEnd){
			//sF = decay * static_cast<float>(*sw);
			sF  = static_cast<float>(*sw);
			// important >= instead of >  (why??)
			if (sF > static_cast<float>(*dw)){
				*dw = sF;
				*d  = *s;
			}
			s++,  sw++;
			d++,  dw++;
		}
	}
}


void QualityOverrideOp::traverse(const Image &src, const Image &srcWeight, const Image &src2, const Image &srcWeight2, Image &dst, Image &dstWeight) const {

	MonitorSource mout(iMonitor,__FILE__, __FUNCTION__);

	mout.debug(4) << src << ", " << srcWeight << ", " << dst << ", " << dstWeight << mout.endl;

	for (size_t k=0; k<src.getChannelCount(); k++){

		// Decayed weight
		float sF,sF2;
		Image::const_iterator s    = src.getChannel(k).begin();
		Image::const_iterator sEnd = src.getChannel(k).end();
		Image::const_iterator sw  = srcWeight.begin();
		Image::const_iterator s2   = src2.getChannel(k).begin();
		Image::const_iterator sw2 = srcWeight2.begin();
		Image::iterator d = dst.getChannel(k).begin();
		Image::iterator dw = dstWeight.begin();
		while (s != sEnd){
			//sF = decay * static_cast<float>(*sw);
			sF  = static_cast<float>(*sw);
			sF2 = static_cast<float>(*sw2);
			// important >= instead of >  (why??)
			//if (sF >= static_cast<float>(*dw)){
			//if (sF > static_cast<float>(*dw)){
			if (sF >= sF2){
				*dw = sF;
				*d  = *s;
			}
			else {
				*dw = sF2;
				*d  = *s2;
			}
			s++,  sw++;
			s2++, sw2++;
			d++,  dw++;
		}
	}
}


}

}

// Drain
