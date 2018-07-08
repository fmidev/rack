/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */
#ifndef FASTAVERAGEOP_H_
#define FASTAVERAGEOP_H_

//#include "SlidingStripeOp.h"
//#include "image/Window.h"
#include "image/SlidingWindow.h"
#include "WindowOp.h"

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

	typedef float value_t;

	inline
	SlidingStripeAverage(int size=1) : SW(size), sum(0.0), count(0) { // scaleResult(1.0) {  // consider scaleWeight
	};

	virtual
	~SlidingStripeAverage(){};

protected:

	/**
	 *   TODO: weight = > weightSUm
	 *   TODO: multiple channels (banks)?
	 */
	virtual
	void initialize(){

		Logger mout(getImgLog(), "SlidingStripeAverage", __FUNCTION__);
		this->setImageLimits();
		this->setLoopLimits();
		 // NOTE: maybe actual area varies, depending on coord handler?
		//scaleResult = this->dst.template getMax<value_t>()/this->src.template getMax<value_t>() / this->getArea();
		// scaleResult = this->src.scaling.getScale()/this->dst.scaling.getScale()/ this->getArea();
		scaling.setConversionScale(this->src.getScaling(), this->dst.getScaling());
		// scaling.setScale(scaling.getScale() * static_cast<double>(this->getArea()));  // replaced by count


		mout.debug(2) << "scaling (including area): " << scaling.toStr() << mout.endl;
		// mout.debug(2) << "scaleResult(by area)=" << scaleResult   << mout.endl;
		mout.debug(2) << this->dst.getCoordinatePolicy() << mout.endl;
		//coordinateHandler.setLimits(src.getWidth(), src.getWidth())
		//ImageOp::adaptCoordinateHandler(src, coordinateHandler);

	}

	virtual
	inline
	void clear(){
		sum = 0.0;
		count = 0;
	};

	virtual
	inline
	void addPixel(Point2D<int> & p){
		//if ((this->location.x == this->location.y) && ((this->location.x&15) == 0))
		//	std::cerr << " fastStripe=" << this->location << ", lead->" << this->locationLead<< " trail->" << this->locationTrail << "  sum=" << sum << '\n';
		if (this->coordinateHandler.validate(p)){
			sum += this->src.template get<value_t>(p);
			++count;
		}
	};

	virtual
	inline
	void removePixel(Point2D<int> & p){
		if (this->coordinateHandler.validate(p)){
			sum -= this->src.template get<value_t>(p);
			--count;
		}
	};


	virtual inline
	void write(){
		// if ((this->location.x == this->location.y) && ((this->location.x&31) == 0))
		//	std::cerr << " loc: " << this->location << ' ' << sum << '\n';
		//this->dst.put(this->location, scaling.inv(sum /static_cast<value_t>(this->getArea()) ) );
		//this->dst.put(this->location, scaling.inv(sum) ); // (area scaling included)
		this->dst.put(this->location, scaling.inv(sum / static_cast<value_t>(count)) ); // (area scaling included)

	}


	virtual	inline
	void setImageLimits() const {
		this->src.adjustCoordinateHandler(this->coordinateHandler);
	}


	value_t sum;
	unsigned int count;

	//double scaleResult;
	ImageScaling scaling;


};

/// Stripe window for computing weight averages
/**
 *  \tparam SW - SlidingStripeHorz or SlidingStripeVert
 */
template <class SW>
class SlidingStripeAverageWeighted : public SlidingStripeAverage<SW> {

public:

	typedef float sum_t;

	SlidingStripeAverageWeighted(int size=1) : SlidingStripeAverage<SW>(size), w(0.0), sumW(0.0) {}; // ,areaF(0.0)

	virtual
	~SlidingStripeAverageWeighted(){};


	/**
	 *   TODO: weight = > weightSUm
	 *   TODO: multiple channels (banks)?
	 */
	virtual
	void initialize(){

		Logger mout(getImgLog(), "SlidingStripeAverageWeighted", __FUNCTION__);

		this->setImageLimits();
		this->setLoopLimits();
		/*
		areaF = this->getArea();  // NOTE: maybe actual area varies, depending on coord handler?
		if (areaF == 0.0){
			mout.error() << "area==0 ?" << mout.endl;
		}
		*/

		// mout.debug(3) << "src.getMax=" << this->src.scaling.template getMax<value_t>() << mout.endl;
		// this->scaleResult = this->dst.template getMax<value_t>() / this->src.template getMax<value_t>();
		this->scaling.setConversionScale(this->src.getScaling(), this->dst.getScaling());
		mout.debug() << "scaling:" << this->scaling << mout.endl;

		// WEIGHT
		this->scalingW.setConversionScale(this->srcWeight.getScaling(), this->dstWeight.getScaling());
		//this->scalingW.setScale(this->scalingW.getScale() * static_cast<double>(this->getArea()));
		mout.debug() << "scalingW:" << this->scalingW << mout.endl;

		//this->SCALE = this->src.scaling.isScaled() || this->dst.scaling.isScaled(); // NEEDED?
		//mout.warn() << "areaF=" << this->areaF << mout.endl;
		//mout.warn() << "scaleResult=" << this->scaleResult << mout.endl;

	}

private:

	sum_t w;
	sum_t sumW;

	/// Scales the window area and the dst intensity.
	// double areaF;

	ImageScaling scalingW;

protected:


	virtual	inline
	void clear(){
		this->sum = 0.0;
		this->sumW = 0.0;
		this->count = 0;
	};

	virtual	inline
	void addPixel(Point2D<int> & p){
		if (this->coordinateHandler.validate(p)){
			this->w     = this->srcWeight.template get<sum_t>(p);
			this->sum  += w*this->src.template get<sum_t>(p);
			this->sumW += w;
			++this->count;
		}
	};

	virtual inline
	void removePixel(Point2D<int> & p){
		if (this->coordinateHandler.validate(p)){
			this->w     = this->srcWeight.template get<sum_t>(p);
			this->sum  -= w*this->src.template get<sum_t>(p);
			this->sumW -= w;
			--this->count;
		}
	};

	virtual inline
	void write(){
		//if (location.x == location.y)			std::cerr << " loc: " << location.x << ' ' << sumW << '\n';
		if (sumW > 0.0)
			this->dst.put(this->location, this->scaling.inv(this->sum/sumW));
			//this->dst.put(this->location, this->scaleResult*this->sum/sumW);
		else
			this->dst.put(this->location, 0);
		//this->dstWeight.put(this->location, this->scaleResult*sumW/areaF);
		//this->dstWeight.put(this->location, this->scalingW.inv(sumW)); // including areaF
		this->dstWeight.put(this->location, this->scalingW.inv(sumW/static_cast<sum_t>(this->count))); // including areaF

	}

};


/// Sliding window averaging operator with optional weighting support.
// TODO: Generalize to SeparableWindowOp !
/**

  Implemented as two consequent SlidingStripeOp's: horizontal filtering followed by a vertical one.

 \code
  drainage shapes.png --average 25   -o average.png
  drainage shapes.png --average 5,55 -o average-vert.png
 \endcode

  If the source image contains several channels, each channel is treated separately.

 \code
  drainage shapes.png --average 25  -o average-rgb.png
 \endcode

 \~exec
   make image-rgba.png
 \~

 This operator applies weighted averaging, if a weight image (alpha channel) is provided.
 \code
  drainage image-rgba.png --average 15 -o average-weighted.png
 \endcode

  \see BlenderOp
 */
class FastAverageOp : public WindowOp< Window<WindowConfig> > {

public:

	FastAverageOp(int width=1, int height=0) : WindowOp<Window<WindowConfig> >(__FUNCTION__, "Window averaging with optional weighting support.", width, height) { //, 1, 2, false) {

	};



	/// Delegates the invocation separately for each channel.
	/**
	 *   If alpha channels are present, delegates to void process(const ImageFrame &src,const ImageFrame &srcWeight, Image &dst,Image &dstWeight) const
	 */
	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		//mout.error() << "not implemented; consider forwarding to processChannelsSeparately()" << mout.endl;
		traverseChannelsSeparately(src, dst);
	}


	/// Main operation, for plain (unweighted) image data.
	virtual
	void traverseChannel(const Channel & src, Channel & dst) const;

	/// Main operation, for weighted image data.
	virtual
	void traverseChannel(const Channel &src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const;


protected:

	virtual inline
	bool processOverlappingWithTemp(const ImageTray<const Channel> & src, ImageTray<Image> & dst) const {
		return false;
	}

};



}  // image::

}  // drain::

#endif
