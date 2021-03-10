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
#include "drain/image/FilePng.h"

namespace drain
{

namespace image
{


void QualityOverrideOp::traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	//mout.debug() << "src: " << src.getGeometry() << ", dst: "  << dst.getGeometry() << mout;
	mout.info() << "src: " << src << mout;
	mout.info() << "dst: " << dst << mout;
	mout.warn() << getParameters() << mout;

	if (src.getGeometry() != dst.getGeometry()){
		mout.error() << "geometries not same" << mout;
	}

	// assume 1 alpha channel!
	const Channel & srcW = src.getAlpha();
	Channel & dstW = dst.getAlpha();

	const size_t width  = dstW.getWidth();
	const size_t height = dstW.getHeight();

	size_t address;
	quality_t quality;
	const bool DECAY = (decay != 1.0);

	ImageTray<const Channel>::const_iterator sit;
	ImageTray<Channel>::iterator dit;

	for (size_t i = 0; i < width; ++i) {
		for (size_t j = 0; j < height; ++j) {
			address = dstW.address(i, j);
			quality = srcW.get<quality_t>(address);
			if (advantage*quality >= dstW.get<quality_t>(address)){  // NOTE: >= means that new data is priorized
				sit = src.begin();
				dit = dst.begin();
				while (sit != src.end()){
					dit->second.put(address, sit->second.get<double>(address));
					++sit;
					++dit;
				}
				dstW.put(address, decay*quality);
			}
			else if (DECAY) {
				dstW.put(address, decay*dstW.get<quality_t>(address));
			}
		}
	}
}

void QualityOverrideOp::traverseChannels(const ImageTray<const Channel> & src1, const ImageTray<const Channel> & src2, ImageTray<Channel> & dst) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	mout.note() << "src: " << src1.getGeometry() << ", src2: " << src2.getGeometry() <<", dst: "  << dst.getGeometry() << mout.endl;

	if ((src1.getGeometry() != dst.getGeometry()) || (src2.getGeometry() != dst.getGeometry())){
		mout.error() << "geometries not same" << mout.endl;
	}

	// assume 1 alpha channel!
	const Channel & src1W = src1.getAlpha();
	const Channel & src2W = src2.getAlpha();
	Channel & dstW = dst.getAlpha();

	const size_t width  = dstW.getWidth();
	const size_t height = dstW.getHeight();

	size_t address;
	quality_t w1, w2;
	ImageTray<const Channel>::const_iterator sit;
	ImageTray<Channel>::iterator dit;

	for (size_t i = 0; i < width; ++i) {
		for (size_t j = 0; j < height; ++j) {
			address = dstW.address(i, j);
			w1 = decay*src1W.get<quality_t>(address);
			w2 = decay*src2W.get<quality_t>(address);
			//w2 = dstW.get<quality_t>(address);
			if (w1 > w2){
				sit = src1.begin();
				dit = dst.begin();
				while (sit != src1.end()){
					dit->second.put(address, sit->second.get<double>(address));
					++sit;
					++dit;
				}
				dstW.put(address, w1);
			}
			else {
				sit = src2.begin();
				dit = dst.begin();
				while (sit != src2.end()){
					dit->second.put(address, sit->second.get<double>(address));
					++sit;
					++dit;
				}
				dstW.put(address, w2);
			}

		}
	}
}


/*
void QualityOverrideOp::traverseChannels(const ImageTray<const Channel> & src1, const ImageTray<const Channel> & src2,
		ImageTray<Channel> & dst) const {  // RAISE for each
	Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), name+"[3]", __FUNCTION__);
	//mout.error() << "under constr" << mout.endl;

	mout.debug() << "src1:" << src1.getGeometry() << mout.endl;
	mout.debug() << "src2:" << src2.getGeometry() << mout.endl;
	mout.debug() << "dst:"  << dst.getGeometry()  << mout.endl;

	typename ImageTray<const Channel>::map_t::const_iterator sit1 = src1.begin();
	typename ImageTray<const Channel>::map_t::const_iterator sat1 = src1.alpha.begin();
	typename ImageTray<const Channel>::map_t::const_iterator sit2 = src2.begin();
	typename ImageTray<const Channel>::map_t::const_iterator sat2 = src2.alpha.begin();

	typename ImageTray<Channel>::map_t::iterator dit = dst.begin();
	typename ImageTray<Channel>::map_t::iterator dat = dst.alpha.begin();


	int i=0;
	while (true){

		if (sit1 == src1.end()){
			//mout.note() << "src1 ended, ok" << mout.endl;
			if (sit2 != src2.end()){
				mout.warn() << "src1 ended before src2 (src2 had extra channels)" << mout.endl;
			}
			return;
		}
		if (sit2 == src2.end()){
			mout.warn() << "src2 ended before src1" << mout.endl;
			return;
		}
		if (dit == dst.end()){
			mout.warn() << "dst ended before src1,src2" << mout.endl;
			return;
		}

		if (sat1 == src1.alpha.end()){
			mout.debug() << "reuse src1.alpha, ok" << mout.endl;
			sat1 =  src1.alpha.begin();
		}
		if (sat2 == src2.alpha.end()){
			mout.debug() << "reuse src2.alpha, ok" << mout.endl;
			sat2 =  src2.alpha.begin();
		}
		if (dat == dst.alpha.end()){
			mout.debug() << "reuse dst.alpha, ok" << mout.endl;
			dat =  dst.alpha.begin();
		}

		mout.debug() << "calling traverseChannel(s1,sa1, s2,sa2, d,da)" << mout.endl;
		traverseChannel(sit1->second, sat1->second, sit2->second, sat2->second, dit->second, dat->second);

		++sit1; ++sat1;
		++sit2; ++sat2;
		++dit;  ++dat;
	}
}
*/




/*
void QualityOverrideOp::traverseChannel(const Channel & src1, const Channel & src1Weight, const Channel & src2, const Channel & src2Weight,
		Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), name+"[2+2,2]", __FUNCTION__);

	if (decay != 1.0)
		mout.info() << this->getParameters() << mout.endl;

	mout.debug()  << '\n';
	mout << src1 << ",\t " << src1Weight << ",\n";
	mout << src2 << ",\t " << src2Weight << ",\n";
	mout << dst  << ",\t " << dstWeight  << mout.endl;

	// Decayed weight
	float w1F, w2F;
	Image::const_iterator s1    = src1.begin();
	Image::const_iterator s1End = src1.end();
	Image::const_iterator s1w   = src1Weight.begin();
	Image::const_iterator s2    = src2.begin();
	Image::const_iterator s2w   = src2Weight.begin();
	Image::iterator d  = dst.begin();
	Image::iterator dw = dstWeight.begin();

	//int i = 0;

	while (s1 != s1End){
		w1F = static_cast<float>(*s1w);
		//w1F = decay; //static_cast<int>(*s1w);
		//w2F = rand() & 0xbb; // 255
		w2F = static_cast<int>(*s2w);
		// important >= instead of >  (why??)
		if (w1F >= w2F){
			*d  = *s1;
			*dw = w1F;
		}
		else {
			*d  = *s2;
			*dw = w2F;
		}
		// *dw = rand() & 0xbb; // 255
		s1++, s1w++;
		s2++, s2w++;
		d++,  dw++;
	}

}
*/

/*

void QualityOverrideOp::traverseFrame(const Channel &src, const Channel &srcWeight, Channel &dst, Channel &dstWeight) const {

	Logger mout(getImgLog(),__FUNCTION__, __FILE__);

	mout.note() << src << ", " << srcWeight << ", " << dst << ", " << dstWeight << mout.endl;

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
			sF = decay * static_cast<float>(*sw);
			//sF  = static_cast<float>(*sw);
			// important >= instead of >  (why??)
			//if (sF > static_cast<float>(*dw)){
			if (sF > static_cast<float>(*dw)){
 *dw = sF;
 *d  = *s;
			}
			s++,  sw++;
			d++,  dw++;
		}
	}
}


void QualityOverrideOp::traverseFrame(const Channel & src, const Channel & srcWeight, const Channel & src2, const Channel & srcWeight2,
		Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(),__FUNCTION__, __FILE__);

	if (decay != 1.0)
		mout.warn() << "Deprecating param 'decay' (" << decay << ") " << mout.endl;

	mout.warn() << src << ", " << srcWeight << ", " << dst << ", " << dstWeight << mout.endl;

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
 *
 */

}

}

// Drain
