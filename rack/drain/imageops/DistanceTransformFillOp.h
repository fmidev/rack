/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANpy; without even the implied warranpy of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */
#ifndef DISTANCETRANSFORMFILLOP_H_
#define DISTANCETRANSFORMFILLOP_H_

#include "image/File.h" // debugging
#include "DistanceTransformOp.h"

//#include "QualityMixerOp.h"


namespace drain
{

namespace image
{

/// Template for DistanceTransformFillLinearOp and DistanceTransformFillExponentialOp
/**

  \tparam T - distance model, like DistanceModelLinear or DistanceModelExponential

 */
template <class T>
class DistanceTransformFillOp : public DistanceTransformOp<T>
{
public:

	// proximity (inverted distance)
	typedef typename T::dist_t dist_t;

	virtual
	~DistanceTransformFillOp(){};

	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;

	virtual
	void traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {
		ImageTray<const Channel> srcTray; //(src, srcAlpha);
		srcTray.setChannels(src);
		srcTray.setAlphaChannels(srcAlpha);
		ImageTray<Channel> dstTray; //(dst, dstAlpha);
		dstTray.setChannels(dst);
		dstTray.setAlphaChannels(dstAlpha);
		traverseChannels(srcTray, dstTray);
	};


protected:

	DistanceTransformFillOp(const std::string &name, const std::string &description, double horz = 10.0, double vert = NAN) :
		DistanceTransformOp<T>(name, description, horz, vert) {
	};


	void traverseDownRight(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;

	void traverseUpLeft(ImageTray<Channel> & src, ImageTray<Channel> & dst) const ;

};

/**
 *
 *    \tparam T - DistanceModel
 */
template <class T>
void DistanceTransformFillOp<T>::traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
	// void DistanceTransformFillOp<T>::process(const ImageFrame &src, const ImageFrame &srcAlpha, ImageFrame & dst, ImageFrame & dstAlpha) const {

	drain::Logger mout(this->name, __FUNCTION__);

	mout.debug() << "start: " << *this << mout.endl;

	mout.debug(1)  << "src: " << src << mout.endl;
	mout.debug(1)  << "dst: " << dst << mout.endl;


	if (!src.hasAlpha() || !dst.hasAlpha()){
		mout.warn()  << "src: " << src << mout.endl;
		mout.warn()  << "dst: " << dst << mout.endl;
		mout.error() << "alpha missing in src and/or dst, see above warning" << mout.endl;
		return;
	}

	// mout.debug(2)  << "init params: " << mout.endl;
	// this->initializeParameters(src.get(),      dst.get());
	mout.debug(2)  << "init params, using alpha: " << mout.endl;
	this->initializeParameters(src.getAlpha(), dst.getAlpha());

	if (this->distanceModel.KNIGHT){
		//mout.warn() << "with chess knight 2+1 distance " << mout.endl;
		//mout.warn() << this->distanceModel.getParameters() << mout.endl;
	}

	mout.debug()   << "calling traverseDownRight" << mout.endl;
	mout.debug(1)  << src << mout.endl;
	mout.debug(1)  << dst << mout.endl;
	traverseDownRight(src, dst);
	/*
	if (getImgLog().getVerbosity() > 10){
		drain::image::File::write(dst,"dtd-i.png");
		drain::image::File::write(dstAlpha,"dtd-a.png");
	}
	 */

	mout.debug()  << "calling traverseUpLeft" << mout.endl;
	mout.debug(1)  << dst << mout.endl;
	traverseUpLeft(dst, dst);
	/*
	if (getImgLog().getVerbosity() > 10){
		drain::image::File::write(dst,"dtu-i.png");
		drain::image::File::write(dstAlpha,"dtu-a.png");
	}
	 */


}


//void DistanceTransformFillOp<T>::traverseDownRight(const ImageFrame &src, const ImageFrame &srcAlpha,
//ImageFrame &dst, ImageFrame &dstAlpha) const {

/**
 *
 *    \tparam T - DistanceModel
 */
template <class T>
void DistanceTransformFillOp<T>::traverseDownRight(const ImageTray<const Channel> & srcTray, ImageTray<Channel> & dstTray) const {

	Logger mout(this->getName(), __FUNCTION__);
	mout.debug() << "start" << mout.endl;

	const Geometry & srcGeometry = srcTray.getGeometry();

	const Channel & srcAlpha = srcTray.getAlpha();
	Channel & dstAlpha = dstTray.getAlpha();

	CoordinateHandler2D coordinateHandler;
	srcTray.adjustCoordinateHandler(coordinateHandler);
	mout.debug(1) << "coordHandler" << coordinateHandler << mout.endl;


	// Winning distance
	dist_t d;
	// Distance candidate
	dist_t dTest;

	// Intensities (graylevel)
	std::vector<double> pixel(srcGeometry.getImageChannelCount());

	mout.debug() << "channel depth:" << pixel.size() << '=' << srcGeometry.getImageChannelCount() << '=' << dstTray.getGeometry().getImageChannelCount() << mout.endl;
	//mout.debug(2) << "src alpha:" << srcAlpha << mout.endl;
	//mout.debug(2) << "dst alpha:" << dstAlpha << mout.endl;

	/// Current cursor location
	Point2D<int> p;
	int &px = p.x;
	int &py = p.y;

	Point2D<int> pTest;
	Point2D<int> pWin;


	const DistanceModel & distanceModel = this->getDistanceModel();

	mout.debug(1) << "distanceModel" << distanceModel << mout.endl;
	mout.debug() << "main loop" << mout.endl;

	for (py=0; py<=coordinateHandler.getYMax(); py++){// TODO check < vs <=
		for (px=0; px<=coordinateHandler.getYMax(); px++){ // TODO check < vs <=

			// Take source value as default
			d = srcAlpha.get<dist_t>(p);
			pWin.setLocation(-1, -1);

			// Compare to upper left neighbour
			if (distanceModel.DIAG){
				pTest.setLocation(px-1,py-1);
				coordinateHandler.handle(pTest); //,width,height);
				dTest = distanceModel.decreaseDiag(dstAlpha.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}
			}

			// Compare to upper neighbour
			pTest.setLocation(px,py-1);
			coordinateHandler.handle(pTest); //width,height);
			dTest = distanceModel.decreaseVert(dstAlpha.get<dist_t>(pTest));
			if (dTest > d){
				d = dTest;
				pWin = pTest;
			}

			// Compare to upper right neighbour
			if (distanceModel.DIAG){
				pTest.setLocation(px+1,py-1);
				coordinateHandler.handle(pTest); //width,height);
				dTest = distanceModel.decreaseDiag(dstAlpha.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}
			}

			// Compare to left neighbour
			pTest.setLocation(px-1,py);
			coordinateHandler.handle(pTest); //,width,height);
			dTest = distanceModel.decreaseHorz(dstAlpha.get<dist_t>(pTest));
			if (dTest > d){
				d = dTest;
				pWin = pTest;
			}

			// HORSE
			if (distanceModel.KNIGHT){

				// Compare to further upper left neighbour
				pTest.setLocation(px-1, py-2);
				coordinateHandler.handle(pTest); //width,height);
				dTest = distanceModel.decreaseKnightVert(dstAlpha.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

				// Compare to further upper right neighbour
				pTest.setLocation(px+1, py-2);
				coordinateHandler.handle(pTest); //width,height);
				dTest = distanceModel.decreaseKnightVert(dstAlpha.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

				// Compare to further left upper neighbour
				pTest.setLocation(px-2, py-1);
				coordinateHandler.handle(pTest); //,width,height);
				dTest = distanceModel.decreaseKnightHorz(dstAlpha.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

				// Compare to further right upper neighbour
				pTest.setLocation(px+2, py-1);
				coordinateHandler.handle(pTest); //,width,height);
				dTest = distanceModel.decreaseKnightHorz(dstAlpha.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

			}



			// Finally, update target image (if data found)
			if (d > 0.0){

				if ((pWin.x != -1) && (pWin.y != -1)){
					dstTray.getPixel(pWin, pixel);
				}
				else {
					srcTray.getPixel(p, pixel);
				}

				dstAlpha.put(p, d);
				dstTray.putPixel(p, pixel);
			}

		}
	}

}



///
/**
 *
 *    \tparam T - DistanceModel
 */
template <class T>
void DistanceTransformFillOp<T>::traverseUpLeft(ImageTray<Channel> & srcTray, ImageTray<Channel> & dstTray) const {

	Logger mout(this->getName(), __FUNCTION__);
	mout.debug() << "start" << mout.endl;

	const Geometry & srcGeometry = srcTray.getGeometry();

	CoordinateHandler2D coordinateHandler;
	srcTray.adjustCoordinateHandler(coordinateHandler);

	const Channel & srcAlpha = srcTray.getAlpha();
	Channel & dstAlpha = dstTray.getAlpha();

	// proximity (inverted distance)
	//typedef float dist_t;
	dist_t d;
	dist_t dTest;

	std::vector<double> pixel(srcGeometry.getImageChannelCount());
	//std::vector<int> pixelTmp(srcGeometry.getImageChannelCount());

	/// Current cursor location
	Point2D<int> p;
	int & px = p.x;
	int & py = p.y;

	Point2D<int> pTest;
	Point2D<int> pWin;

	const DistanceModel & distanceModel = this->getDistanceModel();
	mout.debug(1) << "distanceModel" << distanceModel << mout.endl;

	mout.debug() << "main loop" << mout.endl;

	for (py=coordinateHandler.getYMax(); py>=0; py--){
		for (px=coordinateHandler.getXMax(); px>=0; px--){

			d = srcAlpha.get<dist_t>(p);
			srcTray.getPixel(p,pixel); // move down
			pWin.setLocation(-1, -1);

			// Compare to lower left neighbour
			if (distanceModel.DIAG){
				pTest.setLocation(px-1,py+1);
				coordinateHandler.handle(pTest); //,width,height);
				dTest = distanceModel.decreaseDiag(dstAlpha.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}
			}

			// Compare to lower neighbour
			pTest.setLocation(px,py+1);
			coordinateHandler.handle(pTest); //,width,height);
			dTest = distanceModel.decreaseVert(dstAlpha.get<dist_t>(pTest));
			if (dTest > d){
				d = dTest;
				pWin = pTest;
			}

			// Compare to lower right neighbour
			if (distanceModel.DIAG){
				pTest.setLocation(px+1,py+1);
				coordinateHandler.handle(pTest); //,width,height);
				dTest = distanceModel.decreaseDiag(dstAlpha.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}
			}

			// Compare to right neighbour
			pTest.setLocation(px+1,py);
			coordinateHandler.handle(pTest); //width,height);
			dTest = distanceModel.decreaseHorz(dstAlpha.get<dist_t>(pTest));
			if (dTest > d){
				d = dTest;
				pWin = pTest;
			}

			// Chess knight's move: +2+1
			if (distanceModel.KNIGHT){

				// Compare to further lower left neighbour
				pTest.setLocation(px+1, py+2);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightVert(dstAlpha.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

				// Compare to further lower right neighbour
				pTest.setLocation(px-1, py+2);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightVert(dstAlpha.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

				// Compare to further right lower neighbour
				pTest.setLocation(px+2, py+1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightHorz(dstAlpha.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

				// Compare to further left lower neighbour
				pTest.setLocation(px-2, py+1);
				coordinateHandler.handle(pTest);
				dTest = distanceModel.decreaseKnightHorz(dstAlpha.get<dist_t>(pTest));
				if (dTest > d){
					d = dTest;
					pWin = pTest;
				}

			}


			// Finally, update target image (if stronger data found)
			if (d > 0.0){

				if ((pWin.x != -1) && (pWin.y != -1)){
					dstTray.getPixel(pWin, pixel);
				}
				else {
					srcTray.getPixel(p, pixel);
				}

				dstAlpha.put(p, d);
				dstTray.putPixel(p, pixel);
			}

		}
	}
}









/** Spreeads values using inverse linear distance transform.

16-bit under construction
\~exec
 # rainage --geometry 256,256,3,1 --plot 96,96,255,64,32,255  --plot 160,96,64,255,32,208  --plot 128,160,64,32,255,192  -o dots-rgba.png
 make dots-rgba.png
\~

Examples:
 \code
 drainage dots-rgba.png --distanceTransformFill 30 -o distFill-linear.png --view i -o distFillPlain-linear.png
 \endcode

\~no exec
  #onvert -fill white -frame 2 +append  distFill-linear.png distFillPlain-linear.png   distFill2-linear.png
\~


 */
class DistanceTransformFillLinearOp : public DistanceTransformFillOp<DistanceModelLinear>
{

public:

	inline
	DistanceTransformFillLinearOp(dist_t horz = 10.0, dist_t vert = NAN) :
	DistanceTransformFillOp<DistanceModelLinear> (__FUNCTION__, "Spreads intensities linearly up to distance defined by alpha intensities.",
			horz, vert) {
	};
};

/**

Examples:
 \code
 drainage dots-rgba.png --distanceTransformFillExp 15 -o distFill-exp.png --view i -o distFillPlain-exp.png
 \endcode

\~no exec
 # onvert -fill white -frame 2 +append  distFill-exp.png distFillPlain-exp.png   distFill2-exp.png
\~

 */
class DistanceTransformFillExponentialOp : public DistanceTransformFillOp<DistanceModelExponential>
{

public:

	inline
	DistanceTransformFillExponentialOp(dist_t horz = 10.0, dist_t vert = NAN) :
	DistanceTransformFillOp<DistanceModelExponential> (__FUNCTION__, "Spreads intensities exponentially up to distance defined by alpha intensities.",
			horz, vert) {
	};

};


}
}


#endif /* DISTANCETRANSFORMFILL_H_ */
