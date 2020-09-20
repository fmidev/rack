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
#ifndef SLIDINGWINDOWHISTOGRAM_H_
#define SLIDINGWINDOWHISTOGRAM_H_



#include "SlidingWindowOp.h"
//#include "SlidingOp.h"  // NEW 2015
#include "drain/util/Histogram.h"

namespace drain
{

namespace image
{

struct HistogramConfig {

	int bins;
	float percentage;
	std::string valueFunc;

};

class HistogramWindowConfig : public WindowConfig, public HistogramConfig {
    public:

};

/// Base class for median and str histogram based statistics.
/** 
 *
 *  \tparam - Resource
 *
 *  \see MarginalStatisticsOp
 */
template <class R = WindowCore>
class SlidingWindowHistogram : public SlidingWindow<HistogramWindowConfig, R> {

public:

	SlidingWindowHistogram(int width=1, int height=0, int bins=256) : SlidingWindow<HistogramWindowConfig, R>(width,height) {
		histogram.setSize(bins);
	};

	SlidingWindowHistogram(const HistogramWindowConfig & conf) : SlidingWindow<HistogramWindowConfig, R>(conf) {
		histogram.setSize(conf.bins); // needed?
	};



	/*
	 *   TODO: weight = > weightSUm
	 *   TODO: multiple channels (banks)?
	 *   FIXME: Polar coord problem
	 */
	/** 
	 *   TODO: weight = > weightSUm
	 *   TODO: multiple channels (banks)?
	 */
	virtual inline
	void initialize(){

		drain::Logger mout(getImgLog(), "SlidingWindowHistogram", __FUNCTION__);

		this->setImageLimits();
		this->setLoopLimits();
		this->location.setLocation(0, 0);

		histogram.setSize(this->conf.bins);
		histogram.setSampleCount(this->getArea());
		//histogram.setScale(src.getMin<int>(), src.getMax<int>(), dst.getMin<int>(), dst.getMax<int>());
		histogram.setScale(this->src.getEncoding().template getTypeMin<int>(), this->src.getEncoding().template getTypeMax<int>());

		if (!this->conf.valueFunc.empty())
			histogram.setValueFunc(this->conf.valueFunc.at(0));
		histogram.setMedianPosition(this->conf.percentage);

		mout.debug(3) << "a=" << this->getArea() << " " << mout.endl;
		mout.debug(3) << "histogram=" << histogram << " " << mout.endl;
		mout.debug(3) << this->coordinateHandler << mout.endl;

	}

	virtual inline
	void clear(){

		drain::Logger mout(getImgLog(),"SlidingWindowHistogramWeighted", __FUNCTION__);
		histogram.clearBins();

	}



protected:

	drain::Histogram histogram;

	virtual inline
	void setImageLimits() const {
		this->src.adjustCoordinateHandler(this->coordinateHandler);
	}

	virtual inline
	void removePixel(Point2D<int> & p){
		if (this->coordinateHandler.validate(p))
			histogram.decrement(this->src.template get<int>(p));
	};

	virtual inline
	void addPixel(Point2D<int> & p){

		//if (this->debugDiag(4))std::cerr << this->location << '\n';

		if (this->coordinateHandler.validate(p))
			histogram.increment(this->src.template get<int>(p));
	};

	inline virtual
	void write(){
		this->dst.put(this->location, histogram.getValue());
	};


	/*
	virtual
	void updateHorz();

	virtual
	void updateVert();
	 */

};

/*
class SlidingWindowHistogram : public SlidingWindowHistogramBase<WindowCore> {
public:
	SlidingWindowHistogram(const unweighted::conf_t & conf) : SlidingWindowHistogramBase<WindowCore>(conf) {  };

};
*/

class SlidingWindowHistogramWeighted : public SlidingWindowHistogram<WeightedWindowCore> {

public:

	typedef SlidingWindowHistogram<WindowCore> unweighted;

	SlidingWindowHistogramWeighted(int width=1, int height=0, int bins=256) : SlidingWindowHistogram<WeightedWindowCore>(width,height,bins) {  };

	SlidingWindowHistogramWeighted(const unweighted::conf_t & conf) : SlidingWindowHistogram<WeightedWindowCore>(conf) {  };


	virtual
	void write(){
		this->dst.put(this->location, this->histogram.getValue());
		//dstWeight.put(location, histogram.getSum<double>() / histogram.getSampleCount() ); // faulty?
		this->dstWeight.put(this->location, this->histogram.getMean<double>()); // faulty?
	};

protected:


	virtual inline
	void addPixel(Point2D<int> & p){
		/*
		if (this->debugDiag(4)){
			std::cerr << this->location << '\n';
		}
		*/
		if (this->coordinateHandler.validate(p))
			this->histogram.increment(this->src.get<int>(p), this->srcWeight.get<int>(p));
	};

	virtual inline
	void removePixel(Point2D<int> & p){
		if (this->coordinateHandler.validate(p))
			this->histogram.decrement(this->src.get<int>(p), this->srcWeight.get<int>(p));
	};

};

/// Window histogram for computing [asmdvNX] = average, sum, median, stddev, variance, miN, maX
/**

\code
  drainage gray.png --histogram 5,5,a -o hist-avg.png
  drainage gray.png -T S --histogram 15,15,s -o hist-sum.png
  drainage gray.png --histogram 5,5,m -o hist-med.png
  drainage gray.png --histogram 5,5,d -o hist-dev.png
  drainage gray.png --histogram 5,5,v -o hist-var.png
  drainage gray.png --histogram 5,5,N -o hist-min.png
  drainage gray.png --histogram 5,5,X -o hist-max.png
\endcode

 \see SlidingWindowMedian
 \see FastAverageOp

 */
class SlidingWindowHistogramOp : public SlidingWindowOp<SlidingWindowHistogramWeighted>
{
public:

	SlidingWindowHistogramOp(int width=1, int height=1, std::string valueFunc="a", double percentage=0.5, int bins=256)
: SlidingWindowOp<SlidingWindowHistogramWeighted>("SlidingWindowHistogram",
		"A pipeline implementation of window histogram; valueFunc=[asmdvNX] (avg,sum,median,stddev,variance,miN,maX)"){
		//this->conf.width = width;
		//this->conf.height = height;
		parameters.reference("valueFunc", this->conf.valueFunc = valueFunc, "asmdvXN");
		parameters.reference("percentage", this->conf.percentage = percentage);
		parameters.reference("bins", this->conf.bins = bins);
	}

};

} // image::

} // drain::

#endif

// Drain
