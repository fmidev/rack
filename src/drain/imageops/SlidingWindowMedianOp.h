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



/** 
	\code
	# Basic example: 5x5 window
	drainage image-gray.png --iMedian 5 -o median.png

	# Vertical 3×7 window
	drainage image-gray.png --iMedian 3:7 -o medianVert.png

	# Weighted median – lower end (darkening)
	drainage image-gray.png --iMedian 5:5,0.1 -o median010.png

	# Weighted median – upper end (brightening)
	drainage image-gray.png --iMedian 5:5,0.9 -o median090.png

	# Coloured image: each channel processed separately
	drainage image.png --iMedian 5 -o median-color.png
	\endcode

 */

//template <class R = WindowCore>
class SlidingWindowMedian : public SlidingWindowHistogram<WindowCore> {

  public:
	
	SlidingWindowMedian(int width=1,int height=0, int bins=256) : SlidingWindowHistogram<WindowCore>(width, height, bins) {
		//this->setInfo("A pipeline implementation of window median.","width,height,percentage",p);
	}

	SlidingWindowMedian(const conf_t & conf) : SlidingWindowHistogram<WindowCore>(conf) {
		//this->setInfo("A pipeline implementation of window median.","width,height,percentage",p);
	}


	virtual
	inline
	void write(){
		/*
		if (this->debugDiag(4)){
			std::cerr << location << '\t' << histogram.getMedian<double>() << '\n';
		}
		*/
		this->dst.put(this->location, histogram.getMedian<double>() );
	}


};

class SlidingWindowMedianWeighted : public SlidingWindowHistogramWeighted {

  public:

	typedef SlidingWindowMedian unweighted;

	SlidingWindowMedianWeighted(int width=1,int height=0, int bins=256) : SlidingWindowHistogramWeighted(width, height, bins) {
	}

	SlidingWindowMedianWeighted(const unweighted::conf_t & conf)  : SlidingWindowHistogramWeighted(conf) {
	}


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


class SlidingWindowMedianOp : public SlidingWindowOp<SlidingWindowMedianWeighted>
{
public:

	SlidingWindowMedianOp(int width=1, int height=1, double percentage=0.5, int bins=256)
		: SlidingWindowOp<SlidingWindowMedianWeighted>("SlidingWindowMedian", "A pipeline implementation of window median."){
		parameters.link("percentage", this->conf.percentage = percentage);
		parameters.link("bins", this->conf.bins = bins);
	}
	

};

}
}

#endif // MEDIAN

// Drain
