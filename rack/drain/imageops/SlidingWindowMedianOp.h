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
	void apply(const ImageFrame &src, ImageFrame &dst) const;

	void apply(const ImageFrame &src, const ImageFrame &srcWeight, ImageFrame &dst,Image &dstWeight) const;
*/

};

}
}

#endif // MEDIAN
