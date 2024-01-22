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

#include <drain/image/ImageFile.h>
#include <math.h>

// #include "drain/util/Log.h"
#include "DifferentialOp.h"


namespace drain
{

namespace image
{


void DifferentialOp::getDstConf(const ImageConf & src, ImageConf & dst) const {  // TODO: src, src2, dst

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	dst.setEncoding(src);
	dst.setArea(src.getGeometry()); // EROR?
	dst.setChannelCount(channels, 0);
	//dst.initialize(src.getType(), src.getWidth(), src.getHeight(), channels, 0);
	//dst.setScaling(src.getScaling());

	// For unsigned small ints (uchar, ushort), move origin
	const std::type_info & t = dst.getType();
	if (drain::Type::call<drain::typeIsSmallInt>(t) && (!Type::call<drain::isSigned>(t)) && (dst.getScaling().getOffset()==0.0)){
		const double s = dst.getScaling().getScale()*2.0;
		mout.note() << "Unsigned small integer type (with offset=0.0), changing scaling from " << dst.getScaling();
		dst.setScaling(s, -s * Type::call<typeMax,double>(t)/2.0);
		mout << " to " << dst.getScaling() << mout.endl;
		//dst.setOptimalScale();
	}
}

/*
void DifferentialOp::makeCompatible(const ImageFrame & src, Image & dst) const {
	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	dst.initialize(src.getType(), src.getWidth(), src.getHeight(), channels, 0);
	dst.setScaling(src.getScaling());

	// For unsigned small ints (uchar, ushort), move origin
	const std::type_info & t = dst.getType();
	if (!Type::call<drain::isSigned>(t) && drain::Type::call<drain::typeIsSmallInt>(t) && (dst.getScaling().getOffset()==0.0)){
		const double s = dst.getScaling().getScale()*2.0;
		mout.note() << "Unsigned small integer type (with offset=0.0), changing scaling from " << dst.getScaling();
		dst.setScaling(s, -s * Type::call<typeMax,double>(t)/2.0);
		mout << " to " << dst.getScaling() << mout.endl;
		//dst.setOptimalScale();
	}

}
*/

void DifferentialOp::traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), name+"(DifferentialOp)", __FUNCTION__);

	if (src.empty()){
		mout.error("src empty" );
	}

	if (dst.size() < channels){
		mout.error("dst size(" , dst.size() , ") incompatible with required channel count " , channels );
	}

	if (channels == 1){
		mout.debug("1D (single-direction) operator" );
		traverseChannel(src.get(), dst.get());
	}
	else {
		mout.debug("2D (combined horz & vert) operator" );
		traverseHorz(src.get(), dst.get(0));
		traverseVert(src.get(), dst.get(1));
	}

}




//void GradientOp::traverse(const Channel &src, Channel &dst, int diLow, int djLow, int diHigh, int djHigh) const {
void GradientOp::traverse(const Channel &src, Channel &dst, int di, int dj) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), name+"(GradientOp)", __FUNCTION__);

	/// Type for internal computation (~image intensity).
	typedef double ftype;

	const int width  = src.getWidth();
	const int height = src.getHeight();

	CoordinateHandler2D coordinateHandler(width, height, src.getCoordinatePolicy());

	if ((di==0) && (dj==0))
		mout.error("zero span" );

	/*
	const ftype scaleFinal = scale / spanFinal;
	mout.warn(*this );
	mout.warn("  -- spanFinal=" , spanFinal , " scaleFinal=" , scaleFinal );
	mout.warn("  -- coordHandler" , coordinateHandler );
	dst.dump();
	mout.warn("  -- dst: (see above) " , dst );
	 */
	const bool SCALE = src.getScaling().isScaled() || dst.getScaling().isScaled();
	const ftype spanCoeff = 0.5/static_cast<ftype>(abs(di)+abs(dj));

	mout.debug(this->getParameters() );
	mout.debug2("spanCoeff:" , spanCoeff , ", SCALE:" , (int)SCALE );

	Point2D<int> pLo;
	Point2D<int> pHi;

	if (!this->LIMIT){

		mout.debug("LIMIT=false" );

		if (!SCALE){
			mout.debug("SCALE=false" );
			for (int j = 0; j < height; j++) {
				for (int i = 0; i < width; i++) {
					pLo.setLocation(i-di,j-dj);
					coordinateHandler.handle(pLo);
					pHi.setLocation(i+di,j+dj);
					coordinateHandler.handle(pHi);
					//dst.put(i,j, bias + scale*(src.get<ftype>(pHi) - src.get<ftype>(pLo)));
					dst.put(i,j, spanCoeff*(src.get<ftype>(pHi) - src.get<ftype>(pLo)));  // TODO: span?
				}
			}
		}
		else {
			mout.debug("SCALE=true" );
			for (int j = 0; j < height; j++) {
				for (int i = 0; i < width; i++) {
					pLo.setLocation(i-di,j-dj);
					coordinateHandler.handle(pLo);
					pHi.setLocation(i+di,j+dj);
					coordinateHandler.handle(pHi);
					//dst.put(i,j, dst.scaling.inv(bias + scale*(src.scaling.fwd(src.get<ftype>(pHi)) - src.scaling.fwd(src.get<ftype>(pLo)))));
					//dst.put(i,j, dst.scaling.inv(spanCoeff*(src.scaling.fwd(src.get<ftype>(pHi)) - src.scaling.fwd(src.get<ftype>(pLo)))));
					dst.putScaled(i,j, spanCoeff*(src.getScaled(pHi.x, pHi.y) - src.getScaled(pLo.x, pLo.y)));
				}
			}

		}

	}
	else {

		mout.debug("LIMIT=true" );

		drain::typeLimiter<ftype>::value_t limit = dst.getConf().getLimiter<ftype>();
		if (!SCALE){
			mout.debug("SCALE=false" );
			for (int j = 0; j < height; j++) {
				for (int i = 0; i < width; i++) {
					pLo.setLocation(i-di,j-dj);
					coordinateHandler.handle(pLo);
					pHi.setLocation(i+di,j+dj);
					coordinateHandler.handle(pHi);
					// dst.put(i,j, limit(bias + scale*(src.get<ftype>(pHi) - src.get<ftype>(pLo))));
					dst.put(i,j, limit(spanCoeff*(src.get<ftype>(pHi) - src.get<ftype>(pLo))));
				}
			}
		}
		else {
			mout.debug("SCALE=true" );
			for (int j = 0; j < height; j++) {
				for (int i = 0; i < width; i++) {
					pLo.setLocation(i-di,j-dj);
					coordinateHandler.handle(pLo);
					pHi.setLocation(i+di,j+dj);
					coordinateHandler.handle(pHi);
					// dst.put(i,j, limit(dst.scaling.inv(bias + scale*(src.scaling.fwd(src.get<ftype>(pHi)) - src.scaling.fwd(src.get<ftype>(pLo))))));
					dst.put(i,j, limit(dst.getScaling().inv(spanCoeff*(src.getScaling().fwd(src.get<ftype>(pHi)) - src.getScaling().fwd(src.get<ftype>(pLo))))));
				}
			}
		}

	}

	//drain::image::File::write(dst, "Mika.png");
	// mout.warn("finished\n" );

}




//, int diLow, int djLow, int diHigh, int djHigh) const {
void LaplaceOp::traverse(const Channel &src, Channel &dst, int di, int dj) const {

	Logger mout(getImgLog(),__FILE__, __FUNCTION__);

	/// Type for internal compuation (~image intensity).
	typedef float ftype;

	//makeCompatible(src,dst);
	//coordinateHandler.applyImage(src);

	const int width  = src.getWidth();
	const int height = src.getHeight();

	CoordinateHandler2D coordinateHandler(width, height, src.getCoordinatePolicy());

	/*
	const int iSpan = diLow+diHigh;
	const int jSpan = djLow+djHigh;
	const ftype span = sqrt(iSpan*iSpan + jSpan*jSpan);
	const ftype spanFinal = sqrt(di*di + dj*dj);
	if (spanFinal == 0.0)
	*/
	if ((di==0) && (dj==0))
		mout.error("zero span" );

	if ((di!=0) && (dj!=0))
		mout.error("mixed span" );

	const ftype sign = ((di>0)  || (dj>0)) ? +1.0 : -1.0;

	/*
	const ftype spanCoeff = 1.0f / (2.0f * static_cast<ftype>(di + dj));
	const ftype preCoeff = (spanCoeff>0.0) ? 2.0 : -2.0;
	mout.debug(-2) << " preCoeff=" << preCoeff << " spanCoeff=" << spanCoeff  << mout.endl;
	*/

	//const ftype scale = getParameter("scale",ftype(1)) / span;
	mout .debug3() << getParameters() << mout.endl;
	//mout .debug3() << " bias=" << bias << " scale=" << scale << mout.endl;

	drain::typeLimiter<double>::value_t limit = dst.getConf().getLimiter<double>(); // Type::call<Limiter>(dst.getType());

	Point2D<int> pLo;
	Point2D<int> pHi;
	//const CoordinateHandler & h = src.getCoordinateHandler();
	//ftype result;
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			pLo.setLocation(i-di,j-dj);
			coordinateHandler.handle(pLo);
			pHi.setLocation(i+di,j+dj);
			coordinateHandler.handle(pHi);
			//result = dst.scaling.limit<ftype>(bias + scale*(-src.get<ftype>(pHi) + 2*src.get<ftype>(i,j) - src.get<ftype>(pLo)));
			//result = limit(preCoeff*src.get<ftype>(i,j) - spanCoeff*(src.get<ftype>(pLo) + src.get<ftype>(pHi)));
			//dst.put(i,j, limit(preCoeff*src.get<ftype>(i,j) - spanCoeff*(src.get<ftype>(pLo) + src.get<ftype>(pHi))));
			dst.put(i,j, limit(sign*( -src.get<ftype>(pLo) + 2.0f * src.get<ftype>(i,j) - src.get<ftype>(pHi))));
		}
	}
}


}
}


// Drain
