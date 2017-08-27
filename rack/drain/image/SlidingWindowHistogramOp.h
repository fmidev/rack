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
#include "util/Histogram.h"

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
    public: //re 

};

/// Base class for median and other histogram based statistics.
/** 
 *  \see MarginalStatisticsOp
 */
class SlidingWindowHistogram : public SlidingWindow<HistogramWindowConfig> {

public:

	SlidingWindowHistogram(int width=1, int height=0, int bins=256) : SlidingWindow<HistogramWindowConfig>(width,height) {
		histogram.setSize(bins);
	};

	SlidingWindowHistogram(const HistogramWindowConfig & conf) : SlidingWindow<HistogramWindowConfig>(conf) {
		histogram.setSize(conf.bins); // needed?
	};

	/*
	inline
	void setBins(int bins){
		histogram.setSize(bins);
	}

	/// Median position; value 0.5 corresponds to conventional median.
	inline
	void setMedianPosition(float percentage){
		histogram.setMedianPosition(percentage);
	}

	inline
	void setValueFunc(char c){
		histogram.setValueFunc(c);
	}
	*/


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

		drain::MonitorSource mout(iMonitor, "SlidingWindowHistogram", __FUNCTION__);

		histogram.setSize(conf.bins);
		histogram.setSampleCount(getArea());
		histogram.setScale(src.getMin<int>(), src.getMax<int>(), dst.getMin<int>(), dst.getMax<int>());

		if (!conf.valueFunc.empty())
			histogram.setValueFunc(conf.valueFunc.at(0));
		histogram.setMedianPosition(conf.percentage);

		mout.debug(3) << "a=" << getArea() << " " << mout.endl;
		mout.debug(3) << "histogram=" << histogram << " " << mout.endl;
		mout.debug(3) << coordinateHandler << mout.endl;

	}

	virtual inline
	void clear(){

		drain::MonitorSource mout(iMonitor,"SlidingWindowHistogramWeighted", __FUNCTION__);
		histogram.clearBins();

	}



protected:

	drain::Histogram histogram;

	virtual
	inline
	void removePixel(Point2D<int> & p){
		if (coordinateHandler.validate(p))
			histogram.decrement(src.get<int>(p));
	};

	virtual
	inline
	void addPixel(Point2D<int> & p){
		if (coordinateHandler.validate(p))
			histogram.increment(src.get<int>(p));
	};

	inline
	virtual void write(){
		dst.put(location, histogram.getValue());
	};


	/*
	virtual
	void updateHorz();

	virtual
	void updateVert();
	 */

};

class SlidingWindowHistogramWeighted : public SlidingWindowHistogram {

public:

	typedef SlidingWindowHistogram unweighted;

	SlidingWindowHistogramWeighted(int width=1, int height=0, int bins=256) : SlidingWindowHistogram(width,height,bins) {  };

	SlidingWindowHistogramWeighted(const unweighted::config & conf) : SlidingWindowHistogram(conf) {  };



	/**
	 */
	/*
	void initialize(){

		drain::MonitorSource mout(iMonitor,"SlidingWindowHistogramWeighted", __FUNCTION__);

		mout.debug(3) << "a=" << getArea() << " " << mout.endl;
		mout.debug(3) << "histogram=" << histogram << " " << mout.endl;
		mout.debug(3) << coordinateHandler << mout.endl;
		histogram.setSampleCount(getArea());
		histogram.setScale(src.getMin<int>(), src.getMax<int>(), dst.getMin<int>(), dst.getMax<int>()); // ? unchecked
	};
	*/

	/*
	virtual inline
	void clear(){
		drain::MonitorSource mout(iMonitor,"SlidingWindowHistogramWeighted", __FUNCTION__);
		histogram.clearBins();
	}
	*/

	virtual
	void write(){
		dst.put(location, histogram.getValue());
		//dstWeight.put(location, histogram.getSum<double>() / histogram.getSampleCount() ); // faulty?
		dstWeight.put(location, histogram.getMean<double>()); // faulty?
	};

protected:

	virtual
	inline
	void removePixel(Point2D<int> & p){
		if (coordinateHandler.validate(p))
			histogram.decrement(src.get<int>(p), srcWeight.get<int>(p));
	};

	virtual
	inline
	void addPixel(Point2D<int> & p){
		if (coordinateHandler.validate(p))
			histogram.increment(src.get<int>(p), srcWeight.get<int>(p));
	};

};

/// Window histogram for computing average, sum, median, stddev, variance, maX, miN
/**
 *  \see FastAverageOp
 *  \see FastAverageOp
 */
class SlidingWindowHistogramOp : public SlidingWindowOpT<SlidingWindowHistogramWeighted>
{
public:

	SlidingWindowHistogramOp(int width=1, int height=1, std::string valueFunc="a", double percentage=0.5, int bins=256)
: SlidingWindowOpT<SlidingWindowHistogramWeighted>("SlidingWindowHistogram","A pipeline implementation of window histogram; valueFunc=[asmdvXN] (avg,sum,median,stddev,variance,maX,miN)"){
		//this->conf.width = width;
		//this->conf.height = height;
		parameters.reference("valueFunc", this->conf.valueFunc = valueFunc, "asmdvXN");
		parameters.reference("percentage", this->conf.percentage = percentage);
		parameters.reference("bins", this->conf.bins = bins);
	}

	/*
	double percentage;
	int bins;
	std::string valueFunc;
	*/


protected:

/*
	void traverse(const Image &src, Image &dst) const;

	void traverse(const Image &src, const Image &srcWeight, Image &dst,Image &dstWeight) const;
*/

};

} // image::

} // drain::

#endif

// Drain
