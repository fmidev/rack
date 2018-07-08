/**

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

#include "ImageOp.h"



namespace drain
{

namespace image
{

void ImageOp::process(const ImageFrame & srcFrame, Image & dstImage) const {

	drain::Logger mout(getImgLog(), this->name+"(ImageOp)[const ImageFrame &, Image &]", __FUNCTION__);

	if (processOverlappingWithTemp(srcFrame, dstImage)){
		return;
	}
	else {
		mout.debug(1) << "passed overlap test (directly or using tmp)" << mout.endl;
	}


	ImageTray<const Channel> srcTray;
	srcTray.setChannels(srcFrame);

	makeCompatible(srcFrame, dstImage);
	mout.debug() << "dstIC0: " << dstImage.getChannel(0) << mout.endl;

	ImageTray<Channel> dstTray;
	dstTray.setChannels(dstImage);
	mout.debug() << "dstTC0: " << dstTray.get(0) << mout.endl;

	//process(srcTray, dstTray);
	traverseChannels(srcTray, dstTray);

	/*
	if (srcFrame.hasOverlap(dstImage)){
		mout.debug() << "src has overlap with dst..." << mout.endl;
	}

	mout.debug(1) << "calling makeCompatible()" << mout.endl;

	makeCompatible(srcFrame, dstImage);
	initializeParameters(srcFrame, dstImage);

	mout.debug(1) << "forwarding to: traverseChannels(const ImageTray &, ImageTray &)" << mout.endl;

	ImageTray<const Channel> srcTray;
	srcTray.setChannels(srcFrame);

	ImageTray<Channel> dstTray;
	dstTray.setChannels(dstImage);

	traverseChannels(srcTray, dstTray);
	 */
}

void ImageOp::process(const ImageFrame & src, const ImageFrame & srcWeight, Image & dst, Image & dstWeight) const {

	drain::Logger mout(getImgLog(), this->name+"(ImageOp)[const ImageFrame &, Image &]", __FUNCTION__);


	ImageTray<const Channel> srcTray;
	srcTray.setChannels(src);
	srcTray.setAlphaChannels(srcWeight);


	ImageTray<Image> dstTray;
	//makeCompatible(src, dst);
	//makeCompatible(srcWeight, dstWeight);
	dstTray.set(dst);
	dstTray.setAlpha(dstWeight);

	mout.debug() << "srcTray: " << srcTray << mout.endl;
	mout.debug() << "dstTray: " << dstTray << mout.endl;
	// mout.warn() << "dst scaling: " << dstTray.get().getScaling() << mout.endl;

	process(srcTray, dstTray);

	/*
	if (src.hasOverlap(dst)){
		mout.debug() << "src has overlap with dst..." << mout.endl;
	}
	if (srcWeight.hasOverlap(dstWeight)){
		mout.debug() << "src has overlap with dst..." << mout.endl;
	}

	// mout.debug() << "dst0 scaling: " << dst.getScaling() << mout.endl;

	if (processOverlappingWithTemp(src, srcWeight, dst, dstWeight)){
		return;
	}

	// mout.warn() << "dst1 scaling: " << dst.getScaling() << mout.endl;
	makeCompatible(src, dst);
	// mout.warn() << "dst2 scaling: " << dst.getScaling() << mout.endl;

	makeCompatible(srcWeight, dstWeight);


	initializeParameters(src, dst);
	// mout.warn() << "dst scaling: " << dst.getScaling() << mout.endl;

	ImageTray<const Channel> srcTray; //(src, srcWeight);
	srcTray.setChannels(src);
	srcTray.setAlphaChannels(srcWeight);

	ImageTray<Channel> dstTray; // (dst, dstWeight);
	dstTray.setChannels(dst);
	dstTray.setAlphaChannels(dstWeight);

	// mout.warn() << "dst scaling: " << dstTray.get().getScaling() << mout.endl;

	traverseChannels(srcTray, dstTray);
	*/
}

bool ImageOp::processOverlappingWithTemp(const ImageFrame & srcFrame, Image & dstImage) const {

	drain::Logger mout(getImgLog(), name+"(ImageOp)", __FUNCTION__);

	mout.debug(1) << "checking tmp" << mout.endl;

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

	drain::Logger mout(getImgLog(), this->name+"(trays)", __FUNCTION__);

	mout.debug() << "start" << mout.endl;

	bool IMAGE_OVERLAP = srcChannels.hasOverlap(dstImages);
	bool ALPHA_OVERLAP = srcChannels.alpha.hasOverlap(dstImages.alpha);

	if (IMAGE_OVERLAP || ALPHA_OVERLAP){

		mout.info() << "overlap found, using tmp images" << mout.endl;

		ImageTray<Image> dstImagesTmp;

		std::vector<Image> tmp;
		dstImages.createVector(tmp);
		dstImagesTmp.copyVector(tmp);

		std::vector<Image> tmpAlpha;
		dstImages.alpha.createVector(tmpAlpha);
		dstImagesTmp.alpha.copyVector(tmpAlpha);

		mout.debug(1) << "created dstImagesTmp:" << dstImagesTmp << mout.endl;

		mout.debug() << "re-calling processConditional(srcChannels, dstImagesTmp, checkOvelap=FALSE)" << mout.endl;

		process(srcChannels, dstImagesTmp, false); // ("false" only speeds up; "true" should not lead to recursion anyway)

		mout.debug(1) << "copying tmp images back" << mout.endl;
		ImageTray<Image>::map_t::iterator tit = dstImagesTmp.begin();
		ImageTray<Image>::map_t::iterator dit = dstImages.begin();
		while ((tit != dstImagesTmp.end()) && (dit != dstImages.end())){
			dit->second.copyDeep(tit->second);
			++dit;
			++tit;
		}

		mout.debug(1) << "copying tmp alpha back" << mout.endl;
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
		mout.debug(1) << "no overlap" << mout.endl;
		return false;
	}

}

void ImageOp::process(const ImageTray<const Channel> & srcChannels, ImageTray<Image> & dstImages, bool checkOverlap) const {

	drain::Logger mout(getImgLog(), this->name+"[ImageOp](c ChannelTray, ImageTray, checkOverlap)", __FUNCTION__);

	mout.debug() << "start" << mout.endl;


	if (checkOverlap) {
		if (processOverlappingWithTemp(srcChannels, dstImages))
			return;
	}
	else {
		mout.debug(1) << "skipping overlap check" << mout.endl;
	}

	mout.debug() << "entering main loop (passed checkOvelap)" << mout.endl;

	mout.debug(1) << "Creating dst tray of channels" << mout.endl;
	ImageTray<Channel> dstChannels;
	bool alphasCompatible = false;

	ImageTray<const Channel>::map_t::const_iterator  sit = srcChannels.begin();
	ImageTray<const Channel>::map_t::const_iterator sait = srcChannels.alpha.begin();
	ImageTray<Image>::map_t::iterator  dit = dstImages.begin();
	ImageTray<Image>::map_t::iterator dait = dstImages.alpha.begin();


	while ((sit != srcChannels.end()) && (dit != dstImages.end())){

		mout.debug() << "round " << sit->first << '/' << dit->first << mout.endl;

		makeCompatible(sit->second, dit->second);
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
					makeCompatible(sait->second, dait->second);
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

	mout.debug(1) << "src: " << srcChannels << mout.endl;
	mout.debug(1) << "dst: " << dstChannels << mout.endl;

	mout.debug() << "calling processChannels" << mout.endl;

	//mout.warn() << "calling " << sit->first << '/' << dit->first << mout.endl;
	traverseChannels(srcChannels, dstChannels);

}




void ImageOp::makeCompatible(const ImageFrame & src, Image & dst) const  {

	drain::Logger mout(getImgLog(), name+"(ImageOp)", __FUNCTION__);

	mout.debug(2) << "src: " << src << mout.endl;
	// mout.warn() << "dst0: " << dst << mout.endl;

	if (dst.isSame(src)){
		mout.debug() << "dst == src, ok" << mout.endl;
		return;
	}

	if (!dst.typeIsSet()){
		mout.debug() << "dst type unset, adopting type and scale: " << dst << mout.endl; // Type::getTypeChar(src.getType())
		dst.setType(src.getType());
	}

	if (!dst.getScaling().isPhysical()){
		mout.debug() << "dst (" << dst.getScaling() << ") has no physical range , adopting scaling of src(" << src.getScaling() << ')' << mout.endl;
		dst.adoptScaling(src);
	}
	else {
		mout.debug() << "dst has physical range " << dst.getScaling() << mout.endl;
	}



	dst.setGeometry(src.getGeometry());

	mout.debug(1) << " dst:  " << dst << mout.endl;
	//mout.debug(1) << " dst0: " << dst.getChannel(0) << mout.endl;



	/// TODO: copy alpha, fill alpha?

	//if (dst.setGeometry(src.getGeometry())){
	if (dst.hasAlphaChannel()){
		mout.warn() << "filling alpha channel" << mout.endl;
		dst.getAlphaChannel().fill(dst.getMax<int>());
	}
	//}

	dst.setCoordinatePolicy(src.getCoordinatePolicy());

	mout.debug(3) << "dst: " << dst << mout.endl;

}


void ImageOp::makeCompatible(const ImageFrame & src1, const ImageFrame & src2, Image & dst) const  {

	drain::Logger mout(getImgLog(), name+"(ImageOp)", __FUNCTION__);

	mout.debug(2) << "src1: " << src1 << mout.endl;
	mout.debug(2) << "src2: " << src2 << mout.endl;

	if (dst.hasOverlap(src1)){
		mout.info() << "dst is src1 or has overlap, leaving unmodified" << mout.endl;
		return;
	}

	if (dst.hasOverlap(src2)){
		mout.info() << "dst is src2 or has overlap, leaving unmodified" << mout.endl;
		return;
	}

	makeCompatible(src1, dst);

}

/*
void ImageOp::traverseFrame(const ImageFrame & src, ImageFrame & dst) const {
	drain::Logger mout(this->name+"(ImageOp)[src,dst]", __FUNCTION__);
	mout.debug(1) << "forwarding to: traverseChannels(const ImageTray &, ImageTray &)" << mout.endl;
	const ImageTray<const Channel> srcChannels(src);
	ImageTray<Channel> dstChannels(dst);
	traverseChannels(srcChannels, dstChannels);
}
 */



/*
void ImageOp::processWithTemp(const ImageFrame & srcFrame, Image & dstImage) const {
	drain::Logger mout(getImgLog(), name+"(ImageOp)", __FUNCTION__);
	mout.note() << "checking tmp" << mout.endl;
	Image tmp;
	makeCompatible(srcFrame, tmp);
	initializeParameters(srcFrame, tmp);
	traverseFrame(srcFrame, tmp);
	dstImage.copyDeep(tmp);
}
 */


/*
*/

/*
bool ImageOp::processOverlappingWithTemp(const ImageFrame & src, const ImageFrame & srcWeight, Image & dst, Image & dstWeight) const {

	drain::Logger mout(getImgLog(), name+"(ImageOp)", __FUNCTION__);

	mout.note() << "checking..." << mout.endl;

	if (src.hasOverlap(dst) || srcWeight.hasOverlap(dstWeight)){ // basically also src-dstWeight and srcWeight-dst ...
		mout.note() << "overlapping images, tmp images used" << mout.endl;
		Image tmp(dst); // need copy?
		Image tmpWeight(dstWeight); // need copy?
		process(src, srcWeight, tmp, tmpWeight);
		dst.copyDeep(tmp);
		dstWeight.copyDeep(tmpWeight);
		return true;
	}
	else {
		mout.info() << "separate images, no tmp image used" << mout.endl;
		return false;
	}
}
*/

/*  KEEP THIS UNTIL process(4) fixed
bool ImageOp::processOverlappingClipWithTemp(const Clip<const ImageFrame> & src, Clip<Image> & dst) const {

	drain::Logger mout(getImgLog(), this->getName() + "(ImageOp)[const Clip<const ImageFrame> &, Clip<Image> &]", __FUNCTION__);

	if (src.get().hasOverlap(dst.get())){
		mout.debug() << "overlap detected, using tmp" << mout.endl;
		Image tmp, tmpAlpha;
		Clip<Image> dst2(tmp);
		if (dst.hasAlpha())
			dst2.alpha.append(tmpAlpha);
		process(src, dst2);
		dst.get().copyDeep(tmp);
		if (dst.hasAlpha())
			dst.alpha.get().copyDeep(tmpAlpha);
		return true;
	}
	else
		return false;

}
 */

/*
bool ImageOp::traverseOverlappingWithTemp(const ImageFrame & src, ImageFrame & dst) const {
	drain::Logger mout(getImgLog(), name+"(ImageOp)", __FUNCTION__);
	mout.note() << "checking tmp" << mout.endl;
	if (dst.hasOverlap(src)){
		mout.note() << "overlapping images, tmp image used" << mout.endl;
		Image tmp;
		makeCompatible(src, tmp);
		initializeParameters(src, tmp);
		traverseChannel(src, tmp);
		dst.copyData(tmp);
		return true;
	}
	else {
		return false;
	}
}
*/

/*
bool ImageOp::traverseOverlappingWithTemp(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {

	drain::Logger mout(getImgLog(), name+"(ImageOp)", __FUNCTION__);
	mout.note() << "checking tmp" << mout.endl;

	const bool IMAGE_OVERLAP = dst.hasOverlap(src);
	const bool ALPHA_OVERLAP = dstWeight.hasOverlap(srcWeight);

	if (IMAGE_OVERLAP || ALPHA_OVERLAP){

		mout.note() << "overlapping images, tmp images used" << mout.endl;

		Image tmp;
		if (IMAGE_OVERLAP)
			makeCompatible(src, tmp);
		Channel & dst2  = IMAGE_OVERLAP ? tmp.getChannel(0) : dst;
		initializeParameters(src, dst2);

		Image tmpWeight;
		if (ALPHA_OVERLAP)
			makeCompatible(srcWeight, tmpWeight);
		Channel & dstWeight2 = ALPHA_OVERLAP ? tmpWeight.getChannel(0) : dstWeight;

		traverseChannel(src, srcWeight, dst2, dstWeight2);
		if (IMAGE_OVERLAP)
			dst.copyData(dst2);
		if (ALPHA_OVERLAP)
			dstWeight.copyData(dstWeight2);

		return true;
	}
	else {
		return false;
	}
}
*/





/// Corresponds to processChannelsSeparately
void ImageOp::traverseChannelsSeparately(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	drain::Logger mout(this->name+"(ImageOp::)", __FUNCTION__);

	if (src.empty()){
		mout.error() << "src empty" << mout.endl;
	}

	if (dst.empty()){
		mout.error() << "dst empty" << mout.endl;
	}

	typename ImageTray<const Channel>::map_t::const_iterator sit  = src.begin();
	typename ImageTray<Channel>::map_t::iterator dit = dst.begin();

	initializeParameters(sit->second, dit->second);

	if (src.hasAlpha()){

		typename ImageTray<const Channel>::map_t::const_iterator sait = src.alpha.begin();
		typename ImageTray<Channel>::map_t::iterator dait = dst.alpha.begin();

		while (true){

			mout.debug(1) << "invoking traverseChannel(src, srcAlpha, dst, dstAlpha) for Channel #" << dit->first << mout.endl;

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

			mout.debug(1) << "invoking traverseChannel(src, dst) for Channel #" << dit->first << mout.endl;

			const Channel & srcData  = sit->second;
			Channel & dstData  = dit->second;
			//mout.debug(1) << "dst: " << dstData << mout.endl;
			traverseChannel(srcData, dstData); // apply

			if (++sit == src.end())
				return;

			if (++dit == dst.end()) // re-use dst ?? warning?
				dit = dst.begin();

		}

	}

}

void ImageOp::traverseChannelsEqually(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	drain::Logger mout(this->name+"(ImageOp::)", __FUNCTION__);

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

	typename ImageTray<const Channel>::map_t::const_iterator sit  = src.begin();
	typename ImageTray<Channel>::map_t::iterator dit = dst.begin();
	initializeParameters(sit->second, dit->second);
	while (sit != src.end()){
		mout.debug(1) << "invoke traverseChannel(src, dst) for Channel #" << sit->first << mout.endl;
		traverseChannel(sit->second, dit->second); // apply
		++sit, ++dit;
	}

	typename ImageTray<const Channel>::map_t::const_iterator sait = src.alpha.begin();
	typename ImageTray<Channel>::map_t::iterator dait = dst.alpha.begin();
	while (sait != src.alpha.end()){
		mout.debug(1) << "invoke traverseChannel(srcAlpha, dstAlpha) for Channel #" << dit->first << mout.endl;
		traverseChannel(sait->second, dait->second); // apply
		++sait, ++dait;
	}
}

void ImageOp::traverseChannelsRepeated(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	drain::Logger mout(this->name+"(ImageOp::)", __FUNCTION__);

	if (src.empty()){
		mout.error() << "src empty" << mout.endl;
	}

	if (dst.empty()){
		mout.error() << "dst empty" << mout.endl;
	}

	const size_t n = std::max(src.size(), dst.size());

	typename ImageTray<const Channel>::map_t::const_iterator sit  = src.begin();
	typename ImageTray<Channel>::map_t::iterator dit = dst.begin();

	initializeParameters(sit->second, dit->second);


	if (src.hasAlpha()){

		typename ImageTray<const Channel>::map_t::const_iterator sait = src.alpha.begin();
		typename ImageTray<Channel>::map_t::iterator dait = dst.alpha.begin();

		//while (true){
		for (size_t i = 0; i < n; ++i) {

			mout.debug(1) << "invoke traverseChannel(src, srcAlpha, dst, dstAlpha) for Channel #" << dit->first << mout.endl;

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

			mout.debug(1) << "invoking traverseChannel(src, dst) for Channel #" << dit->first << mout.endl;

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

