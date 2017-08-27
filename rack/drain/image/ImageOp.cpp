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



namespace drain
{

namespace image
{

void ImageOp::help(std::ostream &ostr) const {
	ostr << name << ": " << description << '\n';
	/*
	ostr << '\t';
	if (WEIGHT_SUPPORTED){
		ostr << "weighted";
		if (WEIGHT_SUPPORTED == 2)
			ostr << " (only)";
		ostr << ',';
	}
	if (IN_PLACE)
		ostr << "in-place,";
	if (MULTICHANNEL)
		ostr << "multichannel";
		*/
	//ostr << '\n';
	ostr << '\t' << parameters << '\n';

}


void ImageOp::makeCompatible(const Image & src, Image & dst) const  {

	drain::MonitorSource mout(iMonitor, name+"(ImageOp)", __FUNCTION__);

	mout.debug(2) << "src: " << src << mout.endl;

	if (dst.isSame(src)){
		mout.debug() << "dst == src, ok" << mout.endl;
		return;
	}

	if (!dst.typeIsSet()){
		dst.setType(src.getType());
		// dst.setScale(src.getScale()); // NEW
	}


	if (!Type::isIntegralType(dst.getType())){
		dst.setScale(src.getScale());
		if (dst.getType() != src.getType()){
			mout.info() << "Adopting src scaling for dst (different storage types)" << mout.endl;
		}
	}

	dst.setGeometry(src.getGeometry());
	/// TODO: copy alpha, fill alpha?
	if (dst.getAlphaChannelCount()){
		mout.info() << "resetting alpha channel" << mout.endl;
		dst.getAlphaChannel().fill(dst.getMax<int>());
	}

	dst.setCoordinatePolicy(src.getCoordinatePolicy());

	mout.debug(3) << "dst: " << dst << mout.endl;

}


void ImageOp::makeCompatible(const Image & src1, const Image & src2, Image & dst) const  {

	drain::MonitorSource mout(iMonitor, name+"(ImageOp)", __FUNCTION__);

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




void ImageOp::filter(const Image &src, Image &dst) const {

	MonitorSource mout(iMonitor, name+"(ImageOp)[1]", __FUNCTION__);

	mout.debug(1) << "start" << mout.endl;

	/// If alphas exist and are supported by the operator, delegate to filter(src, srcAlpha, dst, dstAlpha);
	if (filterUsingAlphas(src,dst))  // see further below
		return;

	mout.debug(1) << "next: filterWithTmp" << mout.endl;
	/// If a separate dst is needed, create it and reinvoke this.
	if (filterWithTmp(src, dst))
		return;

	mout.debug(1) << "next: makeCompatible" << mout.endl;
	makeCompatible(src,dst);
	// makeCompatible(srcAlpha,dstAlpha);

	mout.debug(1) << "next: initializeParameters" << mout.endl;
	initializeParameters(src, dst);

	mout.debug(1) << "next: traverseChannelsSeparately" << mout.endl;
	/// If the operation can be computed independently on each channel, split to channels and reinvoke this.
	if (traverseChannelsSeparately(src,dst))
		return;

	//mout.debug(1) << "next: makeCompatible" << mout.endl;
	// mout.warn()
	//makeCompatible(src, dst);


	/// Call local (class-implemented) function
	// mout.warn() << "next: traverse" << mout.endl;
	mout.debug(1) << "finally: traverse" << mout.endl;
	traverse(src, dst);

}

// Caution - lazy implementation.
void ImageOp::filter(const Image & src1, const Image & src2, Image &dst) const {

	MonitorSource mout(iMonitor, name+"(ImageOp)[2]", __FUNCTION__);

	mout.debug(1) << "start" << mout.endl;

	/// If alphas exist and are supported by the operator, delegate to filter(src,srcAlpha,dst,dstAlpha);
	//  if (filterUsingAlphas(src,dst))
	//	  return;

	/// If overlapping, create a copy of dst and reinvoke this function.
	if (filterWithTmp(src1, src2, dst))
		return;

	/// If the operation can be computed independently on each channel, split to channels and reinvoke this.
	if (traverseChannelsSeparately(src1, src2, dst))
		return;

	makeCompatible(src1, src2, dst);
	initializeParameters(src1, dst);

	/// Call local (class-implemented) function
	traverse(src1, src2, dst);

}


void ImageOp::filter(const Image &src, const Image &srcWeight, Image &dst, Image &dstWeight) const {


	/// If overlapping, create copies of dst and dstWeight and reinvoke this.
	if (filterWithTmp(src, srcWeight, dst, dstWeight))
		return;

	/// If the operation can be computed independently on each channel, split to channels and reinvoke this.
	if (traverseChannelsSeparately(src, srcWeight, dst, dstWeight))
		return;

	makeCompatible(src,dst);
	makeCompatible(srcWeight,dstWeight);

	filterCompatible(src, srcWeight, dst, dstWeight);
	/*
	initializeParameters(src, dst); // what about alpha channel scale etc?

	/// Call local (class-implemented) function
	traverse(src,srcWeight,dst,dstWeight);
	*/

}


bool ImageOp::filterUsingAlphas(const Image & src, Image & dst) const {

	//if (!WEIGHT_SUPPORTED)
	//	return false;

	//std::cerr << "ImageOp::filterUsingAlphas 1 \n";

	const size_t iSrc = src.getImageChannelCount();
	const size_t aSrc = src.getAlphaChannelCount();

	if (aSrc == 0)
		return false;

	//std::cerr << "ImageOp::filterUsingAlphas 2 \n";

	MonitorSource mout(iMonitor, name + "(ImageOp)", __FUNCTION__);
	mout.debug(2) << mout.endl;

	const Image srcImage(src,0,    iSrc);
	const Image srcAlpha(src,iSrc, aSrc);

	makeCompatible(src,dst);
	const size_t iDst = dst.getImageChannelCount();
	const size_t aDst = dst.getAlphaChannelCount();
	Image dstImage(dst,0,    iDst);
	Image dstAlpha(dst,iDst, aDst);

	mout.debug(5) << "srcImage" << srcImage << mout.endl;
	mout.debug(5) << "srcAlpha" << srcAlpha << mout.endl;
	mout.debug(5) << "dstImage" << dstImage << mout.endl;
	mout.debug(5) << "dstAlpha" << dstAlpha << mout.endl;

	//filter(srcImage, srcAlpha, dstImage, dstAlpha);
	//traverse(srcImage, srcAlpha, dstImage, dstAlpha);
	//filter(srcImage, srcAlpha, dstImage, dstAlpha);
	filterCompatible(srcImage, srcAlpha, dstImage, dstAlpha);
	//traverse(srcImage, srcAlpha, dstImage, dstAlpha); // makeComp called!
	return true;
}


bool ImageOp::filterWithTmp(const Image & src,Image & dst) const {

	//if (src.hasOverlap(dst)){
	if (src.isSame(dst)){

		MonitorSource mout(iMonitor, name + "(ImageOp)", __FUNCTION__);
		mout.debug(2) << mout.endl;

		Image tmp(dst);
		filter(src,tmp);

		/// Copy
		//  Here, cannot use a derived class :-) like CopyOp().filter(tmp,dst)
		dst.setGeometry(tmp.getGeometry());  // yes, ensure, because dst may be empty initially
		Image::const_iterator si = tmp.begin();
		Image::iterator di = dst.begin();
		Image::const_iterator dEnd = dst.end();
		while (di!=dEnd){
			*di = *si;
			++si;
			++di;
		}
		return true;
	}
	else
		return false;
}

bool ImageOp::filterWithTmp(const Image & src, const Image & src2, Image & dst) const {

	MonitorSource mout(iMonitor, name + "(ImageOp)", __FUNCTION__);
	mout.debug(2) << mout.endl;

	if (src.hasOverlap(dst) || src2.hasOverlap(dst)){

		mout.debug() << "TMP needed, ok" << mout.endl;

		Image tmp;
		filter(src, src2, tmp);

		/// Copy
		// TODO: WHY NOT dst.copyDeep(tmp);
		//  Cannot use derived class :-) CopyOp().filter(tmp,dst)
		dst.setGeometry(tmp.getGeometry());  // yes, ensurem  because dst may be empty initially
		Image::const_iterator si = tmp.begin();
		Image::iterator di = dst.begin();
		Image::const_iterator dEnd = dst.end();
		while (di!=dEnd){
			*di = *si;
			++si;
			++di;
		}
		// mout.warn() << "ready" << mout.endl;

		return true;
	}
	else {
		mout.debug() << "TMP not needed" << mout.endl;
		return false;
	}

}


bool ImageOp::filterWithTmp(const Image & src, const Image & srcAlpha, Image & dst, Image & dstAlpha) const {

	//if (IN_PLACE)
	//	return false;

	const bool DATA_OVERLAP  = src.hasOverlap(dst);
	const bool ALPHA_OVERLAP = srcAlpha.hasOverlap(dstAlpha);

	if ( DATA_OVERLAP || ALPHA_OVERLAP ){

		Image tmp;
		if (DATA_OVERLAP){
			tmp.setType(dst.getType());
			tmp.setGeometry(dst.getGeometry());
		}

		Image tmpAlpha;
		if (ALPHA_OVERLAP){
			tmpAlpha.setType(dstAlpha.getType());
			tmpAlpha.setGeometry(dstAlpha.getGeometry());
		}

		Image & d = DATA_OVERLAP  ? tmp      : dst;
		Image & a = ALPHA_OVERLAP ? tmpAlpha : dstAlpha;

		filter(src, srcAlpha, d, a);

		/// Copy
		//  (Cannot use *derived* class CopyOp().filter(tmp,dst)! )
		if (DATA_OVERLAP)
			dst.copyDeep(tmp);

		if (ALPHA_OVERLAP)
			dstAlpha.copyDeep(tmpAlpha);

		return true;
	}
	else
		return false;

}



bool ImageOp::traverseChannelsSeparately(const Image & src, Image & dst) const {

	MonitorSource mout(iMonitor, name+"(ImageOp)", __FUNCTION__);

	//if (MULTICHANNEL)
	//	return false;

	const size_t imageChannels = src.getImageChannelCount();
	const size_t alphaChannels = src.getAlphaChannelCount();

	if (alphaChannels){
		mout.note() << "skipping alpha channels (" << alphaChannels << ')' << mout.endl;
	}

	const size_t channels = imageChannels ? imageChannels : alphaChannels; //  ie. alpha channel count

	if (channels == 1){
		/// makeCompatible(src,dst); NEW
		return false;
	}
	else {
		mout.debug(2) << mout.endl;

		makeCompatible(src,dst); // This must be here.
		const size_t channelsDst = dst.getChannelCount();
		for (size_t i = 0; i < channels; ++i)
			//traverse(src.getChannel(i), dst.getChannel(i%channelsDst));
			filter(src.getChannel(i), dst.getChannel(i%channelsDst));
		return true;
	}

}

bool ImageOp::traverseChannelsSeparately(const Image & src1, const Image & src2, Image & dst) const {

	MonitorSource mout(iMonitor, name+"(ImageOp)", __FUNCTION__);

	//if (MULTICHANNEL)
	//	return false;

	//const size_t imageChannels1 = src1.getImageChannelCount();
	const size_t alphaChannels1 = src1.getAlphaChannelCount();
	//const size_t imageChannels2 = src2.getImageChannelCount();
	const size_t alphaChannels2 = src2.getAlphaChannelCount();
	if ((alphaChannels1>0) && (alphaChannels2>0)){
		mout.note() << "skipping alpha channels (" << alphaChannels1 << ',' << alphaChannels2 << ')' << mout.endl;
	}


	// const size_t channels = std::max(src1.getImageChannelCount(), src2.getImageChannelCount());  // ? imageChannels : alphaChannels; //  ie. alpha channel count
	makeCompatible(src1, src2, dst);
	const size_t imageChannelsDst = dst.getImageChannelCount();

	//const size_t channels = std::max(src1.getImageChannelCount(), src2.getImageChannelCount());

	if (imageChannelsDst == 1){
		return false;
	}
	else {
		makeCompatible(src1, src2, dst);
		initializeParameters(src1, dst);

		const size_t imageChannels1 = src1.getImageChannelCount();
		const size_t imageChannels2 = src2.getImageChannelCount();
		// mout.debug(2) << mout.endl;
		//makeCompatible(src1, dst); // This must be here.
		//const size_t channelsDst = dst.getChannelCount();
		for (size_t k = 0; k < imageChannelsDst; ++k)
			traverse(src1.getChannel(k % imageChannels1), src2.getChannel(k % imageChannels2), dst.getChannel(k % imageChannelsDst));
		return true;
	}

}



void ImageOp::filterCompatible(const Image & src, const Image & srcAlpha, Image & dst, Image & dstAlpha) const {

		if (traverseChannelsSeparately(src, srcAlpha, dst, dstAlpha))
			return;

		initializeParameters(src, dst);
		traverse(src, srcAlpha, dst, dstAlpha);

}




bool ImageOp::traverseChannelsSeparately(const Image & src, const Image & srcAlpha, Image & dst, Image & dstAlpha) const {

	MonitorSource mout(iMonitor, name+"(ImageOp)", __FUNCTION__);

	mout.debug(1) << mout.endl;


	//const size_t channels = src.getChannelCount();
	const size_t channels = src.getImageChannelCount();
	if (channels == 1){
		return false;
	}
	else {

		mout.debug(2) << mout.endl;

		// makeCompatible(src,dst); // This must be here.
		//makeCompatible(srcAlpha,dstAlpha); // This must be here.

		const size_t channelsSrcAlpha = srcAlpha.getChannelCount();
		const size_t channelsDst      = dst.getImageChannelCount();
		const size_t channelsDstAlpha = dstAlpha.getChannelCount();
		for (size_t k = 0; k < channels; ++k){
			mout.warn() << "calling for channel=" << k << mout.endl;
			filterCompatible(src.getChannel(k), srcAlpha.getChannel(k % channelsSrcAlpha), dst.getChannel(k % channelsDst), dstAlpha.getChannel(k % channelsDstAlpha));
		}
		return true;
	}

}

/*
void ImageOp::setView(Image & src, const std::string & view, Image & dstView) {

	const char v = view.empty() ? 'F' : view.at(0);

	switch (v) {
	case 'F':
		dstView.setView(src, 0, src.getChannelCount(), true);
		break;
	case 'f':
		dstView.setView(src);
		break;
	case 'i':  // image channels (excluding alpha)
		dstView.setView(src, 0, src.getImageChannelCount());
		break;
	case 'a':
		dstView.setView(src.getAlphaChannel());
		break;
	default:
		std::stringstream sstr(view);
		size_t i;
		sstr >> i;
		dstView.setView(src, i, 1);
		break;
	}
}
*/

}
}


// Drain
