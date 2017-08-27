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

#include <math.h>

// #include "util/Debug.h"
#include "GradientOp.h"


namespace drain
{

namespace image
{



void GradientOp::traverse(const Image &src, Image &dst, int diLow, int djLow, int diHigh, int djHigh) const {

	MonitorSource mout(iMonitor, name+"(GradientOp)", __FUNCTION__);

	/// Type for internal computation (~image intensity).
	typedef float ftype;

	//makeCompatible(src,dst);

	const int width  = src.getWidth();
	const int height = src.getHeight();

	CoordinateHandler2D coordinateHandler(width, height, src.getCoordinatePolicy());

	const int iSpan = diLow+diHigh;
	const int jSpan = djLow+djHigh;
	const ftype spanFinal = sqrt(iSpan*iSpan + jSpan*jSpan);

	if (spanFinal == 0.0)
		throw (std::runtime_error("GradientOp: zero span"));



	/*
	const ftype scaleFinal = scale / spanFinal;
	mout.warn() << *this << mout.endl;
	mout.warn() << "  -- spanFinal=" << spanFinal << " scaleFinal=" << scaleFinal << mout.endl;
	mout.warn() << "  -- coordHandler" << coordinateHandler << mout.endl;
	dst.dump();
	mout.warn() << "  -- dst: (see above) " << dst << mout.endl;
	 */

	Point2D<int> pLo;
	Point2D<int> pHi;

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			pLo.setLocation(i-diLow,j-djLow);
			coordinateHandler.handle(pLo);
			pHi.setLocation(i+diHigh,j+djHigh);
			coordinateHandler.handle(pHi);
			// if ((i==j)&&((i&15)==0)) 	std::cerr << '\t' << pHi << '\t' << pLo << std::endl;
			//	dst.put(i,j, dst.limit<ftype>(bias + scale*(src.get<ftype>(pHi) - src.get<ftype>(pLo))));
			dst.put(i,j, bias + scale*(src.get<ftype>(pHi) - src.get<ftype>(pLo)));
		}
	}

	// mout.warn() << "finished\n" << mout.endl;

}



void GradientHorizontalOp::traverse(const Image & src, Image &dst) const {
	const int span = getParameter("span",2);
	if (span>0)
		GradientOp::traverse(src, dst, span/2, 0, (span+1)/2, 0);
	else
		GradientOp::traverse(src, dst, span/2, 0, (span-1)/2, 0);
}


void GradientVerticalOp::traverse(const Image &src,Image &dst) const {
	const int span = this->getParameter("span",2);
	if (span>0)
		GradientOp::traverse(src, dst, 0, span/2, 0, (span+1)/2);
	else
		GradientOp::traverse(src, dst, 0, span/2, 0, (span-1)/2);
}





void LaplaceOp::traverse(const Image &src,Image &dst, int diLow, int djLow, int diHigh, int djHigh) const {

	MonitorSource mout(iMonitor,__FILE__, __FUNCTION__);

	/// Type for internal compuation (~image intensity).
	typedef float ftype;

	makeCompatible(src,dst);
	//coordinateHandler.applyImage(src);

	const int width  = src.getWidth();
	const int height = src.getHeight();

	CoordinateHandler2D coordinateHandler(width, height, src.getCoordinatePolicy());
	//
	//const float bias  = getParameter("bias",0.0);

	const int iSpan = diLow+diHigh;
	const int jSpan = djLow+djHigh;
	const ftype span = sqrt(iSpan*iSpan + jSpan*jSpan);

	if (span == 0.0)
		throw (std::runtime_error("GradientOp: zero span"));

	//const ftype scale = getParameter("scale",ftype(1)) / span;
	mout.debug(3) << " bias=" << bias << " scale=" << scale << mout.endl;

	Point2D<int> pLo;
	Point2D<int> pHi;
	//const CoordinateHandler & h = src.getCoordinateHandler();
	ftype result;
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			pLo.setLocation(i-diLow,j-djLow);
			coordinateHandler.handle(pLo.x,pLo.y);
			pHi.setLocation(i+diHigh,j+djHigh);
			coordinateHandler.handle(pHi.x,pHi.y);
			result = dst.limit<ftype>(bias + scale*(-src.get<ftype>(pHi) + 2*src.get<ftype>(i,j) - src.get<ftype>(pLo)));
			dst.put(i,j, result);
		}
	}
}


void LaplaceHorizontalOp::traverse(const Image & src, Image &dst) const {
	//const int span = getParameter("span",2);
	if (span>0)
		LaplaceOp::traverse(src, dst, span/2, 0, (span+1)/2, 0);
	else
		LaplaceOp::traverse(src, dst, span/2, 0, (span-1)/2, 0);
}


void LaplaceVerticalOp::traverse(const Image &src, Image &dst) const {
	//const int span = getParameter("span",2);
	if (span>0)
		LaplaceOp::traverse(src, dst, 0, span/2, 0, (span+1)/2);
	else
		LaplaceOp::traverse(src, dst, 0, span/2, 0, (span-1)/2);
}





}
}


// Drain
