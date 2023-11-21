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

#include <drain/image/ImageFile.h> // debugging
#include <drain/util/Fuzzy.h>
#include "FunctorOp.h"
#include "DistanceTransformOp.h"


namespace drain
{

namespace image
{

/// Template for DistanceTransformFillLinearOp and DistanceTransformFillExponentialOp
/**

  \tparam T - iDistance model, like DistanceModelLinear or DistanceModelExponential

 */
template <class T>
class DistanceTransformFillOp : public DistanceTransformOp<T>
{
public:

	virtual inline
	int srcAlpha() const {
		return 2; // REQUIRED
	};

	// proximity (inverted distance)
	typedef float dist_t;

	virtual
	~DistanceTransformFillOp(){};

    virtual
    void getDstConf(const ImageConf & srcConf, ImageConf & dstConf) const {

    	dstConf.setGeometry(srcConf.getGeometry());
    	if (!dstConf.hasAlphaChannel())
    		dstConf.setAlphaChannelCount(1);

    }


    /// Like ImageOp::makeCompatible(), but clears the alpha channel.
    virtual // TODO: non-virtual, ie, final!
	void makeCompatible(const ImageConf & src, Image & dst) const {

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		mout.debug3() << "src: " << src << mout;

		ImageConf dstConf(dst.getConf());
		dstConf.setCoordinatePolicy(src.getCoordinatePolicy());
		getDstConf(src, dstConf);

		if (!dstConf.typeIsSet())
			dstConf.setType(src.getType());

		if (!dstConf.hasAlphaChannel())
			dstConf.setAlphaChannelCount(1);


		dst.setConf(dstConf);

		mout.debug() << "clearing alpha (use traverseChannel(s) to avoid)" << mout;

		// Important
		dst.getAlphaChannel().fill(0);

		mout.debug3() << "dst: " << dst << mout;

	};


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

	DistanceTransformFillOp(const std::string &name, const std::string &description, dist_t horz = 10.0, dist_t vert = NAN,
			DistanceModel::topol_t topology=DistanceModel::KNIGHT) : // PIX_ADJACENCY_
		DistanceTransformOp<T>(name, description, horz, vert, topology), alphaThreshold(0.0, 0.0) {
		this->parameters.link("alphaThreshold", alphaThreshold.tuple(), "0..1").fillArray = true;
	};

	//DistanceTransformFillOp(const DistanceTransformFillOp & op) : DistanceTransformOp<T>(op) {
	//};


	void traverseDownRight(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;

	void traverseUpLeft(ImageTray<Channel> & src, ImageTray<Channel> & dst) const ;

	//double alphaThreshold;
	drain::Range<double> alphaThreshold;

};

/**
 *
 *    \tparam T - DistanceModel
 */
template <class T>
void DistanceTransformFillOp<T>::traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
	// void DistanceTransformFillOp<T>::process(const ImageFrame &src, const ImageFrame &srcAlpha, ImageFrame & dst, ImageFrame & dstAlpha) const {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.debug() << "start: " << *this << mout.endl;

	if (!src.hasAlpha()){
		mout.warn()  << "src: " << src << mout;
		mout.error() << "required alpha channel missing in src" << mout;
		return;
	}
	else
		mout.debug2()  << "src: " << src << mout.endl;

	if (!dst.hasAlpha()){
		mout.warn()  << "dst: " << dst << mout;
		mout.error() << "required alpha channel missing in dst" << mout.endl;
		return;
	}
	else
		mout.debug2()  << "dst: " << dst << mout.endl;


	// mout.debug3()  << "init params: " << mout.endl;
	mout.debug3()  << "init params, using alpha: " << mout.endl;
	this->initializeParameters(src.getAlpha(), dst.getAlpha());
	// dst.getAlpha().fill(0); NEW: MOved to ...

	//drain::image::File::write(src.getAlpha(), "dts-a.png");
	//drain::image::File::write(dst.getAlpha(),"dt0-a.png");


	mout.debug()   << "calling traverseDownRight" << mout.endl;
	// mout.debug2()  << src << mout.endl;
	// mout.debug2()  << dst << mout.endl;
	traverseDownRight(src, dst);
	/*
	if (getImgLog().getVerbosity() > 10){
		drain::image::File::write(dst,"dtd-i.png");
		drain::image::File::write(dstAlpha,"dtd-a.png");
	}
	 */

	mout.debug()  << "calling traverseUpLeft" << mout.endl;
	// mout.debug2()  << dst << mout.endl;
	traverseUpLeft(dst, dst);
	/*
	if (getImgLog().getVerbosity() > 10){
		drain::image::File::write(dst,"dtu-i.png");
		drain::image::File::write(dstAlpha,"dtu-a.png");
	}
	 */
	if (alphaThreshold.max > 0.0){ // TODO: check if (0,1) = neutral

		Channel & dstAlpha = dst.alpha.get();

		UnaryFunctorOp<FuzzyStep<double>,true> threshold;
		threshold.functor.set(alphaThreshold);
		mout.debug() << "Thresholding: "<< threshold.functor << mout;
		// mout.debug() << threshold << mout;
		threshold.traverseChannel(dstAlpha, dstAlpha);
		//FilePng::write(dstAlpha, "a2.png");
	}


}

/// Step 1/2: forward traversal.
/**
 *
 *    \tparam T - DistanceModel
 */
template <class T>
void DistanceTransformFillOp<T>::traverseDownRight(const ImageTray<const Channel> & srcTray, ImageTray<Channel> & dstTray) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.debug() << "distModel: " << this->distanceModel << mout;

	DistanceNeighbourhood chain;
	this->distanceModel.createChain(chain, true);

	mout.debug2() << "neighbourHood " << drain::sprinter(chain) << mout;

	const Channel & srcAlpha = srcTray.getAlpha();
	Channel & dstAlpha = dstTray.getAlpha();

	CoordinateHandler2D coordinateHandler(srcTray.get(0));
	mout.debug() << "coordHandler " << coordinateHandler << mout;

	mout.debug3() << "src alpha:" << srcAlpha << mout;
	mout.debug3() << "dst alpha:" << dstAlpha << mout;

	/// Current cursor location
	Point2D<int> p;
	Point2D<int> pTest;
	Point2D<int> pWin;

	// Winning distance
	dist_t dWin;
	// Distance candidate
	dist_t dTest;

	const size_t K = std::min(srcTray.size(), dstTray.size());

	const Range<int> & xRange = coordinateHandler.getXRange();
	const Range<int> & yRange = coordinateHandler.getYRange();

	size_t address = 0;
	size_t addressWin = 0;

	mout.debug() << "main loop, K=" << K <<  mout.endl;
	for (p.y=0; p.y<=yRange.max; ++p.y){
		for (p.x=0; p.x<=xRange.max; ++p.x){

			address = dstAlpha.address(p.x, p.y);

			// Take source value as default
			dWin = srcAlpha.get<dist_t>(address);
			pWin.setLocation(p);

			for (const DistanceElement & elem: chain){
				pTest.setLocation(p.x + elem.diff.x, p.y + elem.diff.y);
				coordinateHandler.handle(pTest);
				dTest = this->distanceModel.decrease(dstAlpha.get<dist_t>(pTest), elem.coeff);
				if (dTest > dWin){
					dWin = dTest;
					pWin = pTest; // consider direct addressWin = dstAlpha.address(pWin.x, pWin.y); so, pTest
				}
			}


			// Finally, update target image (if data found)
			if (dWin > 0.0){

				dstAlpha.put(address, dWin);

				addressWin = dstAlpha.address(pWin.x, pWin.y);

				if (addressWin != address){
					for (size_t k=0; k<K; ++k) // copy from DST
						dstTray.get(k).put(address, dstTray.get(k).get<dist_t>(addressWin));
				}
				else {
					for (size_t k=0; k<K; ++k) // copy from SRC
						dstTray.get(k).put(address, srcTray.get(k).get<dist_t>(address));
				}

			}

			/*
			if ((p.x == p.y) && ((p.x&15)==0)){
				std::cerr << p << "\t a=" << address << ", aW=" << addressWin << "\t" << dWin << '>' << dTest  <<  '\t';
				std::cerr << p << "\t s=" << srcAlpha.get<dist_t>(address) << '=' << srcAlpha.get<dist_t>(p) <<  '\t';
				std::cerr << p << "\t d=" << dstAlpha.get<dist_t>(address) << '=' << dstAlpha.get<dist_t>(p) <<  '\n';
			}
			*/

		}
	}

}



/// Step 1/2: backward traversal.
/**
 *
 *    \tparam T - DistanceModel
 */
template <class T>
void DistanceTransformFillOp<T>::traverseUpLeft(ImageTray<Channel> & srcTray, ImageTray<Channel> & dstTray) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);
	mout.debug() << "start" << mout.endl;

	mout.debug2() << "this->distanceModel" << this->distanceModel << mout.endl;

	DistanceNeighbourhood chain;
	this->distanceModel.createChain(chain, false);

	CoordinateHandler2D coordinateHandler(srcTray.get(0));

	const Channel & srcAlpha = srcTray.getAlpha();
	Channel & dstAlpha = dstTray.getAlpha();

	/// Current cursor location
	Point2D<int> p;
	Point2D<int> pTest;
	Point2D<int> pWin;

	// proximity (inverted distance)
	dist_t dWin;
	dist_t dTest;

	// Current position
	size_t address;
	// Winning position
	size_t addressWin;

	const size_t K = std::min(srcTray.size(), dstTray.size());

	const Range<int> & xRange = coordinateHandler.getXRange();
	const Range<int> & yRange = coordinateHandler.getYRange();

	mout.debug("main loop, K=", K);

	for (p.y=yRange.max; p.y>=0; --p.y){
		for (p.x=xRange.max; p.x>=0; --p.x){

			address = dstAlpha.address(p.x, p.y);

			dWin = srcAlpha.get<dist_t>(address);
			pWin.setLocation(p);

			for (const DistanceElement & elem: chain){
				pTest.setLocation(p.x + elem.diff.x, p.y + elem.diff.y);
				coordinateHandler.handle(pTest);
				dTest = this->distanceModel.decrease(dstAlpha.get<dist_t>(pTest), elem.coeff);
				if (dTest > dWin){
					dWin = dTest;
					pWin = pTest;
				}
			}


			// Finally, update target image (if stronger data found)
			if (dWin > 0.0){

				dstAlpha.put(address, dWin);

				addressWin = dstAlpha.address(pWin.x, pWin.y);

				if (addressWin != address){
					for (size_t k=0; k<K; ++k) // copy from DST
						dstTray.get(k).put(address, dstTray.get(k).get<dist_t>(addressWin));
				}
				else {
					for (size_t k=0; k<K; ++k) // copy from SRC
						dstTray.get(k).put(address, srcTray.get(k).get<dist_t>(address));
				}

			}

		}
	}
}








/// Spreeads values using inverse linear distance transform.
/**

\~exec
make dots-rgba.png dots-rgba-16b.png
make spots-rgba.png spots-rgba-16b.png
\~

Examples on three distinct pixels (red, green, and blue):
\code
  drainage spots-rgba.png     --iDistanceTransformFill 50 -o distFill-rgba.png
  drainage spots-rgba-16b.png --iDistanceTransformFill 50 -o distFill-rgba-16b.png
\endcode



The radii do not have to be symmetric:
\code
  # Asymmetric radii:
  drainage spots-rgba.png     --iDistanceTransformFill 20:40,30:50 -o distFillAsym-rgba.png
\endcode


Examples on a graphical image:
\~exec
drainage graphic.png -V 0 --iNegate -o graphic-mask.png
drainage graphic.png -a graphic-mask.png -o graphic-rgba.png
make graphic-rgba-16b.png
\~
\code
  drainage graphic-rgba.png          --iDistanceTransformFill 20 -o distFill2.png
  # 16-bit image to 8-bit output
  drainage graphic-rgba-16b.png -T C --iDistanceTransformFill 20 -o distFill2-8b.png
  drainage graphic-rgba-16b.png -T S --iDistanceTransformFill 20 -o distFill2-16b.png
\endcode

\~exec
# ma ke graphic-tr-ia.png distFill2-ia.png distFill2-8b-ia.png distFill2-16b-ia.png
# con vert +append graphic-tr-ia.png distFill2-ia.png distFill-compare.png
#
\~

\see DistanceTransformFillExponentialOp
\see BlenderOp
\see FastAVerageOp

 */
class DistanceTransformFillLinearOp : public DistanceTransformFillOp<DistanceModelLinear>
{

public:

	inline
	DistanceTransformFillLinearOp(dist_t horz = 10.0, dist_t vert = NAN,
			DistanceModel::topol_t topology=DistanceModel::KNIGHT) :
			//DistanceModel::topol_t topology=DistanceModel::PIX_ADJACENCY_KNIGHT) :

			DistanceTransformFillOp<DistanceModelLinear> (__FUNCTION__, "Spreads intensities linearly up to distance defined by alpha intensities.",
			horz, vert, topology) {
	};

};


/**

\~exec
# ma ke dots-rgba.png dots-rgba-16b.png
#
make spots-rgba.png spots-rgba-16b.png
\~

Examples:
\code
  # Basic example
  drainage spots-rgba.png          --iDistanceTransformFillExp 50 -o distFill-exp-rgba.png
  drainage spots-rgba-16b.png -T S --iDistanceTransformFillExp 50 -o distFill-exp-rgba-16b.png
\endcode

Examples on a graphical image:
\~exec
drainage graphic.png -V 0 --iNegate -o graphic-mask.png
drainage graphic.png -a graphic-mask.png -o graphic-tr.png -T S --iCopy f -o graphic-tr-16b.png
\~
\code
  drainage graphic-tr.png          --iDistanceTransformFillExp 20 -o distFill2Exp.png
  # 16-bit image
  drainage graphic-tr-16b.png -T C --iDistanceTransformFillExp 20 -o distFill2Exp-8b.png
  # 16-bit image
  drainage graphic-tr-16b.png -T S --iDistanceTransformFillExp 20 -o distFill2Exp-16b.png
\endcode

\~exec
# ma ke graphic-tr-ia.png distFill2Exp-ia.png distFill2Exp-8b-ia.png distFill2Exp-16b-ia.png
# con vert +append graphic-tr-ia.png distFill2Exp-ia.png distFillExp-compare.png
\~


\see DistanceTransformFillOp
\see BlenderOp
\see FastAVerageOp

 */
class DistanceTransformFillExponentialOp : public DistanceTransformFillOp<DistanceModelExponential>
{

public:

	inline
	DistanceTransformFillExponentialOp(dist_t horz = 10.0, dist_t vert = NAN,
			//DistanceModel::topol_t topology=DistanceModel::PIX_ADJACENCY_KNIGHT) :
			DistanceModel::topol_t topology=DistanceModel::KNIGHT) :
			DistanceTransformFillOp<DistanceModelExponential> (__FUNCTION__, "Spreads intensities exponentially up to distance defined by alpha intensities.",
			horz, vert, topology) {
	};

};


}
}


#endif /* DISTANCETRANSFORMFILL_H_ */

