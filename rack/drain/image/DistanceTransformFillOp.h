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
#ifndef DISTANCETRANSFORMFILLOP_H_
#define DISTANCETRANSFORMFILLOP_H_

#include "File.h" // debugging
#include "DistanceTransformOp.h"

#include "QualityMixerOp.h"


namespace drain
{

namespace image
{

/// Fills image with
/**

\~exec
drainage --geometry 256,256,3,1 --plot 96,96,255,64,32,255  --plot 160,96,64,255,32,208  --plot 128,160,64,32,255,192  -o dots-rgba.png
\~


  \tparam T - DistanceModel
 */
template <class T>
class DistanceTransformFillOp : public DistanceTransformOp<T>
{
public:

	~DistanceTransformFillOp(){};

	void filter(const Image &src, Image &dest) const;

	void filter(const Image &src, const Image &srcAlpha, Image &dst,	Image &dest) const;

	//double horse;

protected:

	DistanceTransformFillOp(const std::string &name, const std::string &description, double horz = 10.0, double vert = -1.0) :
		DistanceTransformOp<T>(name, description, horz, vert) {
	};

	/*
	inline
	virtual void makeCompatible(const Image &src, Image &dst) const  {
		dst.initialize(src.getType(),src.getGeometry());
	};

	inline
	virtual void makeCompatible(const Image &src, const Image &srcAlpha, Image &dst, Image &dstAlpha) const  {
		dst.initialize(src.getType(), src.getGeometry());
		dstAlpha.initialize(srcAlpha.getType(), srcAlpha.getGeometry()); // NEW
		//dst.setType(src.getType());
		//dst.setGeometry(src.getGeometry());
	};
	 */

	void traverseDownRight(const Image &src,  const Image &srcAlpha,
			Image &dst, Image &destAlpha) const;

	void traverseUpLeft(const Image &src,  const Image &srcAlpha,
			Image &dst, Image &destAlpha) const ;

	//mutable DistanceModelLinear distanceLinear;

};

/**
 *
 *    \tparam T - DistanceModel
 */
template <class T>
void DistanceTransformFillOp<T>::filter(const Image &src, Image &dst) const {

	drain::MonitorSource mout(drain::image::iMonitor,"DistanceTransformFillOp::filter");

	//mout.note(3) << getParameters() << mout.endl;

	this->makeCompatible(src,dst);
	this->initializeParameters(src,dst); // NEW 2013

	const unsigned int iCh = src.getImageChannelCount();

	const unsigned int aCh = src.getAlphaChannelCount();
	if (aCh == iCh){
		for (unsigned int i = 0; i < iCh; ++i)
			filter(src.getChannel(i), src.getChannel(iCh+i), dst.getChannel(i), dst.getChannel(iCh+i));
	}
	else {
		if (aCh == 0)
			mout.error() << "No alphaChannels" << mout.endl;
		const Image srcView(src,0,iCh);
		Image dstView(dst,0,iCh);
		filter(srcView, src.getAlphaChannel(), dstView, dst.getAlphaChannel());
	}
}


/**
 *
 *    \tparam T - DistanceModel
 */
template <class T>
void DistanceTransformFillOp<T>::filter(const Image &src, const Image &srcAlpha, Image &dst, Image &dstAlpha) const {

	drain::MonitorSource mout(this->name, __FUNCTION__);

	// TODO
	//if (filterWithTmp(src,srcAlpha,dst,dstAlpha))
	// 	return;
	//dst.name = "DTF target";

	this->makeCompatible(src,dst);
	this->makeCompatible(srcAlpha,dstAlpha);

	//_initializeParameters(src,dst);
	this->initializeParameters(srcAlpha,dstAlpha);

	//if (true){

		if (this->distanceModel.KNIGHT){
			// mout.warn() << "with chess knight 2+1 distance " << mout.endl;
			//mout.warn() << this->distanceModel.getParameters() << mout.endl;
		}

		//mout.warn() << "no blend" << mout.endl;

		traverseDownRight(src, srcAlpha, dst, dstAlpha);
		if (iMonitor.getVerbosity() > 10){
			drain::image::File::write(dst,"dtd-i.png");
			drain::image::File::write(dstAlpha,"dtd-a.png");
		}

		traverseUpLeft(dst, dstAlpha, dst, dstAlpha);
		if (iMonitor.getVerbosity() > 10){
			drain::image::File::write(dst,"dtu-i.png");
			drain::image::File::write(dstAlpha,"dtu-a.png");
		}


}


/**
 *
 *    \tparam T - DistanceModel
 */
template <class T>
void DistanceTransformFillOp<T>::traverseDownRight(const Image &src,const Image &srcAlpha,
		Image &dst, Image &dstAlpha) const {



	const int width  = src.getWidth();
	const int height = src.getHeight();
	const CoordinateHandler2D coordinateHandler(width,height,src.getCoordinatePolicy());

	//unsigned int iChannels = src.getImageChannelCount();

	// proximity (inverted distance)
	typedef float dtype;
	dtype d;
	//dtype dOrig;
	dtype dTest;

	// Intensity (graylevel)
	//T g;
	//std::cerr << "iChannels: " << iChannels << std::endl;
	std::vector<int> pixel(src.getImageChannelCount());
	//std::vector<dtype> pixel(src.getImageChannelCount());

	//std::cerr << "koe" << std::endl;

	/// Current cursor location
	Point2D<int> p;
	int &px = p.x;
	int &py = p.y;

	Point2D<int> pTest;
	Point2D<int> pWin;

	//coordinateOverflowHandler.setBounds(srcDist.getBounds());
	const DistanceModel & distanceModel = this->getDistanceModel();
	//const DistanceModel & distanceModel = this->distanceModel;

	// debug
	//double D_ORIG;

	for (py=0; py<height; py++){
		for (px=0; px<width; px++){

			// Take source value as default
			d = srcAlpha.get<dtype>(p);
			src.getPixel(p, pixel);
			pWin.setLocation(-1, -1);

			// Compare to upper left neighbour
			pTest.setLocation(px-1,py-1);
			coordinateHandler.handle(pTest); //,width,height);
			dTest = distanceModel.decreaseDiag(dstAlpha.get<dtype>(pTest));
			if (dTest > d){
				d = dTest;
				pWin = pTest;
				// dst.getPixel(pTest,pixel);
			}

			// Compare to upper neighbour
			pTest.setLocation(px,py-1);
			coordinateHandler.handle(pTest); //width,height);
			dTest = distanceModel.decreaseVert(dstAlpha.get<dtype>(pTest));
			if (dTest > d){
				d = dTest;
				pWin = pTest;
				// dst.getPixel(pTest,pixel);
			}

			// Compare to upper right neighbour
			pTest.setLocation(px+1,py-1);
			coordinateHandler.handle(pTest); //width,height);
			dTest = distanceModel.decreaseDiag(dstAlpha.get<dtype>(pTest));
			if (dTest > d){
				d = dTest;
				pWin = pTest;
				// dst.getPixel(pTest,pixel);
			}

			// Compare to left neighbour
			pTest.setLocation(px-1,py);
			coordinateHandler.handle(pTest); //,width,height);
			dTest = distanceModel.decreaseHorz(dstAlpha.get<dtype>(pTest));
			if (dTest > d){
				d = dTest;
				pWin = pTest;
				// dst.getPixel(pTest,pixel);
			}

			// HORSE
			if (distanceModel.KNIGHT){

				// Compare to further upper left neighbour
				pTest.setLocation(px-1, py-2);
				coordinateHandler.handle(pTest); //width,height);
				dTest = distanceModel.decreaseKnightVert(dstAlpha.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
					// dst.getPixel(pTest,pixel);
				}

				// Compare to further upper right neighbour
				pTest.setLocation(px+1, py-2);
				coordinateHandler.handle(pTest); //width,height);
				dTest = distanceModel.decreaseKnightVert(dstAlpha.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

				// Compare to further left upper neighbour
				pTest.setLocation(px-2, py-1);
				coordinateHandler.handle(pTest); //,width,height);
				dTest = distanceModel.decreaseKnightHorz(dstAlpha.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

				// Compare to further right upper neighbour
				pTest.setLocation(px+2, py-1);
				coordinateHandler.handle(pTest); //,width,height);
				dTest = distanceModel.decreaseKnightHorz(dstAlpha.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

			}



			// Finally, update target image (if data found)
			if (d > 0.0){

				if ((pWin.x != -1) && (pWin.y != -1)){
					dst.getPixel(pWin, pixel);
					/*
					if (false){
						for (int k=0; k<pixel.size(); k++){
							pixel[k] = (dOrig*pixel[k] + d*dst.get<double>(pWin.x, pWin.y, k)) / (dOrig+d);
						}
					}
					else
					*/
				}
				dstAlpha.put(p, d);
				dst.putPixel(p, pixel);
			}

		}
	}
	// return dst;
}


/**
 *
 *    \tparam T - DistanceModel
 */
template <class T>
void DistanceTransformFillOp<T>::traverseUpLeft(
		const Image &src, const Image &srcAlpha,
		Image &dst, Image &dstAlpha) const {

	const int width  = src.getWidth();
	const int height = src.getHeight();
	const CoordinateHandler2D coordinateHandler(width, height, src.getCoordinatePolicy()) ;

	// proximity (inverted distance)
	typedef float dtype;
	dtype d;
	//dtype dOrig;
	dtype dTest;

	std::vector<int> pixel(src.getImageChannelCount());
	std::vector<int> pixelTmp(src.getImageChannelCount());

	/// Current cursor location
	Point2D<int> p;
	int &px = p.x;
	int &py = p.y;

	Point2D<int> pTest;
	Point2D<int> pWin;

	const DistanceModel & distanceModel = this->getDistanceModel();

	for (py=height-1; py>=0; py--){
		for (px=width-1; px>=0; px--){

			d = srcAlpha.get<dtype>(p);
			src.getPixel(p,pixel);
			pWin.setLocation(-1, -1);

			// Compare to lower left neighbour
			pTest.setLocation(px-1,py+1);
			coordinateHandler.handle(pTest); //,width,height);
			dTest = distanceModel.decreaseDiag(dstAlpha.get<dtype>(pTest));
			if (dTest > d){
				d = dTest;
				pWin = pTest;
			}

			// Compare to lower neighbour
			pTest.setLocation(px,py+1);
			coordinateHandler.handle(pTest); //,width,height);
			dTest = distanceModel.decreaseVert(dstAlpha.get<dtype>(pTest));
			if (dTest > d){
				d = dTest;
				pWin = pTest;
			}

			// Compare to lower right neighbour
			pTest.setLocation(px+1,py+1);
			coordinateHandler.handle(pTest); //,width,height);
			dTest = distanceModel.decreaseDiag(dstAlpha.get<dtype>(pTest));
			if (dTest > d){
				d = dTest;
				pWin = pTest;
			}

			// Compare to right neighbour
			pTest.setLocation(px+1,py);
			coordinateHandler.handle(pTest); //width,height);
			dTest = distanceModel.decreaseHorz(dstAlpha.get<dtype>(pTest));
			if (dTest > d){
				d = dTest;
				pWin = pTest;
			}

			// Chess knight's move: +2+1
			if (distanceModel.KNIGHT){

				// Compare to further lower left neighbour
				pTest.setLocation(px+1, py+2);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightVert(dstAlpha.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

				// Compare to further lower right neighbour
				pTest.setLocation(px-1, py+2);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightVert(dstAlpha.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

				// Compare to further right lower neighbour
				pTest.setLocation(px+2, py+1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightHorz(dstAlpha.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

				// Compare to further left lower neighbour
				pTest.setLocation(px-2, py+1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightHorz(dstAlpha.get<dtype>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

			}


			// Finally, update target image (if stronger data found)
			if (d > 0.0){ // && (srcAlpha.get<dtype>(px,py) > 0.0) ){

				if ((pWin.x != -1) && (pWin.y != -1)){
					dst.getPixel(pWin, pixel);
					/* /// BLEND
					if (false){
						for (int k=0; k<pixel.size(); k++){
							pixel[k] = (dOrig*pixel[k] + d*dst.get<double>(pWin.x, pWin.y, k)) / (dOrig+d);
						}
					}
					else
					*/
				}

				dstAlpha.put(p, d);
				dst.putPixel(p, pixel);
			}




		}
	}
}









/** Spreeads values using inverse linear distance transform.

16-bit under construction
\~exec
drainage --geometry 256,256,3,1 --plot 96,96,255,64,32,255  --plot 160,96,64,255,32,208  --plot 128,160,64,32,255,192  -o dots-rgba.png
\~

Examples:
 \code
 drainage dots-rgba.png --distanceTransformFill 10 -o distFill-linear.png --view i -o distFillPlain-linear.png
 \endcode

\~exec
convert -fill white -frame 2 +append  distFill-linear.png distFillPlain-linear.png   distFill2-linear.png
\~


*/
class DistanceTransformFillLinearOp : public DistanceTransformFillOp<DistanceModelLinear>
{

public:

	inline
	DistanceTransformFillLinearOp(double horz = 10.0, double vert = -1.0) :
	DistanceTransformFillOp<DistanceModelLinear> (__FUNCTION__, "Spreads intensities linearly up to distance defined by alpha intensities.",
			horz, vert) {
	};
};

/**

Examples:
 \code
 drainage dots-rgba.png --distanceTransformFillExp 10 -o distFill-exp.png --view i -o distFillPlain-exp.png
 \endcode

\~exec
convert -fill white -frame 2 +append  distFill-exp.png distFillPlain-exp.png   distFill2-exp.png
\~



 */
class DistanceTransformFillExponentialOp : public DistanceTransformFillOp<DistanceModelExponential>
{

public:

	inline
	DistanceTransformFillExponentialOp(double horz = 10.0, double vert = -1.0) :
	DistanceTransformFillOp<DistanceModelExponential> (__FUNCTION__, "Spreads intensities exponentially up to distance defined by alpha intensities.",
		horz, vert) {
	};

};


}
}


#endif /* DISTANCETRANSFORMFILL_H_ */

// Drain
