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
#ifndef FUNCTOROP_H_
#define FUNCTOROP_H_


#include "util/Functor.h"
#include "util/TypeUtils.h"
#include "image/File.h"
#include "ImageOp.h"

namespace drain
{

namespace image
{

// TODO: change to use functor bank, FunctorBank
/* *
 *   drainage image.png --functor FuzzyBell2,10,5  -o out.png
 *   drainage image.png image2.png --functor Add,10,5:FuzzyBell2,10,5 -o out.png
 *
 */

/// Helper class that only ensures that functor is initialized before ImageOp
template <class F>
struct FunctorWrapper {
	F functor;
};


/// Class for using simple function objects (like std::functor) for sequential pixel iteration.
/**!
 *   \template F - unary (single-parameter) functor
 *
 *   \see GammaOp
 */
template <class F>
class FunctorOp : public FunctorWrapper<F>, public ImageOp
{

public:

	FunctorOp() : LIMIT(false){};

	inline
	void adaptParameters(bool adaptLimit = false){
		ReferenceMap & p = this->getParameters();
		p.append(this->functor.getParameters());
		if (adaptLimit){
			if (!p.empty()) // ?
				p.reference("LIMIT", this->LIMIT);
		}
	}

	bool LIMIT;


protected:

	virtual
	bool processOverlappingWithTemp(const ImageFrame & srcFrame, Image & dstImage) const {
		return false;
	};

	inline
	bool processOverlappingWithTemp(const ImageTray<const Channel> & src, ImageTray<Image> & dst) const {
		return false;
	}

	/**
	 *  \param adaptParameters - add all the parameters of the functors to this op's parameters
	 *  \param adaptLimit      - add LIMIT option to visible parameters
	 */
	FunctorOp(bool adaptParams = false, bool adoptLimit = false) :
		FunctorWrapper<F>(),
		ImageOp(this->functor.getName(), this->functor.getDescription()),
		LIMIT(false) {
		if (adaptParams)
			adaptParameters(adoptLimit);
	};


	virtual ~FunctorOp(){};



};

/// Class for using simple function objects (like std::functor) for sequential pixel iteration.
/**!
 *   \template F - unary (single-parameter) functor
 */
template <class F>
class UnaryFunctorOp : public FunctorOp<F>
{
public:

	/**
	 *  \param adaptParameters - add all the parameters of the functors to this op's parameters
	 *  \param adaptLimit      - add LIMIT option to visible parameters
	 */
	//UnaryFunctorOp(bool adaptParams = false, bool adoptLimit = false) : FunctorOp<F>(adaptParams, adoptLimit) {
	UnaryFunctorOp(bool adaptParams = true, bool adoptLimit = true) : FunctorOp<F>(adaptParams, adoptLimit) {
	};


	virtual ~UnaryFunctorOp(){};

	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {  //  = 0;
		//drain::Logger mout(this->getName()+"(UnaryFunctorOp)", __FUNCTION__);
		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		mout.debug(1) << "invoking processChannelsSeparately()" << mout.endl;
		ImageOp::traverseChannelsSeparately(src, dst);
	}

	/// Process the image.
	/**
	 *   In future versions there may dedicated processing for integral and floatimg-point base types.
	 *   (If some of images are double, then
	 */
	void traverseChannel(const Channel &src, Channel & dst) const;

	/// Policy: exclude alpha channels, ie. apply to image channels only.
	virtual
	void traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {

		//Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), this->name+"(FunctorOp)", __FUNCTION__);
		//Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL this->name+"(FunctorOp)", __FUNCTION__);
		Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		mout.note() << "using image channels only (discarding alpha channels)" << mout.endl;
		traverseChannel(src, dst);
	};

protected:



	virtual
	void initializeParameters(const ImageFrame &src, const ImageFrame &dst) const {

		Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		// mout.warn() << *this << mout.endl;
		this->functor.update();

	}

};

template <class T>
void UnaryFunctorOp<T>::traverseChannel(const Channel &src, Channel & dst) const {

	//Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), this->name+"(UnaryFunctorOp)", __FUNCTION__);
	Logger mout(getImgLog(), __FUNCTION__, __FILE__);
	mout.debug() << "start" << mout.endl;

	const ImageScaling  & ss = src.getScaling();
	const ImageScaling  & ds = dst.getScaling();
	const bool SCALE = ss.isScaled() || ds.isScaled();
	mout.debug() << "SCALE=" << (int)SCALE << mout.endl;
	mout.debug() << "functor=" << this->functor << mout.endl;

	if (mout.isDebug(2)){
		for (int i = 0; i < 256; i+=8) {
			std::cout << i << "\t -> " << ss.fwd(i) << "\t" << this->functor(ss.fwd(i)) << '\n';
		}
	}

	if (SCALE){
		mout.debug(1) << "ss scale:" << ss << mout.endl;
		mout.debug(1) << "ds scale:" << ds << mout.endl;
	}

	Channel::const_iterator s  = src.begin();
	Channel::iterator d = dst.begin();

	if (!this->LIMIT){
		if (!SCALE){ // Simplest first
			/// Unlimited, unscaled, ie. "direct". This option may be sufficient if dst is double, for example.
			while (d != dst.end()){
				*d = this->functor(static_cast<double>(*s));
				++s;
				++d;
			}
		}
		else {
			while (d != dst.end()){
				*d = ds.inv(this->functor(ss.fwd(static_cast<double>(*s))));
				++s;
				++d;
			}
		}
	}
	else { // LIMIT

		if (!drain::Type::call<drain::typeIsInteger>(dst.getType()))
			mout.warn() << "float dst type, but LIMIT applied" << dst << mout.endl;

		//typedef drain::typeLimiter<double> Limiter;
		drain::typeLimiter<double>::value_t limiter = dst.getEncoding().getLimiter<double>(); //Type::call<Limiter>(dst.getType());
		if (!SCALE){
			while (d != dst.end()){
				*d = limiter(this->functor(static_cast<double>(*s)));
				++s;
				++d;
			}
		}
		else {
			while (d != dst.end()){
				*d = limiter(ds.inv(this->functor(ss.fwd(static_cast<double>(*s)))));
				++s;
				++d;
			}
		}
	}


	//File::write(dst, "mika.png");

}





/// Class for using two-parameter function objects (like std::functor) for sequential and spatial pixel iteration.
/**!
 *   \template F - binary (two parameter) functor
 */
template <class F>
class BinaryFunctorOp : public FunctorOp<F> //public FunctorWrapper<F>, public ChannelOp
{
public:

	/// Default constructor.
	/**
	 *  By default, all the parameters of the wrapped functor are adapted.
	 */
	//BinaryFunctorOp(bool adaptParams = false, bool adoptLimit = false) : FunctorOp<F>(adaptParams, adoptLimit) {
	BinaryFunctorOp(bool adaptParams = true, bool adoptLimit = true) : FunctorOp<F>(adaptParams, adoptLimit) { // , LIMIT(false)
	};

	virtual
	~BinaryFunctorOp(){};

	virtual
	inline
	void makeCompatible(const ImageFrame & src, Image & dst) const  {
		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL this->name+"(BinaryFunctorOp::)", __FUNCTION__);
		if (dst.isEmpty() || !dst.typeIsSet()){
			dst.initialize(src.getType(), src.getGeometry());
		}
		mout.debug() << dst << mout.endl;
		/*
		if (dstScaling.getScale() == 0.0){

		}
		*/
	}


	inline
	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL this->name+"(BinaryFunctorOp::)", __FUNCTION__);
		mout.debug(1) << "delegating to: processChannelsRepeated(src, dst)" << mout.endl;
		this->traverseChannelsRepeated(src, dst);
	}

	virtual  inline
	void traverseChannel(const Channel &src, Channel & dst) const {
		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		mout.debug(1) << "delegating unary src to binary, with src2=dst: (src, dst) => (src,dst, dst) " << mout.endl;
		traverseChannel(src, dst, dst);
	}

	virtual  inline
	void traverseChannel(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {
		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		mout.warn() << "discarding alpha channels, delegating (src, srcWeight, dst, dstWeight) to plain (src, dst) " << mout.endl;
		traverseChannel(src, dst);
	}


	inline
	virtual
	void traverseChannel(const Channel &src1, const Channel &src2, Channel & dst) const {

		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), this->name+"(BinaryFunctorOp)[ch1, ch2, chDst]", __FUNCTION__);
		mout.debug(1) << "start: " << *this << mout.endl;
		mout.debug(2) << "src1: " << src1 << mout.endl;
		mout.debug(2) << "src2: " << src2 << mout.endl;

		if ((src1.getGeometry() == src2.getGeometry()) && (src1.getGeometry() == dst.getGeometry()) ){
			traverseSequentially(src1, src2, dst);
		}
		else {
			traverseSpatially(src1, src2, dst);
		}
	}

	virtual
	void traverseSequentially(const Channel &src1, const Channel &src2, Channel & dst) const;

	virtual
	void traverseSpatially(const Channel &src1, const Channel &src2, Channel & dst) const;


	virtual
	void initializeParameters(const ImageFrame &src, const ImageFrame &dst) const {

		Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), this->name+"(BinaryFunctorOp)", __FUNCTION__);
		mout.debug() << "Unary init for Binary" << mout.endl; // << *this
		//this->functor.setDstMax(dst.getMax<double>());
		this->functor.update();

	}

	virtual
	void initializeParameters(const ImageFrame &src, const ImageFrame &src2, const ImageFrame &dst) const {

		//Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		//mout.warn() << "Binary init for Binary" << *this << mout.endl;
		//this->functor.setDstMax(dst.getMax<double>());
		this->functor.update();

	}

};


template <class T>
void BinaryFunctorOp<T>::traverseSequentially(const Channel &src1, const Channel &src2, Channel & dst) const {


	Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL getImgLog(), this->name+"(BinaryFunctorOp)", std::string(__FUNCTION__)+"(src,src2,DST)");
	mout.debug() << "start, " << this->functor << mout.endl;
	//mout.warn() << this->functor.scaleFinal << mout.endl;

	const ImageScaling  & s1s = src1.getScaling();
	const ImageScaling  & s2s = src2.getScaling();
	const ImageScaling  & ds  = dst.getScaling();
	const bool SCALE = s1s.isScaled() || s2s.isScaled() || ds.isScaled();
	mout.debug() << "SCALE::" << (int)SCALE << mout.endl;
	if (SCALE){
		mout.debug() << "s1 scale:" << s1s << mout.endl;
		mout.debug() << "s2 scale:" << s2s << mout.endl;
		mout.debug() << "d  scale:" <<  ds << mout.endl;
	}

	Image::const_iterator s1 = src1.begin();
	Image::const_iterator s2 = src2.begin();
	Image::iterator d = dst.begin();

	if (!this->LIMIT){
		mout.debug() << "LIMIT=False" << mout.endl;
		if (!SCALE){
			/// Unlimited, unscaled, ie. "direct". This option is reasonable if dst is double, for example.
			while (d != dst.end()){
				*d = this->functor(*s1, *s2);
				++s1, ++s2;
				++d;
			}
		}
		else {
			/// Unlimited but scaled.
			while (d != dst.end()){
				*d = ds.inv(this->functor(s1s.fwd(*s1), s2s.fwd(*s2)));
				++s1, ++s2;
				++d;
			}
		}
	}
	else {
		mout.debug() << "LIMIT=True" << mout.endl;
		if (!drain::Type::call<drain::typeIsInteger>(dst.getType()))
			mout.warn() << "float dst type, but LIMIT applied" << dst << mout.endl;
		//typedef drain::typeLimiter<double> Limiter;
		drain::typeLimiter<double>::value_t limit = dst.getEncoding().getLimiter<double>(); //Type::call<Limiter>(dst.getType());
		if (!SCALE){
			while (d != dst.end()){
				*d = limit(this->functor(*s1, *s2));
				++s1, ++s2;
				++d;
			}
		}
		else {
			while (d != dst.end()){
				*d = limit(ds.inv(this->functor(s1s.fwd(*s1), s2s.fwd(*s2))));
				++s1, ++s2;
				++d;
			}
		}
	}

}

template <class T>
void BinaryFunctorOp<T>::traverseSpatially(const Channel &src1, const Channel &src2, Channel & dst) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL this->name, std::string(__FUNCTION__)+"( src,src2, dst)");
	mout.debug() << "start" << mout.endl;

	// // const double coeff1    = src1.getScaling().getScale();
	const size_t width1    = src1.getWidth();
	const size_t height1   = src1.getHeight();
	CoordinateHandler2D handler1(width1, height1, src1.getCoordinatePolicy());

	// // const double coeff2    = src2.getScaling().getScale();
	const size_t width2    = src2.getWidth();
	const size_t height2   = src2.getHeight();
	CoordinateHandler2D handler2(width2, height2, src2.getCoordinatePolicy());

	mout.debug(1) << handler1 << mout.endl;
	mout.debug(1) << handler2 << mout.endl;

	const size_t width    = dst.getWidth();   //std::max(width1, width2);
	const size_t height   = dst.getHeight(); // std::max(height2, height2);
	// // const double scaleDst = dst.getScaling().getScale();
	// // if (scaleDst == 0.0) 			mout.warn() << "zero scaling coeff for dst" << mout.endl;
	// // const double coeffDst = (scaleDst != 0.0) ? 1.0/scaleDst : dst.getScaling().getMax<double>();
	const bool SCALE = src1.getScaling().isScaled() || src2.getScaling().isScaled() || dst.getScaling().isScaled();


	Point2D<int> p1;
	Point2D<int> p2;

	if (!this->LIMIT){
		mout.debug() << "LIMIT=false" << mout.endl;
		if (!SCALE){
			mout.debug() << "SCALE=false" << mout.endl;
			for (size_t i = 0; i < width; ++i) {
				for (size_t j = 0; j < height; ++j) {
					handler1.handle( p1.setLocation(i,j) );
					handler2.handle( p2.setLocation(i,j) );
					dst.put(i,j, this->functor(src1.get<double>(p1), src2.get<double>(p2)));
				}
			}
		}
		else {
			mout.debug() << "SCALE=true" << mout.endl;
			for (size_t i = 0; i < width; ++i) {
				for (size_t j = 0; j < height; ++j) {
					handler1.handle( p1.setLocation(i,j) );
					handler2.handle( p2.setLocation(i,j) );
					dst.put(i,j, dst.getScaling().inv(this->functor(
							src1.getScaling().fwd(src1.get<double>(p1)),
							src2.getScaling().fwd(src2.get<double>(p2))
					)));
					//dst.put(i,j, dst.getScaling().fwd(coeffDst * this->functor(coeff1*src1.get<double>(p1), coeff2*src2.get<double>(p2))));
				}
			}
		}
	}
	else { // LIMITED
		mout.debug() << "LIMIT=true" << mout.endl;
		if (!drain::Type::call<drain::typeIsInteger>(dst.getType()))
			mout.warn() << "float dst type, but LIMIT applied" << dst << mout.endl;
		//typedef drain::typeLimiter<double> Limiter;
		//Limiter::value_t limiterPtr = Type::call<Limiter>(dst.getType());
		drain::typeLimiter<double>::value_t limit = dst.getEncoding().getLimiter<double>();

		if (!SCALE){
			mout.debug() << "SCALE=false" << mout.endl;
			for (size_t i = 0; i < width; ++i) {
				for (size_t j = 0; j < height; ++j) {
					handler1.handle( p1.setLocation(i,j) );
					handler2.handle( p2.setLocation(i,j) );
					dst.put(i,j, limit(this->functor(src1.get<double>(p1), src2.get<double>(p2))));
				}
			}
		}
		else {
			mout.debug() << "SCALE=true" << mout.endl;
			//mout.warn() <<  << mout.endl;
			for (size_t i = 0; i < width; ++i) {
				for (size_t j = 0; j < height; ++j) {
					handler1.handle( p1.setLocation(i,j) );
					handler2.handle( p2.setLocation(i,j) );
					dst.put(i,j, limit(dst.getScaling().inv(this->functor(
							src1.getScaling().fwd(src1.get<double>(p1)),
							src2.getScaling().fwd(src2.get<double>(p2))
					))));
					//dst.put(i,j, dst.getScaling().fwd(coeffDst * this->functor(coeff1*src1.get<double>(p1), coeff2*src2.get<double>(p2))));
				}
			}
		}
	}

	/*
		if (this->LIMIT){
			for (size_t i = 0; i < width; ++i) {
				for (size_t j = 0; j < height; ++j) {
					handler1.handle( p1.setLocation(i,j) );
					handler2.handle( p2.setLocation(i,j) );
					dst.put(i,j, dst.getScaling().limit<double>(coeffDst * this->functor(coeff1*src1.get<double>(p1), coeff2*src2.get<double>(p2))));
				}
			}
		}
		else {
			for (size_t i = 0; i < width; ++i) {
				for (size_t j = 0; j < height; ++j) {
					handler1.handle( p1.setLocation(i,j) );
					handler2.handle( p2.setLocation(i,j) );
					dst.put(i,j, coeffDst * this->functor(coeff1*src1.get<double>(p1), coeff2*src2.get<double>(p2)));
				}
			}
		}
	 */

}


}

}

#endif /*FunctorOP_H_*/

// Drain
