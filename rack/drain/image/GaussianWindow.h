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
#ifndef GaussianWindow_H_
#define GaussianWindow_H_

#include "Window.h"
//#include "WindowOp.h"
//#include "SlidingWindow.h"

namespace drain
{

namespace image
{

/// Extends WindowConfig with \c radius
class GaussianWindowConf : public WindowConfig {

public:

	double radius;

};

///
/**
 *  \tparam DIR - direction: true=horzizontal, false=vertical
 *  \tparam R   - Core for window input & output images
 */
template <bool DIR=true, class R=WindowCore>
class GaussianStripe : public Window<GaussianWindowConf,R> { // consider scaleWeight

public:

	typedef float value_t;

	GaussianStripe(int n, double radius = 1.0) : Window<GaussianWindowConf,R>(DIR?n:1, DIR?1:n), weightSum(0), value(0), scaleResult(1.0) {
		this->conf.radius = radius;
		w = 0.0;
		sumW = 0.0;
	};

	virtual
	~GaussianStripe(){};

	virtual inline
	void setImageLimits() const {
		this->coordinateHandler.set(this->src.getConf());
		//this->src.adjustCoordinateHandler(this->coordinateHandler);
	}


	void initialize(){

		drain::Logger mout(getImgLog(), "GaussianStripe", __FUNCTION__);

		this->setImageLimits();
		this->setLoopLimits();

		//const int n = this->conf.width*this->conf.height; // width*1 or 1*height
		// (DIR?this->conf.width:this->conf.height);
		const int n    = std::max(this->conf.frame.width, this->conf.frame.height); // width*1 or 1*height
		const int bias = std::min(this->iRange.min, this->jRange.min);
		lookUp.resize(n);
		//weightSum = 0.0;
		value_t f;
		const double radiusAbs  = this->conf.radius * static_cast<value_t>(n)*0.5;
		const double radiusAbs2 = radiusAbs*radiusAbs;
		if (radiusAbs <= 0.0){
			mout.error() << "Zero radius2: " << radiusAbs << mout.endl;
		}

		int iNorm;
		mout.debug() << "frame:" << this->conf.frame << mout.endl;
		for (int i = 0; i < n; ++i) {
			iNorm = (i + bias);
			f = exp2(-static_cast<double>(iNorm*iNorm) / radiusAbs2);
			lookUp[i]  = f;
			weightSum += f;
			mout.debug() << i << '\t' << iNorm << '\t' << f << '\t' << weightSum << mout.endl;
		}
		mout.debug() << "weightSum = " << weightSum << mout.endl;

		scaleResult = this->src.getScaling().getScale() / this->dst.getScaling().getScale();
		mout.debug() << "scale = " << scaleResult << mout.endl;

		//this->coordinateHandler.setLimits(srcWidth, srcHeight);
		//this->coordinateHandler.setLimits(this->src.getWidth(), this->src.getHeight());

	}


	// Copied from sliding window
	void setSize(size_t width = 1){
		this->Window<GaussianWindowConf,R>::setSize(width, 1);
	}

protected:

	virtual
	void setSize(size_t width, size_t height){
		drain::Logger mout(getImgLog(), "SlidingStripe", __FUNCTION__);
		if (height > 1)
			mout.warn() << "horz stripe, height(" << height << ") discarded" << mout.endl;
		Window<GaussianWindowConf,R>::setSize(width, 1);
	}


protected:

	std::vector<value_t> lookUp;


	virtual
	void write() {
		this->dst.put(this->location, value);
	};

protected:

	// Truncated sum of the gaussian weights (typically close to 1.0)
	value_t weightSum;

	mutable
	value_t value;

	value_t scaleResult;
	// value_t scaleResultWeight;  // consider!

	virtual
	void update(); // = 0;

	mutable
	Point2D<int> locationTmp;

	// Weighted version only:
	/// Current weight
	value_t w;
	/// Current weighted sum
	value_t sumW;

};


///
/**
 *  \tparam DIR - direction: true=horzizontal, false=vertical
 *  \tparam R   - Window input & output images
 */
template <bool DIR=true>
class GaussianStripeWeighted : public GaussianStripe<DIR, WeightedWindowCore> {

public:

	GaussianStripeWeighted(int n, double radius=1.0) : GaussianStripe<DIR, WeightedWindowCore>(n, radius) {
	};

protected:

	virtual
	inline
	void write(){
		this->dst.put(this->location, this->value);
		this->dstWeight.put(this->location, this->weightSum);
	};


};


}  // image::

}  // drain::

#endif

// Drain
