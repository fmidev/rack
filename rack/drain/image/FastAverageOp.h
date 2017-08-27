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
#ifndef FASTAVERAGEOP_H_
#define FASTAVERAGEOP_H_

//#include "SlidingStripeOp.h"
#include "WindowOp.h"
#include "SlidingWindow.h"

namespace drain
{

namespace image
{

/// Stripe window for computing averages
/**
 *  \tparam SW - SlidingStripeHorz or SlidingStripeVert
 */
template <class SW>
class SlidingStripeAverage : public SW {

public:

	inline
	SlidingStripeAverage(int size=1) : SW(size), sum(0.0), scaleResult(1.0) {  // consider scaleWeight
	};


protected:

	/**
	 *   TODO: weight = > weightSUm
	 *   TODO: multiple channels (banks)?
	 */
	virtual
	void initialize(){

		MonitorSource mout(iMonitor, "SlidingStripeAverage", __FUNCTION__);
		 // NOTE: maybe actual area varies, depending on coord handler?
		//scaleResult = this->dst.template getMax<float>()/this->src.template getMax<float>() / this->getArea();
		scaleResult = this->src.getScale()/this->dst.getScale()/ this->getArea();

		mout.debug(2) << "scaleResult(by area)=" << scaleResult   << mout.endl;
		mout.debug(2) << this->dst.getCoordinatePolicy() << mout.endl;
		//coordinateHandler.setLimits(src.getWidth(), src.getWidth())
		//ImageOp::adaptCoordinateHandler(src, coordinateHandler);

	}

	virtual
	inline
	void clear(){
		sum = 0.0;
	};

	virtual
	inline
	void addPixel(Point2D<int> & p){
		//if ((location.x == location.y) && ((location.x&15) == 0))
		//	std::cerr << " fastStripe=" << location << " lead->" << locationHandled << "  sum=" << sum << '\n';
		if (this->coordinateHandler.validate(p))
			sum += this->src.template get<float>(p);
	};

	virtual
	inline
	void removePixel(Point2D<int> & p){
		if (this->coordinateHandler.validate(p))
			sum -= this->src.template get<float>(p);
	};


	virtual
	inline
	void write(){
		//if ((location.x == location.y) && ((location.x&31) == 0))
		//	std::cerr << " loc: " << location << ' ' << sum << '\n';
		this->dst.put(this->location, scaleResult * sum);
	}




	double sum;

	double scaleResult;

private:


};

/// Stripe window for computing weight averages
/**
 *  \tparam SW - SlidingStripeHorz or SlidingStripeVert
 */
template <class SW>
class SlidingStripeAverageWeighted : public SlidingStripeAverage<SW> {

public:


	SlidingStripeAverageWeighted(int size=1) : SlidingStripeAverage<SW>(size) {};

	/**
	 *   TODO: weight = > weightSUm
	 *   TODO: multiple channels (banks)?
	 */
	virtual
	void initialize(){

		MonitorSource mout(iMonitor, "SlidingStripeAverageWeighted", __FUNCTION__);

		areaF = this->getArea();  // NOTE: maybe actual area varies, depending on coord handler?
		if (areaF == 0.0)
			mout.warn() << "area==0 ?" << mout.endl;

		mout.debug(3) << "src.getMax=" << this->src.template getMax<float>() << mout.endl;
		this->scaleResult = this->dst.template getMax<float>() / this->src.template getMax<float>();
		//mout.warn() << "areaF=" << this->areaF << mout.endl;
		//mout.warn() << "scaleResult=" << this->scaleResult << mout.endl;

	}

private:

	float w;
	double sumW;

	/// Scales the window area and the dst intensity.
	double areaF;

protected:

	virtual
	inline
	void clear(){
		this->sum = 0.0;
		this->sumW = 0.0;
	};

	virtual
	inline
	void addPixel(Point2D<int> & p){
		if (this->coordinateHandler.validate(p)){
			this->w     = this->srcWeight.template get<float>(p);
			this->sum  += w*this->src.template get<float>(p);
			this->sumW += w;
		}
	};

	virtual
	inline
	void removePixel(Point2D<int> & p){
		if (this->coordinateHandler.validate(p)){
			this->w     = this->srcWeight.template get<float>(p);
			this->sum  -= w*this->src.template get<float>(p);
			this->sumW -= w;
		}
	};

	virtual
	inline
	void write(){
		//if (location.x == location.y)			std::cerr << " loc: " << location.x << ' ' << sumW << '\n';
		if (sumW > 0.0)
			this->dst.put(this->location, this->scaleResult*this->sum/sumW);
		else
			this->dst.put(this->location, 0);
		this->dstWeight.put(this->location, this->scaleResult*sumW/areaF);
		//dst.at(location) =  128;
	}

};



/// Sliding window averaging operator with optional weighting support.
/**

  Implemented as two consequent SlidingStripeOp's: horizontal filtering followed by a vertical one.

 \code
  drainage gray.png --average 21   -o average.png
  drainage gray.png --average 5,51 -o average-vert.png
 \endcode

  If the source image contains several channels, each channel is treated separately.

 \code
  drainage image.png --average 21   -o average-rgb.png
 \endcode

\~exec
make image-rgba.png
\~

 This operator applies weighted averaging, if a weight image (alpha channel) is provided.
 \code
  drainage image-rgba.png --average 15 -o average-weighted.png
 \endcode

  \see Recursive
 */
class FastAverageOp : public WindowOp
{
public:

	FastAverageOp(int width=1, int height=0) : WindowOp(__FUNCTION__, "Window averaging with optional weighting support.") { //, 1, 2, false) {
		this->width  = width;
		this->height = height;
	};

	/// Top-level call that repeates the call to each image channel.
	/**
	 *   If alpha channels are present, delegates to void filter(const Image &src,const Image &srcWeight, Image &dst,Image &dstWeight) const
	 */
	//virtual
	//void filter(const Image &src, Image &dst) const;
	inline
	virtual
	void initializeParameters(const Image &src, const Image &dst) const {
		//dst.setScale(src.getScale());
	}


	/// Calls SlidingStripeOp<T,T2>::filter() separately for each image channel. This is natural for many operations, such as averaging.
	virtual
	void traverse(const Image &src, Image &dst) const;

	/// Top-level call that repeates the call to each channel.
	//virtual
	//void filter(const Image &src,const Image &srcWeight, Image &dst,Image &dstWeight) const;
	virtual
	void traverse(const Image &src, const Image & srcWeight, Image &dst, Image &dstWeight) const;


protected:

	/// Uses own tmp image, hence returns false.
	virtual
	bool filterWithTmp(const Image & src, Image & dst) const {
		return false;
	}


	/// Uses own tmp image, hence returns false.
	virtual
	bool filterWithTmp(const Image & src, const Image & src2, Image & dst) const {
		return false;
	}


};



}  // image::

}  // drain::

#endif

// Drain
