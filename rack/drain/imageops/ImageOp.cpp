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

#include "ImageOp.h"
#include "drain/image/File.h" // debugging


namespace drain
{

namespace image
{

void ImageOp::getDstConf(const ImageConf & src, ImageConf & dst) const {

	if ((dst.getScale()==0.0) || !dst.typeIsSet())
		dst.setEncoding(src.getEncoding());

	dst.setGeometry(src.getGeometry()); //geometry = src.geometry;

	dst.setCoordinatePolicy(src.getCoordinatePolicy());

}


void ImageOp::makeCompatible(const ImageConf & srcConf, Image & dst) const  {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), name+"(ImageOp)", __FUNCTION__);

	// mout.warn() << "srcConf:      " << srcConf << mout.endl;
	// mout.warn() << "dst.getConf:  " << dst.getConf() << mout.endl;

	ImageConf dstConf(dst.getConf());

	/// Coord policy is changed very rarely, so it is copied here, as a default.
	//  Image operator can change it if needed, with getDstConf().
	dstConf.setCoordinatePolicy(srcConf.coordinatePolicy);
	// mout.warn() << "dstConf0:  " << dstConf << mout.endl;

	/// By default, use src type. Image operator changes it if needed (e.g. signed, integer or float).
	//if (!dst.typeIsSet())
	// dst.setType(src.getType());

	/// Should set geometry, including channel geometry.
	getDstConf(srcConf, dstConf);

	if (!dstConf.typeIsSet())
		dstConf.setType(srcConf.getType());

	// mout.warn() << "dstConfRet:" << dstConf << mout.endl;
	// dstConf.setScaling(src.getScaling());
	// mout.warn() << "dstConf2: " << dstConf << mout.endl;
	/*
	if (dst.hasSameSegment(src)){
		if (SEPARATE){
			mout.warn() << "dst == src, SEPARATE tmp coming up? "  << mout.endl;
		}
		mout.debug() << "dst == src, ok" << mout.endl;
		return;
	}
	*/

	// Definitive storage type
	/*
	if (dstConf.typeIsSet()){
		dst.setType(dstConf.getType());
	}

	// Fallback: copy src type.
	if (!dst.typeIsSet()){
		dst.setType(src.getType());
	}
	*/

	/*
	if (!dstConf.isPhysical()){
		mout.warn() << "dstConf (" << dstConf.getScaling() << ") has no physical range , adopting scaling of src (" << srcConf.getScaling() << ')' << mout.endl;
		dstConf.adoptScaling(srcConf, srcConf.getType(), dstConf.getType());
	}
	else {
		mout.debug() << "dst has physical range " << dstConf.getScaling() << mout.endl;
	}
	*/
	const bool ORIG_ALPHA = dst.hasAlphaChannel();

	dst.setConf(dstConf);

	//dst.setGeometry(dstConf.getGeometry());

	/// Some operators, like QualityOverrideOp, do not like modifying the target dst (unless empty)
	if (dst.hasAlphaChannel() && !ORIG_ALPHA){
		double maxValue = dst.getConf().getTypeMax<double>();
		mout.info() << "filling alpha channel with " << maxValue << mout.endl;
		dst.getAlphaChannel().fill(maxValue);
	}

	/*
	mout.warn() << "final dst:    " << dst << mout;
	mout.warn() << "final dst[0]: " << dst.getChannel(0) << mout;
	ImageView view(dst, 0, 1);
	mout.warn() << "final viewDst: " << view << mout;
	*/
}


void ImageOp::makeCompatible2(const ImageFrame & src1, const ImageFrame & src2, Image & dst) const  {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), name+"(ImageOp)", __FUNCTION__);

	mout.debug3() << "src1: " << src1 << mout.endl;
	mout.debug3() << "src2: " << src2 << mout.endl;

	if (dst.hasOverlap(src1)){
		mout.info() << "dst is src1 or has overlap, leaving unmodified" << mout.endl;
		return;
	}

	if (dst.hasOverlap(src2)){
		mout.info() << "dst is src2 or has overlap, leaving unmodified" << mout.endl;
		return;
	}

	makeCompatible(src1.getConf(), dst);

}


void ImageOp::process(const ImageFrame & srcFrame, Image & dstImage) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), this->name+"(ImageOp)[const ImageFrame &, Image &]", __FUNCTION__);

	if (processOverlappingWithTemp(srcFrame, dstImage)){
		return;
	}
	else {
		mout.debug2() << "passed overlap test (directly or using tmp)" << mout.endl;
	}

	mout.special() << "srcFrame:     " << srcFrame << mout;
	mout.debug()   << "srcFrame Conf: " << srcFrame.getConf() << mout;
	//mout.special() << "srcFrame Enc:  " << srcFrame.getEncoding() << mout;
	mout.debug()   << "srcFrame Sca:  " << srcFrame.getScaling() << mout;

	ImageTray<const Channel> srcTray;
	srcTray.setChannels(srcFrame);

	makeCompatible(srcFrame.getConf(), dstImage);
	mout.debug() << "dstIC0: " << dstImage.getChannel(0) << mout.endl;

	ImageTray<Channel> dstTray;
	dstTray.setChannels(dstImage);
	mout.debug() << "dstTC0: " << dstTray.get(0) << mout.endl;

	traverseChannels(srcTray, dstTray);

}

void ImageOp::process(const ImageFrame & src, const ImageFrame & srcWeight, Image & dst, Image & dstWeight) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), this->name+"(ImageOp)[const ImageFrame &, Image &]", __FUNCTION__);


	ImageTray<const Channel> srcTray;
	srcTray.setChannels(src);
	srcTray.setAlphaChannels(srcWeight);

	ImageTray<Image> dstTray;
	dstTray.set(dst);
	dstTray.setAlpha(dstWeight);

	mout.debug() << "srcTray: " << srcTray << mout.endl;
	mout.debug() << "dstTray: " << dstTray << mout.endl;
	// mout.warn() << "dst scaling: " << dstTray.get().getScaling() << mout.endl;

	process(srcTray, dstTray);

}

bool ImageOp::processOverlappingWithTemp(const ImageFrame & srcFrame, Image & dstImage) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), name+"(ImageOp)", __FUNCTION__);

	mout.debug2() << "checking tmp" << mout.endl;

	if (srcFrame.hasOverlap(dstImage)){
		mout.debug() << "overlapping images, tmp image used" << mout.endl;
		Image tmp(dstImage.getType()); // need copy?
		process(srcFrame, tmp);
		dstImage.copyDeep(tmp);
		return true;
	}
	else {
		mout.debug() << "separate images, no tmp image used" << mout.endl;
		return false;
	}
}


bool ImageOp::processOverlappingWithTemp(const ImageTray<const Channel> & srcChannels, ImageTray<Image> & dstImages) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), this->name+"[ImageOp](trays)", __FUNCTION__);

	//mout.debug() << "start" << mout.endl;
	mout.debug() << srcChannels << mout.endl;
	mout.debug() << dstImages   << mout.endl;


	bool IMAGE_OVERLAP = srcChannels.hasOverlap(dstImages);
	bool ALPHA_OVERLAP = srcChannels.alpha.hasOverlap(dstImages.alpha);

	mout.debug() << "overlaps: image:" << IMAGE_OVERLAP << ", alpha=" << ALPHA_OVERLAP << mout.endl;

	if (IMAGE_OVERLAP || ALPHA_OVERLAP){

		mout.info() << "overlap found, using tmp images" << mout.endl;

		ImageTray<Image> dstImagesTmp;

		std::vector<Image> tmp;
		dstImages.createVector(tmp);
		dstImagesTmp.copyVector(tmp);

		std::vector<Image> tmpAlpha;
		dstImages.alpha.createVector(tmpAlpha);
		dstImagesTmp.alpha.copyVector(tmpAlpha);

		mout.debug2() << "created dstImagesTmp:" << dstImagesTmp << mout.endl;

		mout.debug() << "re-calling processConditional(srcChannels, dstImagesTmp, checkOvelap=FALSE)" << mout.endl;

		process(srcChannels, dstImagesTmp, false); // ("false" only speeds up; "true" should not lead to recursion anyway)

		mout.debug2() << "copying tmp images back" << mout.endl;
		ImageTray<Image>::map_t::iterator tit = dstImagesTmp.begin();
		ImageTray<Image>::map_t::iterator dit = dstImages.begin();
		while ((tit != dstImagesTmp.end()) && (dit != dstImages.end())){
			dit->second.copyDeep(tit->second);
			++dit;
			++tit;
		}

		mout.debug2() << "copying tmp alpha back" << mout.endl;
		ImageTray<Image>::map_t::iterator tait = dstImagesTmp.alpha.begin();
		ImageTray<Image>::map_t::iterator dait = dstImages.alpha.begin();
		while ((tait != dstImagesTmp.alpha.end()) && (dait != dstImages.alpha.end())){
			dait->second.copyDeep(tait->second);
			++dait;
			++tait;
		}
		return true;
	}
	else {
		mout.debug2() << "no overlap" << mout.endl;
		return false;
	}

}

void ImageOp::process(const ImageTray<const Channel> & srcChannels, ImageTray<Image> & dstImages, bool checkOverlap) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), this->name+"[ImageOp](c ChannelTray, ImageTray, checkOverlap)", __FUNCTION__);

	mout.debug() << "start" << mout.endl;


	if (checkOverlap) {
		if (processOverlappingWithTemp(srcChannels, dstImages))
			return;
	}
	else {
		mout.debug2() << "skipping overlap check" << mout.endl;
	}

	mout.debug() << "entering main loop (completed checkOverlap)" << mout.endl;

	mout.debug2() << "Creating dst tray of channels" << mout.endl;
	ImageTray<Channel> dstChannels;
	bool alphasCompatible = false;

	ImageTray<const Channel>::map_t::const_iterator  sit = srcChannels.begin();
	ImageTray<const Channel>::map_t::const_iterator sait = srcChannels.alpha.begin();
	ImageTray<Image>::map_t::iterator  dit = dstImages.begin();
	ImageTray<Image>::map_t::iterator dait = dstImages.alpha.begin();


	while ((sit != srcChannels.end()) && (dit != dstImages.end())){

		mout.note() << "round " << sit->first << '/' << dit->first << mout.endl;

		makeCompatible(sit->second.getConf(), dit->second);
		mout.debug() << "srcChannel: " << sit->second << mout.endl;
		mout.debug() << "dstImage:   " << dit->second << mout.endl;

		dstChannels.appendImage(dit->second);

		if (dait == dstImages.alpha.end()){
			dait =  dstImages.alpha.begin();
			alphasCompatible = true; // all W
		}
		else {
			if (srcChannels.hasAlpha()){ // alphas empty?
				if (!alphasCompatible){
					makeCompatible(sait->second.getConf(), dait->second);
				}
				dstChannels.appendAlpha(dait->second);
			}
			++dait;
		}

		if (sait == srcChannels.alpha.end()){
			sait =  srcChannels.alpha.begin();
		}
		else {
			++sait;
		}

		++sit;
		++dit;

	}

	mout.debug2() << "src: " << srcChannels << mout.endl;
	mout.debug2() << "dst: " << dstChannels << mout.endl;

	mout.debug() << "calling traverseChannels" << mout.endl;

	//mout.warn() << "calling " << sit->first << '/' << dit->first << mout.endl;
	traverseChannels(srcChannels, dstChannels);

}






/// Corresponds to processChannelsSeparately
void ImageOp::traverseChannelsSeparately(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL this->name+"(ImageOp::)", __FUNCTION__);

	if (src.empty()){
		mout.error() << "src empty" << mout.endl;
	}

	if (dst.empty()){
		mout.error() << "dst empty" << mout.endl;
	}

	ImageTray<const Channel>::map_t::const_iterator sit  = src.begin();
	ImageTray<Channel>::map_t::iterator dit = dst.begin();

	mout.debug() = "initializeParameters";

	initializeParameters(sit->second, dit->second);

	if (src.hasAlpha()){

		ImageTray<const Channel>::map_t::const_iterator sait = src.alpha.begin();
		ImageTray<Channel>::map_t::iterator dait = dst.alpha.begin();

		while (true){

			mout.special() << "channel #" << dit->first << ": traverseChannel(src, srcAlpha, dst, dstAlpha)" << mout.endl;

			const Channel & srcData  = sit->second;
			const Channel & srcAlpha = sait->second;

			Channel & dstData  = dit->second;
			Channel & dstAlpha = dait->second;

			traverseChannel(srcData, srcAlpha, dstData, dstAlpha); // apply

			if (++sit == src.end())
				return;

			if (++sait == src.alpha.end()) // re-use alpha(s)
				sait = src.alpha.begin();

			if (++dit == dst.end()) // re-use dst ?? warning?
				dit = dst.begin();

			if (++dait == dst.alpha.end()) // re-use alpha(s)
				dait = dst.alpha.begin();

		}
	}
	else {

		while (true){

			//mout.warn() << "invoking traverseChannel(src, dst) for Channel #" << dit->first << mout.endl;
			mout.special() << "channel #" << dit->first << ": traverseChannel(src, dst)" << mout.endl;

			const Channel & srcData  = sit->second;
			Channel & dstData  = dit->second;
			//mout.debug2() << "dst: " << dstData << mout.endl;
			traverseChannel(srcData, dstData); // apply

			if (++sit == src.end())
				return;

			if (++dit == dst.end()) // re-use dst ?? warning?
				dit = dst.begin();

		}

	}

}

void ImageOp::traverseChannelsEqually(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL this->name+"(ImageOp::)", __FUNCTION__);

	/*
	if (src.getGeometry() != dst.getGeometry()){
		mout.error() << "inequal geometry: " << src.getGeometry() << " vs. " << dst.getGeometry() << mout.endl;
	}
	 */

	if (src.size() != dst.size()){
		mout.error() << "inequal geometry: " << src.size() << " vs. " << dst.size() << mout.endl;
	}

	if (src.empty()){
		mout.warn() << "no input" << mout.endl;
		return;
	}

	ImageTray<const Channel>::map_t::const_iterator sit  = src.begin();
	ImageTray<Channel>::map_t::iterator dit = dst.begin();

	initializeParameters(sit->second, dit->second);

	while (sit != src.end()){
		mout.debug2() << "invoke traverseChannel(src, dst) for Channel #" << sit->first << mout.endl;
		traverseChannel(sit->second, dit->second); // apply
		++sit, ++dit;
	}

	ImageTray<const Channel>::map_t::const_iterator sait = src.alpha.begin();
	ImageTray<Channel>::map_t::iterator dait = dst.alpha.begin();

	while (sait != src.alpha.end()){
		mout.debug2() << "invoke traverseChannel(srcAlpha, dstAlpha) for Channel #" << dit->first << mout.endl;
		traverseChannel(sait->second, dait->second); // apply
		++sait, ++dait;
	}
}

void ImageOp::traverseChannelsRepeated(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL this->name+"(ImageOp::)", __FUNCTION__);

	if (src.empty()){
		mout.error() << "src empty" << mout.endl;
	}

	if (dst.empty()){
		mout.error() << "dst empty" << mout.endl;
	}

	const size_t n = std::max(src.size(), dst.size());

	ImageTray<const Channel>::map_t::const_iterator sit  = src.begin();
	ImageTray<Channel>::map_t::iterator dit = dst.begin();

	initializeParameters(sit->second, dit->second);


	if (src.hasAlpha()){

		ImageTray<const Channel>::map_t::const_iterator sait = src.alpha.begin();
		ImageTray<Channel>::map_t::iterator dait = dst.alpha.begin();

		//while (true){
		for (size_t i = 0; i < n; ++i) {

			mout.debug2() << "invoke traverseChannel(src, srcAlpha, dst, dstAlpha) for Channel #" << dit->first << mout.endl;

			const Channel & srcData  = sit->second;
			const Channel & srcAlpha = sait->second;

			Channel & dstData  = dit->second;
			Channel & dstAlpha = dait->second;

			traverseChannel(srcData, srcAlpha, dstData, dstAlpha); // apply

			if (++sit == src.end())
				sit = src.begin();

			if (++sait == src.alpha.end())
				sait = src.alpha.begin();

			if (++dit == dst.end()){
				mout.note() << "revisiting dst channels" << mout.endl;
				dit = dst.begin();
			}

			if (++dait == dst.alpha.end())
				dait = dst.alpha.begin();

		}
	}
	else {

		for (size_t i = 0; i < n; ++i) {

			mout.debug2() << "invoking traverseChannel(src, dst) for Channel #" << dit->first << mout.endl;

			const Channel & srcData  = sit->second;
			Channel & dstData  = dit->second;
			traverseChannel(srcData, dstData); // apply

			if (++sit == src.end())
				sit = src.begin();

			if (++dit == dst.end()) // re-use dst ?? warning?
				dit = dst.begin();

		}

	}

}








}
}


// Drain
