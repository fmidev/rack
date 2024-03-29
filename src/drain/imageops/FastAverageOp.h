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
//#include "drain/image/Window.h"
#include "drain/image/SlidingWindow.h"
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


		mout.debug3("scaling (including area): " , scaling.toStr() );
		// mout.debug3("scaleResult(by area)=" , scaleResult   );
		mout.debug3(this->dst.getCoordinatePolicy() );
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
		Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		mout.debug(this->src );
		this->coordinateHandler.set(this->src.getGeometry(), this->src.getCoordinatePolicy());
		mout.debug(this->coordinateHandler );
		// this->src.adjustCoordinateHandler(this->coordinateHandler);
	}


	value_t sum;
	unsigned int count;

	//double scaleResult;
	 drain::ValueScaling scaling;


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
			mout.error("area==0 ?" );
		}
		*/

		// mout .debug3() << "src.getMax=" << this->src.scaling.template getMax<value_t>() << mout.endl;
		// this->scaleResult = this->dst.template getMax<value_t>() / this->src.template getMax<value_t>();
		this->scaling.setConversionScale(this->src.getScaling(), this->dst.getScaling());
		mout.debug("scaling:" , this->scaling );

		// WEIGHT
		this->scalingW.setConversionScale(this->srcWeight.getScaling(), this->dstWeight.getScaling());
		//this->scalingW.setScale(this->scalingW.getScale() * static_cast<double>(this->getArea()));
		mout.debug("scalingW:" , this->scalingW );

		//this->SCALE = this->src.scaling.isScaled() || this->dst.scaling.isScaled(); // NEEDED?
		//mout.warn("areaF=" , this->areaF );
		//mout.warn("scaleResult=" , this->scaleResult );

	}

private:

	sum_t w;
	sum_t sumW;

	/// Scales the window area and the dst intensity.
	// double areaF;

	 drain::ValueScaling scalingW;

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
  drainage shapes.png --iAverage 25   -o average.png
  drainage shapes.png --iAverage 5:55 -o average-vert.png
 \endcode

  If the source image contains several channels, each channel is treated separately.

 \code
  drainage image.png --iAverage 25  -o average-rgb.png
 \endcode

 \~exec
   make image-rgba.png
   make graphic-rgba.png
   make spots-rgba.png
   make spots-rgba-16b.png
 \~

 This operator applies weighted averaging, if a weight image (alpha channel) is provided.
 \code
  drainage image-rgba.png --iAverage 25 -o average-weighted-rgba.png
 \endcode

\code
  drainage spots-rgba.png     --iAverage 50 -o spots-avg-rgba.png
  drainage spots-rgba-16b.png --iAverage 50 -o spots-avg-rgba-16b.png
\endcode

\code
  drainage graphic-rgba.png          --iAverage 5 -o graphicAvg-rgba.png
  drainage graphic-rgba-16b.png -T C --iAverage 5 -o graphicAvg-8b-rgba.png
  drainage graphic-rgba-16b.png -T S --iAverage 5 -o graphicAvg-16b-rgba.png
\endcode


\see BlenderOp
\see DistanceTranformFillOp

 */
class FastAverageOp : public WindowOp< Window<WindowConfig> > {

public:

	FastAverageOp(int width=1, int height=0) : WindowOp<Window<WindowConfig> >(__FUNCTION__,
			"Window averaging with optional weighting support.", width, height) {
	};

	FastAverageOp(const FastAverageOp & op) : WindowOp<Window<WindowConfig> >(op) {
		//parameters.copyStruct(op.getParameters(), op, *this);
	}

	/// Forces equal dst image. (Future versions may support different encoding.)
	void getDstConf(const ImageConf & src, ImageConf & dst) const;

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


///  Utility for explicitly
class FastAverageWeightedOp : public FastAverageOp {

public:

	inline
	FastAverageWeightedOp(int width=1, int height=0) : FastAverageOp(width, height) {
	};

	inline
	FastAverageWeightedOp(const FastAverageWeightedOp & op) : FastAverageOp(op) {
	};

	/// Tell if alpha channel(s) is required in input.
	/**
	 *  Future option:
	 *  0: not used
	 *  1: optional, supported
	 *  2: required
	 */
	virtual inline
	int srcAlpha() const {
		return 2; // required
	};

};


}  // image::

}  // drain::

#endif

// Drain
