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
#ifndef PixelVectorOP_H_
#define PixelVectorOP_H_

#include <cmath>

#include "ImageOp.h"
//#include "util/Functor.h"
//#include "util/Fuzzy.h"
//#include "util/Registry.h"
#include "util/FunctorBank.h"

#include "util/FunctorPack.h"

namespace drain
{

namespace image
{



///  A base class for operations between of intensity vectors of two images.
/**  In each point(i,j), performs  an operation for vectors constructed the intensities in different channels.
 *   Produces a flat, single-channel image.
 *
 *  \see DistanceOp
 *  \see ProductOp
 *  \see MagnitudeOp
 */
class PixelVectorOp : public ImageOp
{
public:



	virtual
	void makeCompatible(const ImageFrame &src, Image &dst) const;

	/*
	virtual
	inline
	void traverseFrame(const ImageFrame &src, ImageFrame & dst) const {
		Logger mout(getImgLog(), name+"::PixelVectorOp[src, dst]", __FUNCTION__);
		mout.error() << "undefined method" << mout.endl;
		//traverseFrame(src, dst, dst);
	}
	*/


	std::string functorName;
	std::string functorParams;

protected:

	PixelVectorOp(const std::string & name, const std::string & description) :
		ImageOp(name,description + " Post-scaling with desired functor."), rescale(0.0), POW(1.0), INVPOW(1.0) { //, l(1){ , rescale(0.0),
		parameters.separator = ':';
		parameters.reference("functor", this->functorName);
		parameters.reference("params", this->functorParams);
	};

	double rescale;
	double POW;
	double INVPOW;

};



template <class F>
class BinaryPixelVectorOp : public PixelVectorOp
{

public:

	BinaryPixelVectorOp(const std::string & name, const std::string & description) : PixelVectorOp(name, description){
	};

	virtual
	inline
	void process(const ImageFrame &src, Image &dst) const {
		Logger mout(getImgLog(), name+"(PixelVectorOp)", __FUNCTION__);
		mout.debug() << "delegating: process(src, dst, dst)" << mout.endl;
		process(src, dst, dst);
	};

	//double getValue(double src) const {return 0.0;};

	virtual
	inline
	void process(const ImageFrame &src1, const ImageFrame &src2, Image &dst) const {
		Logger mout(getImgLog(), name+"(PixelVectorOp)", __FUNCTION__);
		mout.debug() << "imitating: " << mout.endl;
		//process2WithTemp(src, dst);
		if (src2.hasOverlap(dst)){
			Image tmp;
			tmp.copyDeep(src2);
			makeCompatible(src1, dst);
			traverseFrame(src1, tmp, dst);
			/*
			const ImageTray<const Channel> srcTray1(src1);
			const ImageTray<const Channel> srcTray2(tmp);
			ImageTray<Channel> dstTray(dst);
			traverseChannels(srcTray1, srcTray2, dstTray);
			*/
		}
		else {
			makeCompatible(src1, dst);
			/*
			const ImageTray<const Channel> srcTray1(src1);
			const ImageTray<const Channel> srcTray2(src2);
			ImageTray<Channel> dstTray(dst);
			traverseChannels(srcTray1, srcTray2, dstTray); */
			traverseFrame(src1, src2, dst); // lower
		}
	};

	virtual
	void traverseChannels(const ImageTray<const Channel> & src, const ImageTray<const Channel> & src2,
			ImageTray<Channel> & dst) const;

	void traverseFrame(const ImageFrame &src1, const ImageFrame &src2, ImageFrame &dst) const {
		Logger mout(getImgLog(), name+"(PixelVectorOp)", __FUNCTION__);
		mout.debug() << "delegate to: traverseChannels(src, dst, dst)" << mout.endl;
		ImageTray<const Channel> src1Channels;
		src1Channels.setChannels(src1);
		ImageTray<const Channel> src2Channels;
		src2Channels.setChannels(src2);
		ImageTray<Channel> dstChannels;
		dstChannels.setChannels(dst);
		traverseChannels(src1Channels, src2Channels, dstChannels);
	}

	/*
	inline
	double getValue(double src, double src2) const {
		const BinaryFunctor & f = functor;
		return f(src, src2);
	}
	*/
protected:

	F binaryFunctor;

};

template <class F>
void BinaryPixelVectorOp<F>::traverseChannels(const ImageTray<const Channel> & src1, const ImageTray<const Channel> & src2,
		ImageTray<Channel> & dst) const {


	Logger mout(getImgLog(), name+"(PixelVectorOp)", __FUNCTION__);

	if (src1.empty())
		mout.error() << "src1 empty" << mout.endl;

	if (src2.empty())
		mout.error() << "src2 empty" << mout.endl;

	if (dst.empty())
		mout.error() << "dst empty" << mout.endl;

	Channel & dstChannel = dst.get();

	if (src1.getGeometry() != src2.getGeometry()){
		mout.warn() << "src1" << src1.getGeometry() << mout.endl;
		mout.warn() << "src2" << src2.getGeometry() << mout.endl;
		mout.note() << "dstChannel" << dstChannel << mout.endl;
		mout.error() << "src1 and src2 geometry error" << mout.endl;
		return;
	}

	const int width  = src1.getGeometry().getWidth(); //std::min(src1.getWidth(),src2.getWidth());
	const int height = src1.getGeometry().getHeight(); //std::min(src1.getHeight(),src2.getHeight());
	const size_t channels = src1.size(); // std::min(src1.getChannelCount(),src2.getChannelCount());

	double x=0.0, sum=0.0;

	mout.debug(2) << *this << mout.endl;
	//mout.debug(2) << "src1: " << src1  << mout.endl;
	//mout.debug(2) << "src2: " << src2 << mout.endl;
	//mout.debug(2) << "dst:  " << dst  << mout.endl;

	FunctorBank & functorBank = getFunctorBank();

	if (!functorBank.has(functorName)){
		//functorBank.help(std::cerr);
		mout.error() << "functor not found: " << functorName << mout.endl;
		return;
	}

	UnaryFunctor & scalingFunctor = functorBank.get(functorName).clone();
	//mout.debug() << scalingFunctor.getName() << ':' << scalingFunctor.getDescription() << mout.endl;
	//scalingFunctor.setScale(dstChannel.scaling.getMax<double>(), 0.0);
	const std::type_info & t = dstChannel.getType();
	if (Type::call<drain::typeIsSmallInt>(t)){
		const double m = Type::call<typeMax, double>(t);
		scalingFunctor.setScale(m, 0.0); // TODO consider invScale, invOffset?
		mout.info() << "small int type, scaling with its maximum (" << m << ')' << mout.endl;
	}

	scalingFunctor.setParameters(functorParams);
	mout.debug(1) << scalingFunctor.getName() << ':' << scalingFunctor << mout.endl;

	const double coeff = (rescale>0.0) ? 1.0/rescale : 1.0/static_cast<double>(channels);
	const bool USE_POW    = (POW != 1.0);
	const bool USE_INVPOW = (INVPOW != 1.0);
	mout.debug(2) << "coeff " << coeff << mout.endl;
	const BinaryFunctor & f = binaryFunctor;
	size_t a;
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {

			sum = 0.0;
			a = dstChannel.address(i,j);
			for (size_t k = 0; k < channels; k++){
				// getValue( src1.get(k).get<double>(a) , src2.get(k).get<double>(a) );
				x = f(src1.get(k).get<double>(a) , src2.get(k).get<double>(a)) ;
				if (USE_POW)
					sum += pow(x, POW);
				else
					sum += x;
			}
			if (USE_INVPOW)
				dstChannel.put(a, scalingFunctor(pow(coeff*sum, INVPOW)));
			else
				dstChannel.put(a, scalingFunctor(coeff*sum));
		}
	}

}


/// Computes the distance between intensity vectors.
/**
 *    dst(i,j) = \sqrt{ src(i,j,0)*src2(i,j,0) + ... + src(i,j,k)*src2(i,j,k) }.

 \code
   drainage color.png color-rot180.png  --distance FuzzyStep:0,255 -o distance.png
 \endcode

 \see ProductOp
 \see MagnitudeOp
 */
class DistanceOp : public BinaryPixelVectorOp<SubtractionFunctor> {

public:

	DistanceOp() : BinaryPixelVectorOp<SubtractionFunctor>(__FUNCTION__, "Computes the distance of pixel vectors."){
		POW    = 2.0;
		INVPOW = 0.5;
	};

};


///  Computes dot product of intensities of two images.
/**!
 *    dst(i,j) = \sqrt{ src(i,j,0)*src2(i,j,0) + ... + src(i,j,k)*src2(i,j,k) }.

\code
  drainage color.png color-rot180.png --product FuzzyStep:0,155  -o fuzzyStep.png
\endcode

 *  \see DistanceOp
 *  \see MagnitudeOp
 */
class ProductOp : public BinaryPixelVectorOp<MultiplicationFunctor>
{
public:

	ProductOp() : BinaryPixelVectorOp<MultiplicationFunctor>(__FUNCTION__, "Computes the dot product of pixel vectors."){
		POW    = 1.0;
		INVPOW = 0.5;
	};

};

/// Computes pixel magnitude, that is, euclidean norm of the intensity vector.
/**

 dst(i,j) = \sqrt{ src(i,j,0)^2 + ... + src(i,j,k)^2 }.

 \code
 drainage image.png --magnitude FuzzyStep:0,255   -o magnitude-step.png
 drainage image.png --magnitude FuzzyBell:0,-150  -o magnitude-bell.png
 \endcode

 \see DistanceOp
 \see ProductOp

 */
class MagnitudeOp : public BinaryPixelVectorOp<MultiplicationFunctor> {

public:

	//MagnitudeOp(const std::string & mapping = "linear", double l=2.0) : PixelVectorOp(__FUNCTION__,"Computes the magnitude of a pixel vector."){
	MagnitudeOp() : BinaryPixelVectorOp<MultiplicationFunctor>(__FUNCTION__, "Computes the magnitude of a pixel vector."){
		POW = 1.0; // because x*x natively
		INVPOW = 0.5;
	};

	virtual
	inline
	void process(const ImageFrame &src, Image &dst) const {
		Logger mout(getImgLog(), name+"(PixelVectorOp)", __FUNCTION__);
		mout.debug() << "delegating to: process(src, src, dst)" << mout.endl;
		BinaryPixelVectorOp<MultiplicationFunctor>::process(src, src, dst);
	};

	virtual
	inline
	void traverseFrame(const ImageFrame &src, ImageFrame & dst) const {
		Logger mout(getImgLog(), name+"::PixelVectorOp[src, dst]", __FUNCTION__);
		mout.warn() << "delegating to: traverseFrame(src, src, dst)" << mout.endl;
		BinaryPixelVectorOp<MultiplicationFunctor>::traverseFrame(src, src, dst);
	}

	virtual
	inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		Logger mout(getImgLog(), name+"::PixelVectorOp[src,dst]", __FUNCTION__);
		mout.debug() << "delegate to: traverseChannels(src, SRC, dst)" << mout.endl;
		BinaryPixelVectorOp<MultiplicationFunctor>::traverseChannels(src, src, dst);
	}

};


}
}


#endif /*PixelVectorOP_H_*/

// Drain
