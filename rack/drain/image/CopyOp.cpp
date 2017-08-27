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
#include "util/Debug.h"
#include "CopyOp.h"

namespace drain {

namespace image {


void CopyOp_OLD::filter(const Image & src, Image & dst) const {

	drain::MonitorSource mout(iMonitor,"CopyOp::filter");

	Image srcV;

	//const std::string &s = getParameter("srcView","f");
	/*
	mout.note(4) << parameters << mout.endl;
	mout.note(5) << "src: " << src << mout.endl;
	mout.note(5) << "dst: " << dst << mout.endl;
	 */

	const size_t width  = src.getWidth();
	const size_t height = src.getHeight();
	const size_t iSrc = src.getImageChannelCount();
	const size_t aSrc = src.getAlphaChannelCount();

	if (srcView == "f"){
		srcV.setView(src);
	}
	else if (srcView == "i"){
		srcV.setView(src,0, iSrc);
	}
	else if (srcView == "a"){
		srcV.setView(src,iSrc, aSrc);
	}
	else {
		srcV.setView(src, (size_t) getParameter("srcView"));
	}

	mout.debug(3) << "srcV: " << srcV << mout.endl;

		/*
	switch (srcView[0]) {
	case 'f': // full image
		srcV.setView(src);
		break;
	case 'i':  // image channels (ie. excluding alpha)
		srcV.setView(src, 0, src.getImageChannelCount());
		break;
	case 'a': // alpha channel(s)
		srcV.setView(src, src.getImageChannelCount(), src.getAlphaChannelCount());
		break;
	default:
		srcV.setView(src, (size_t) getParameter("srcView"));
	}
		 */

	const std::string &t = getParameter("dstType");
	if ((t=="dst")||(t=="*")){
		if (!dst.typeIsSet())
			dst.setType(src.getType());
	}
	else if (t=="src"){
		dst.setType(src.getType());
		dst.setLimits(src.getMin<double>(),src.getMax<double>());
	}
	else
		dst.setType(t.at(0));
	/*
	else if (t=="8")
		dst.setType<unsigned char>();
	else if (t=="16")
		dst.setType<unsigned short>();
	else
		throw std::runtime_error(t + " <= CopyOp: illegal target type.");
	 */

	Image dstV;
	//const Data &d = getParameter("dstView","0");

	const size_t iSrcV  = srcV.getImageChannelCount();
	const size_t aSrcV  = srcV.getAlphaChannelCount();


	if (dstView == "f"){
		dst.setGeometry(srcV.getGeometry());
		dstV.setView(dst);
	}
	else if (dstView == "i"){
		dst.setGeometry(width,height,iSrcV + aSrcV);
		dstV.setView(dst,0,dst.getImageChannelCount());
	}
	else if (dstView == "a"){
		const size_t iDst  = dst.getImageChannelCount();
		dst.setGeometry(width, height, iDst, iSrcV + aSrcV);
		dstV.setView(dst, iDst, iSrcV + aSrcV);
	}
	else {
		// Start channel
		const size_t k  = (size_t)getParameter("dstView");
		// Channel count needed
		const size_t k2 = k + srcV.getChannelCount();

		if (k2 > dst.getChannelCount())
			dst.setGeometry(width, height, k2);
		else
			dst.setGeometry(width, height, dst.getImageChannelCount(),dst.getAlphaChannelCount());

		dstV.setView(dst,k, srcV.getChannelCount());
	}

	mout.debug(3) << "dst: " << dst << mout.endl;
	mout.debug(3) << "dstV:" << dstV << mout.endl;

	mout.debug(3) << "dst iters: " << dst.begin() <<  '-' << dst.end() << mout.endl;
	mout.debug(3) << "dstV iters:" << dstV.begin() << '-' << dstV.end() << mout.endl;
	Image::iterator di=dstV.begin();
	Image::iterator die=dstV.end();
	mout.debug(3) << "dstV iters:" << di << '-' << die << mout.endl;

	mout.debug(3) << "dst   byteSize:" << dst.begin().getByteSize() << mout.endl;
	mout.debug(3) << "dstV  byteSize:" << dstV.begin().getByteSize() << mout.endl;
	mout.debug(3) << "dstVb byteSize:" << di.getByteSize() << mout.endl;


	const double scale = dst.getMax<double>()/src.getMax<double>();
	mout.debug(3) << "scale: " << scale << mout.endl;

	Image::const_iterator si = srcV.begin();
	//int i=0;
	for (Image::iterator di=dstV.begin(); di!=dstV.end(); ++di,++si){
	//for (Image::iterator di=dst.begin(); di!=dst.end(); ++di,++si){
		//*si = 5;
		//std::cout << (long) di << ' ' << (long)di - (long)dstV.begin() << ' ' << i << '\n';
		*di = dst.limit<double>(scale * static_cast<double>(*si));
		//++i;
	}

}

}  // namespace image

}  // namespace drain

// Drain
