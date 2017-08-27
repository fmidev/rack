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
#ifndef SLIDINGWINDOWMEDIANOP_H_
#define SLIDINGWINDOWMEDIANOP_H_


#include "SlidingWindowHistogramOp.h"

namespace drain
{

namespace image
{

// using namespace std;


/** 
	\code
	drainage gray.png --median 5 -o median.png
	\endcode

	\code
	drainage gray.png --median 3,7 -o medianVert.png
	\endcode

	\code
	drainage gray.png --median 5,5,0.1 -o median010.png
	\endcode

	\code
	drainage gray.png --median 5,5,0.9 -o median090.png
	\endcode

	\code
	drainage image.png --median 5 -o median-color.png
	\endcode
 */
class SlidingWindowMedian : public SlidingWindowHistogram {

  public:
	
	SlidingWindowMedian(int width=1,int height=0, int bins=256) : SlidingWindowHistogram(width, height, bins) {
		//this->setInfo("A pipeline implementation of window median.","width,height,percentage",p);
	}

	SlidingWindowMedian(const config & conf) : SlidingWindowHistogram(conf) {
		//this->setInfo("A pipeline implementation of window median.","width,height,percentage",p);
	}


	/*
	virtual
	inline
	void initialize(){

		MonitorSource mout(iMonitor, "SlidingWindowMedian", __FUNCTION__);
		histogram.setScale(src.getMin<int>(),src.getMax<int>(),dst.getMin<int>(),dst.getMax<int>());
		mout.debug(3) << "histogram:" << histogram << mout.endl;
		SlidingWindowHistogram::initialize();

		mout.debug(3) << "finished." << mout.endl;

	}
	*/

	virtual
	inline
	void write(){
		dst.put(location, histogram.getMedian<double>() );
	}
	
	/*
	virtual
	inline
	void toStream(std::ostream & ostr) const {
		Window<HistogramWindowConfig>::toStream(ostr);
		ostr << histogram;
	}
	*/

};

class SlidingWindowMedianWeighted : public SlidingWindowHistogramWeighted {

  public:

	typedef SlidingWindowMedian unweighted;

	SlidingWindowMedianWeighted(int width=1,int height=0, int bins=256) : SlidingWindowHistogramWeighted(width, height, bins) {
	}

	SlidingWindowMedianWeighted(const unweighted::config & conf)  : SlidingWindowHistogramWeighted(conf) {
	}

	//Value 0.5 corresponds to standard median
	/*
	inline
	void setLimit(float percentage){
		histogram.setMedianPosition(percentage);
	}
	*/


	/*
	virtual
	inline
	void initialize(){

		MonitorSource mout(iMonitor,"SlidingWindowMedian", __FUNCTION__);
		histogram.setScale(src.getMin<int>(), src.getMax<int>(), dst.getMin<int>(), dst.getMax<int>());
		mout.debug(3) << "histogram:" << histogram << mout.endl;
		SlidingWindowHistogramWeighted::initialize();
		mout.debug(3) << "finished." << mout.endl;

	}
	*/

	virtual
	inline
	void write(){
		dst.put(location, histogram.getMedian<double>() );
		dstWeight.put(location, histogram.getSum<double>() / histogram.getSize() ); // TODO divBy0
	}

	/*
	virtual
	inline
	void toStream(std::ostream & ostr) const {
		Window<HistogramWindowConfig>::toStream(ostr);
		ostr << histogram;
	}
	*/

};


class SlidingWindowMedianOp : public SlidingWindowOpT<SlidingWindowMedianWeighted>
{
public:

	SlidingWindowMedianOp(int width=1, int height=1, double percentage=0.5, int bins=256)
		: SlidingWindowOpT<SlidingWindowMedianWeighted>("SlidingWindowMedian", "A pipeline implementation of window median."){
		//this->conf.width = width;
		//this->conf.height = height;
		//reference("width", this->width, width);
		//reference("height", this->height, height);
		parameters.reference("percentage", this->conf.percentage = percentage);
		parameters.reference("bins", this->conf.bins = bins);
	}
	
	//double percentage;
	//int bins;


protected:

/*
	void traverse(const Image &src, Image &dst) const;

	void traverse(const Image &src, const Image &srcWeight, Image &dst,Image &dstWeight) const;
*/

};

}
}

#endif // MEDIAN

// Drain
