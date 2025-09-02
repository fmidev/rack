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


#include <drain/image/ImageFile.h>
#include <drain/TypeUtils.h>
#include "drain/util/Functor.h"
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

	FunctorWrapper(){};

	FunctorWrapper(const FunctorWrapper<F> & wrapper) : functor(wrapper.functor) {
		//functor.getParameters().copyStruct(op.functor.getParameters(), op.functor, functor);
	};

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

	FunctorOp(const FunctorOp & op) : ImageOp(op), LIMIT(op.LIMIT){
		this->getParameters().copyStruct(op.getParameters(), op, *this);  // will add LIMIT (only)
	};

	virtual ~FunctorOp(){};

	inline
	void adaptParameters(bool adaptLimit = false){
		ReferenceMap & p = this->getParameters();
		p.append(this->functor.getParameters());
		if (adaptLimit){
			if (!p.empty()) // ?
				p.link("LIMIT", this->LIMIT);
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



};

/// Class for using simple function objects (like std::functor) for sequential pixel iteration.
/**!
 *   \template F - unary (single-parameter) functor
 *   \template NORM - normalize to 0...1 (unsigned) and -1...1 (signed)
 *   \template SIGN - select signed, else unsigned
 */
template <class F,bool NORM=false,bool SIGN=false>
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

	inline
	UnaryFunctorOp(const UnaryFunctorOp<F> & op) : FunctorOp<F>(false, false){
		this->parameters.copyStruct(op.parameters, op, *this);
	};

	virtual ~UnaryFunctorOp(){};

	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {  //  = 0;
		//drain::Logger mout(this->getName()+"(UnaryFunctorOp)", __FUNCTION__);
		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		mout.debug2("invoking processChannelsSeparately()" );
		ImageOp::traverseChannelsSeparately(src, dst);
	}

	/// Process the image.
	/**
	 *   In future versions there may dedicated processing for integral and floatimg-point base types.
	 *   (If some of images are double, then
	 */
	void traverseChannel(const Channel &src, Channel & dst) const;


protected:

	void getDstConf(const ImageConf & src, ImageConf & dst) const {

		// OLD
		/*
		if ((dst.getScale()==0.0) || !dst.typeIsSet())
				dst.setEncoding(src.getEncoding());
		*/

		// New 2024, after bugs with:--physicalRange 0:1 for --iSegmentArea --iRemap --iThreshold
		if (!dst.typeIsSet()){
			dst.setEncoding(src.getEncoding());
		}
		else if (src.isScaled() && !dst.isScaled()){
			//dst.setEncoding(src.getEncoding());
			if (dst.getType() == src.getType()){
				dst.setScaling(src.getScaling());
			}
			else {
				dst.setPhysicalRange(src.getPhysicalRange());
			}
		}


		// TODO: check if int, and unsigned, and minValue
		if (SIGN && (dst.getMinPhys()>=0.0)){ // bug? why not leq
			Logger mout(getImgLog(), __FILE__, __FUNCTION__);
			mout.warn(this->functor.getName() , " would need signed type instead of : " , dst.getEncoding() );
		}

		if (NORM){
			if (SIGN)
				dst.setPhysicalRange(-1.0, +1.0, true);
			else
				dst.setPhysicalRange( 0.0, +1.0, true);
		}

		dst.setGeometry(src.getGeometry()); //geometry = src.geometry;

		dst.setCoordinatePolicy(src.getCoordinatePolicy());

	}


	virtual
	void initializeParameters(const ImageFrame &src, const ImageFrame &dst) const {

		Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		// mout.warn(*this );
		this->functor.updateBean();

	}

};

template <class T,bool NORM,bool SIGN>
void UnaryFunctorOp<T,NORM,SIGN>::traverseChannel(const Channel &src, Channel & dst) const {

	//Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), this->name+"(UnaryFunctorOp)", __FUNCTION__);
	Logger mout(getImgLog(), __FILE__, __FUNCTION__);
	mout.debug("start" );

	const drain::ValueScaling  & ss = src.getScaling();
	const drain::ValueScaling  & ds = dst.getScaling();
	const bool SCALE = ss.isScaled() || ds.isScaled();
	mout.debug("SCALE=" , (int)SCALE );
	mout.debug("functor=" , this->functor );

	if (mout.isDebug(2)){
		for (int i = 0; i < 256; i+=8) {
			std::cout <<  __FILE__ << ':' << __FUNCTION__ << ':' << i << "\t -> " << ss.fwd(i) << "\t" << this->functor(ss.fwd(i)) << '\n';
		}
	}

	if (SCALE){
		mout.debug2("ss scale:" , ss );
		mout.debug2("ds scale:" , ds );
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
			mout.warn("float dst type, but LIMIT applied" , dst );

		//typedef drain::typeLimiter<double> Limiter;
		drain::typeLimiter<double>::value_t limiter = dst.getLimiter<double>(); //Type::call<Limiter>(dst.getType());
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

	BinaryFunctorOp(const BinaryFunctorOp & op) : FunctorOp<F>(false, false){
		this->parameters.copyStruct(op.parameters, op, *this);
	};

	virtual
	~BinaryFunctorOp(){};



	inline
	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL this->name+"(BinaryFunctorOp::)", __FUNCTION__);
		mout.debug2("delegating to: processChannelsRepeated(src, dst)" );
		this->traverseChannelsRepeated(src, dst);
	}

	virtual  inline
	void traverseChannel(const Channel &src, Channel & dst) const {
		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		mout.debug2("delegating unary src to binary, with src2=dst: (src, dst) => (src,dst, dst) " );
		traverseChannel(src, dst, dst);
	}

	/*
	virtual  inline
	void traverseChannel(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {
		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		mout.warn("discarding alpha channels, delegating (src, srcWeight, dst, dstWeight) to plain (src, dst) " );
		traverseChannel(src, dst);
	}
	*/



	inline
	virtual
	void traverseChannel(const Channel &src1, const Channel &src2, Channel & dst) const {

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), this->name+"(BinaryFunctorOp)[ch1, ch2, chDst]", __FUNCTION__);
		mout.debug2("start: " , *this );
		mout.debug3("src1: " , src1 );
		mout.debug3("src2: " , src2 );

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

		Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), this->name+"(BinaryFunctorOp)", __FUNCTION__);
		mout.debug("Unary init for Binary" ); // << *this
		//this->functor.setDstMax(dst.getMax<double>());
		this->functor.updateBean();

	}

	virtual
	void initializeParameters(const ImageFrame &src, const ImageFrame &src2, const ImageFrame &dst) const {

		//Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		//mout.warn("Binary init for Binary" , *this );
		//this->functor.setDstMax(dst.getMax<double>());
		this->functor.updateBean();

	}

protected:

	/// Does not change dst geometry
	void getDstConf(const ImageConf & src, ImageConf & dst) const {

		if ((dst.getScale()==0.0) || !dst.typeIsSet())
			dst.setEncoding(src.getEncoding());

		if (dst.isEmpty()){
			Logger mout(getImgLog(), __FILE__, __FUNCTION__);
			mout.warn("2nd image empty, problems ahead" , *this );
			dst.setGeometry(src.getGeometry());
			dst.setCoordinatePolicy(src.getCoordinatePolicy());
		}

	}



};


template <class T>
void BinaryFunctorOp<T>::traverseSequentially(const Channel &src1, const Channel &src2, Channel & dst) const {


	Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), this->name+"(BinaryFunctorOp)", std::string(__FUNCTION__)+"(src,src2,DST)");
	mout.debug("start, " , this->functor );
	//mout.warn(this->functor.scaleFinal );

	const drain::ValueScaling  & s1s = src1.getScaling();
	const drain::ValueScaling  & s2s = src2.getScaling();
	const drain::ValueScaling  & ds  = dst.getScaling();
	const bool SCALE = s1s.isScaled() || s2s.isScaled() || ds.isScaled();
	mout.debug("SCALE::" , (int)SCALE );
	if (SCALE){
		mout.debug("s1 scale:" , s1s );
		mout.debug("s2 scale:" , s2s );
		mout.debug("d  scale:" ,  ds );
	}

	Image::const_iterator s1 = src1.begin();
	Image::const_iterator s2 = src2.begin();
	Image::iterator d = dst.begin();

	if (!this->LIMIT){
		mout.debug("LIMIT=False" );
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
		mout.debug("LIMIT=True" );
		if (!drain::Type::call<drain::typeIsInteger>(dst.getType()))
			mout.warn("float dst type, but LIMIT applied" , dst );
		//typedef drain::typeLimiter<double> Limiter; getEncoding().
		drain::typeLimiter<double>::value_t limit = dst.getLimiter<double>(); //Type::call<Limiter>(dst.getType());
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

	Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL this->name, std::string(__FUNCTION__)+"( src,src2, dst)");
	mout.debug("start" );

	// // const double coeff1    = src1.getScaling().getScale();
	const size_t width1    = src1.getWidth();
	const size_t height1   = src1.getHeight();
	CoordinateHandler2D handler1(width1, height1, src1.getCoordinatePolicy());

	// // const double coeff2    = src2.getScaling().getScale();
	const size_t width2    = src2.getWidth();
	const size_t height2   = src2.getHeight();
	CoordinateHandler2D handler2(width2, height2, src2.getCoordinatePolicy());

	mout.debug2(handler1 );
	mout.debug2(handler2 );

	const size_t width    = dst.getWidth();   //std::max(width1, width2);
	const size_t height   = dst.getHeight(); // std::max(height2, height2);
	// // const double scaleDst = dst.getScaling().getScale();
	// // if (scaleDst == 0.0) 			mout.warn("zero scaling coeff for dst" );
	// // const double coeffDst = (scaleDst != 0.0) ? 1.0/scaleDst : dst.getScaling().getMax<double>();
	const bool SCALE = src1.getScaling().isScaled() || src2.getScaling().isScaled() || dst.getScaling().isScaled();


	Point2D<int> p1;
	Point2D<int> p2;

	if (!this->LIMIT){
		mout.debug("LIMIT=false" );
		if (!SCALE){
			mout.debug("SCALE=false" );
			for (size_t i = 0; i < width; ++i) {
				for (size_t j = 0; j < height; ++j) {
					handler1.handle( p1.setLocation(i,j) );
					handler2.handle( p2.setLocation(i,j) );
					dst.put(i,j, this->functor(src1.get<double>(p1), src2.get<double>(p2)));
				}
			}
		}
		else {
			mout.debug("SCALE=true" );
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
		mout.debug("LIMIT=true" );
		if (!drain::Type::call<drain::typeIsInteger>(dst.getType()))
			mout.warn("float dst type, but LIMIT applied" , dst );
		// typedef drain::typeLimiter<double> Limiter;
		// Limiter::value_t limiterPtr = Type::call<Limiter>(dst.getType());
		// .getEncoding()
		drain::typeLimiter<double>::value_t limit = dst.getLimiter<double>();

		if (!SCALE){
			mout.debug("SCALE=false" );
			for (size_t i = 0; i < width; ++i) {
				for (size_t j = 0; j < height; ++j) {
					handler1.handle( p1.setLocation(i,j) );
					handler2.handle( p2.setLocation(i,j) );
					dst.put(i,j, limit(this->functor(src1.get<double>(p1), src2.get<double>(p2))));
				}
			}
		}
		else {
			mout.debug("SCALE=true" );
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
