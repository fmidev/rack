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


#include "ImageOp.h"
#include "util/Functor.h"
//#include "SequentialOp.h" TODO?
//#include "SequentialImageOp.h"

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
 */
template <class F>
class FunctorOp : public FunctorWrapper<F>, public ImageOp
{

public:

	inline
	void adaptParameters(bool adaptLimit = false){
		//getPa
		this->getParameters().append(this->functor.getParameters());
		if (adaptLimit){
			if (!this->getParameters().empty())
				this->getParameters().reference("LIMIT", this->LIMIT);
		}
	}

	bool LIMIT;

protected:

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


	/// Process the image.
	/**
	 *   In future versions there may dedicated processing for integral and floatimg-point base types.
	 *   (If some of images are double, then
	 */
	inline
	virtual
	void traverse(const Image &src, Image & dst) const {

		MonitorSource mout(iMonitor, this->name+"(FunctorOp)", __FUNCTION__);
		mout.debug() << "start" << mout.endl;

		const double coeff  = src.getScale();
		//const double dstMax = dst.getMax<double>();
		const double scaleDst = dst.getScale();
		const double coeffDst = (scaleDst != 0.0) ? 1.0/scaleDst : dst.getMax<double>();
		if (scaleDst == 0.0)
			mout.warn() << "zero scaling coeff for dst" << mout.endl;

		Image::const_iterator s  = src.begin(); // FIXME  separate const_iterator and iterator
		Image::iterator d = dst.begin();
		if (coeff == 1.0){
			if (this->LIMIT){
				while (d != dst.end()){
					*d = dst.limit<double>(coeffDst * this->functor(static_cast<double>(*s)));
					++s;
					++d;
				}
			}
			else {
				while (d != dst.end()){
					*d = coeffDst * this->functor(static_cast<double>(*s));
					++s;
					++d;
				}
			}
		}
		else {
			if (this->LIMIT){
				while (d != dst.end()){
					*d = dst.limit<double>(coeffDst * this->functor(coeff * static_cast<double>(*s)));
					++s;
					++d;
				}
			}
			else {
				while (d != dst.end()){
					*d = coeffDst * this->functor(coeff * static_cast<double>(*s));
					++s;
					++d;
				}
			}
		}

	}

	/// If source contains both image channels and alpha channels, only the image channels will be processed.
	/*
	 *  To apply scaling to alpha channels, call traverse(src, dst) explicitly for them.
	 */
	inline
	virtual
	void traverse(const Image &src, const Image &srcWeight, Image & dst, Image & dstWeight) const {
		traverse(src, dst);
	}

protected:



	virtual
	void initializeParameters(const Image &src, const Image &dst) const {

		MonitorSource mout(iMonitor, this->name, __FUNCTION__);
		// mout.warn() << *this << mout.endl;
		this->functor.update();

	}

};


/**!
 *   \template F - binary (two parameter) functor
 */
template <class F>
class BinaryFunctorOp : public FunctorOp<F> //public FunctorWrapper<F>, public ImageOp
{
public:

	/// Default constructor.
	/**
	 *  By default, all the parameters of the wrapped functor are adapted.
	 */
	//BinaryFunctorOp(bool adaptParams = false, bool adoptLimit = false) : FunctorOp<F>(adaptParams, adoptLimit) {
	BinaryFunctorOp(bool adaptParams = true, bool adoptLimit = true) : FunctorOp<F>(adaptParams, adoptLimit) {
	//BinaryFunctorOp(bool adaptParams = false) : FunctorWrapper<F>(), ImageOp(this->functor.name, this->functor.description), LIMIT(false) {
		//if (adaptParams)	adaptParameters(adoptLimit);
	};

	virtual
	inline
	void filter(const Image &src, Image &dst) const {
		filter(src, dst, dst); // below
	};

	virtual
	inline
	void filter(const Image & src1, const Image & src2, Image &dst) const {
		ImageOp::filter(src1, src2, dst);
	};


	//virtual ~FunctorOp(){};
	inline
	virtual
	void traverse(const Image &src, Image & dst) const {
		drain::MonitorSource mout(iMonitor, this->name, __FUNCTION__);
		mout.debug() << "delegating unary src to binary, with src2=dst: (src, dst) => (src,dst, dst) " << mout.endl;
		traverse(src, dst, dst);
	}

	inline
	virtual
	void traverse(const Image &src1, const Image &src2, Image & dst) const {

		drain::MonitorSource mout(iMonitor, this->name+"(BinaryFunctorOp)", __FUNCTION__);

		mout.debug(2) << "Running: " << *this << mout.endl;
		//mout.warn() << "src1: " << src1 << mout.endl;
		//mout.warn() << "src2: " << src2 << mout.endl;

		if ((src1.getGeometry() == src2.getGeometry()) && (src1.getGeometry() == dst.getGeometry()) ){
			traverseSequentially(src1, src2, dst);
		}
		else {
			//mout.warn() << "Not implemented: " << *this << mout.endl;
			traverseSpatially(src1, src2, dst);
		}
	}

	inline
	virtual
	void traverseSequentially(const Image &src1, const Image &src2, Image & dst) const {


		MonitorSource mout(this->name, std::string(__FUNCTION__)+"(src,src2, dst)");
		mout.debug() << "start" << mout.endl;

		const double coeff1 = src1.getScale();
		const double coeff2 = src2.getScale();
		//const double dstMax = dst.getMax<double>();
		const double scaleDst = dst.getScale();
		const double coeffDst = (scaleDst != 0.0) ? 1.0/scaleDst : dst.getMax<double>();
		if (scaleDst == 0.0)
			mout.warn() << "zero scaling coeff for dst" << mout.endl;


		Image::const_iterator s1 = src1.begin(); // FIXME  separate const_iterator and iterator
		Image::const_iterator s2 = src2.begin();
		Image::iterator d = dst.begin();



		if ((coeff1 == 1.0) && (coeff2 == 1.0)){
			if (this->LIMIT){
				while (d != dst.end()){
					*d = dst.limit<double>(coeffDst * this->functor(static_cast<double>(*s1), static_cast<double>(*s2)));
					++s1;
					++s2;
					++d;
				}
			}
			else {
				while (d != dst.end()){
					*d = coeffDst * this->functor(static_cast<double>(*s1), static_cast<double>(*s2));
					++s1;
					++s2;
					++d;
				}
			}

		}
		else {
			if (this->LIMIT){
				while (d != dst.end()){
					*d = dst.limit<double>(coeffDst * this->functor(coeff1*static_cast<double>(*s1), coeff2*static_cast<double>(*s2)));
					++s1;
					++s2;
					++d;
				}
			}
			else {
				while (d != dst.end()){
					*d = coeffDst * this->functor(coeff1*static_cast<double>(*s1), coeff2*static_cast<double>(*s2));
					++s1;
					++s2;
					++d;
				}
			}

		}

	}

	inline
	virtual
	void traverseSpatially(const Image &src1, const Image &src2, Image & dst) const {

		MonitorSource mout(this->name, std::string(__FUNCTION__)+"(src,src2, dst)");
		mout.debug() << "start" << mout.endl;

		const double coeff1    = src1.getScale();
		const size_t width1    = src1.getWidth();
		const size_t height1   = src1.getHeight();
		CoordinateHandler2D handler1(width1, height1, src1.getCoordinatePolicy());

		const double coeff2    = src2.getScale();
		const size_t width2    = src2.getWidth();
		const size_t height2   = src2.getHeight();
		CoordinateHandler2D handler2(width2, height2, src2.getCoordinatePolicy());


		// const size_t width    = std::max(width1, width2);
		// const size_t height   = std::max(height2, height2);
		const size_t width    = dst.getWidth();   //std::max(width1, width2);
		const size_t height   = dst.getHeight(); // std::max(height2, height2);
		const double scaleDst = dst.getScale();
		const double coeffDst = (scaleDst != 0.0) ? 1.0/scaleDst : dst.getMax<double>();
		if (scaleDst == 0.0)
			mout.warn() << "zero scaling coeff for dst" << mout.endl;

		Point2D<int> p1;
		Point2D<int> p2;

		//for (size_t k = 0; k < channels; ++k) {
		//	size_t k1 = k % channels1;
		//	size_t k2 = k % channels2;
		if (this->LIMIT){
			for (size_t i = 0; i < width; ++i) {
				for (size_t j = 0; j < height; ++j) {
					handler1.handle( p1.setLocation(i,j) );
					handler2.handle( p2.setLocation(i,j) );
					dst.put(i,j, dst.limit<double>(coeffDst * this->functor(coeff1*src1.get<double>(p1), coeff2*src2.get<double>(p2))));
				}
			}
		}
		else {
			for (size_t i = 0; i < width; ++i) {
				for (size_t j = 0; j < height; ++j) {
					handler1.handle( p1.setLocation(i,j) );
					handler2.handle( p2.setLocation(i,j) );
					dst.put(i,j, coeffDst * this->functor(coeff1*src1.get<double>(p1), coeff2*src2.get<double>(p2)));
					//filterValueD(src.get<double>(p1.x, p1.y, k1), src2.get<double>(p2.x, p2.y, k2)));
					//dst.put(i,j, filterValueD(src.get<double>(p1.x, p1.y, k1), src2.get<double>(p2.x, p2.y, k2)));
				}
			}
		}

		//}
	}


	bool LIMIT;

protected:


	virtual
	void initializeParameters(const Image &src, const Image &dst) const {

		MonitorSource mout(iMonitor, this->name, __FUNCTION__);
		mout.debug() << "Unary init for Binary" << mout.endl; // << *this
		//this->functor.setDstMax(dst.getMax<double>());
		this->functor.update();

	}

	virtual
	void initializeParameters(const Image &src, const Image &src2, const Image &dst) const {

		//MonitorSource mout(iMonitor, this->name, __FUNCTION__);
		//mout.warn() << "Binary init for Binary" << *this << mout.endl;
		//this->functor.setDstMax(dst.getMax<double>());
		this->functor.update();

	}

};



//MAX_16B(0xffff)

}

}

#endif /*FunctorOP_H_*/

// Drain
