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
#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

//
#include <cmath>

#include <vector>
#include <iostream>
#include <stdexcept>
#include <map>


#include "LinearScaling.h"

namespace drain
{

// // // using namespace std;

//std::cerr;

/// Class for computing a histogram and some statistics: average, min, max, mean, std.dev, sum.
/**
 *  T  input type, eg. unsigned char for 8-bit images.
 *  T2 output type for values not within the set of input values, like std.dev.
 */
class Histogram : protected std::vector<unsigned long> {
public:

	Histogram(size_t size=256);

	Histogram(const Histogram & histogram);

	virtual ~Histogram(){};

	LinearScaling scaling;

	/// Sets the number of bins; the resolution of the histogram.
	void setSize(size_t s);

	inline
	int getSize() const { return bins; };

	/// Does not change the size of the histogram.
	void clearBins();


	/// Sets the expected sample count.
	void setSampleCount(long int n){
		//sampleCount = n;
		sampleCountNEW = n;
		sampleCountMedian = static_cast<size_t>(weight * static_cast<double>(n));
	}

	size_t getSampleCount() const {
		return sampleCountNEW;
	}

	/// Max refers to upper limit.
	//void setScale(int inMin, int inMax, int outMin, int outMax);
	//void setScale(int inMin, int inMax, int outMin, int outMax){
	//	scaling.setRange(0,inMax,outMin, outMax);
	//}

	/// Set range of original (physical) values to be mapped on the limited number of bins. Note: max refers to open upper limit.
	inline
	void setScale(double dataMin, double dataMax){
		scaling.setRange(0.0, bins-1.0, dataMin, dataMax);
	};


	inline
	int getInMin() const { return 0; };

	/// Returns the upperLimit (exclusive)
	inline
	int getUpperBoundIn() const { return bins; };

	inline
	int getOutMin() const { return scaling.forward(0); };

	/// Returns the upperLimit (exclusive)
	inline
	int getUpperBoundOut() const { return scaling.forward(bins); };



	/// Set location of the median, if not in the middle (50%).
	/**
	 *
	 */
	inline
	void setMedianPosition(double pos){
		weight = pos;
		sampleCountMedian = static_cast<size_t>(weight * sampleCountNEW);
	};

	/*
	template <class T>
	inline
	bool withinLimits(T i) const {
		return  ((i >= inMin) && (i <= inMax));  // check if ok inMax is too much
	}
	*/

	template <class T>
	inline
	void increment(T i){
		//++(*this)[((i-inMin)*bins)/inSpan];
		++(*this)[scaling.inverse(i)];
		++sampleCountNEW;  // TODO: slow?
	}

	template <class T>
	inline
	void increment(T i, int count){
		(*this)[scaling.inverse(i)] += count;
		//(*this)[((i-inMin)*bins)/inSpan] += count;
		sampleCountNEW += count;
	}

	template <class T>
	inline
	void decrement(T i){
		--(*this)[scaling.inverse(i)]; // [((i-inMin)*bins)/inSpan];
		--sampleCountNEW;
	}

	template <class T>
	inline
	void decrement(T i, int count){
		(*this)[scaling.inverse(i)] -= count; //[((i-inMin)*bins)/inSpan] -= count;
		sampleCountNEW -= count;
	}


	template <class T>
	inline
	T scaleOut(const size_type &i) const {
		//return static_cast<T>(outMin + (i*outSpan)/bins);
		return static_cast<T>(scaling.forward(i));
	}




	/// Statistics


	// double (Histogram::* get)() const;
    // map<char,double (Histogram::*)() const> getsy;

	/// Returns average, min, Max, Sum, stdev, meDian
	/**
	 *  \par key - the letter of desired quantity: [a]verage, [s]um, std[d]ev, [vField]ariance, [m]edian, mi[n], ma[x],
	 */
	// DEPRECATED, see getValue
	template <class T>
	inline
	T get(const char & key){
		switch (key) {
		case 'a':
			return getMean<T>();
		case 's':
			return getSum<T>();
		case 'm':
			return getMedian<T>();
		case 'd':
			return getStdDeviation<T>();
		case 'v':
			return getVariance<T>();
		case 'X':
			return getMax<T>();
		case 'N':
			return getMin<T>();
		default:
			// Log
			throw std::runtime_error(std::string("Histogram::get unimplemented type: ") + key);
			return 0;
		}
	}

	/// 
	//  @param p applies to weighted median; for standard median, p = 0.5.
	template <class T>
	inline
	T getMax() const {
		for (size_type i = bins-1; i > 0; --i)
			if ((*this)[i] > 0)
				return scaleOut<T>(i);
		// cerr << "Warning: Histogram empty.\n";
		return  getUpperBoundOut(); //static_cast<T>(outMax);
	}


	template <class T>
	inline
	T getMin() const {
		for (size_type i = 0; i < bins; ++i)
			if ((*this)[i] > 0)
				return scaleOut<T>(i);
		//cerr << "Warning: Histogram empty.\n";
		return  getUpperBoundOut(); //static_cast<T>(outMax);
	}

	/// Unscaled sum.
	template <class T>
	inline
	T getSum() const {
		size_type _sum;
		_sum = 0;
		for (size_type i = 0; i < bins; i++){
			_sum += (*this)[i] * scaleOut<T>(i); // count * nominal (should be scaled to middle)
		}
		return _sum;
	}

	//template <class T2>
	template <class T>
	inline
	T getMean() const {
		//return scaleOut(getSum()/sampleCount);
		if (sampleCountNEW > 0)
		  return getSum<T>()/sampleCountNEW;
		else
		  return 0;
	}


	template <class T>
	inline
	T getMedian() const {
		size_type _sum;
		_sum = 0;
		for (size_type i = 0; i < bins; ++i){
			_sum += (*this)[i];
			if (_sum >= sampleCountMedian){
				return scaleOut<T>(i);
			}
		}
		return getUpperBoundOut(); //static_cast<T>(outMax);
	}


	template <class T>
	inline
	T getWeightedMedian(float p) const {
		size_type _sum;
		if ((p < 0.0) || (p>1.0)){
			throw std::runtime_error("Histogram<T>::getMedian: median point <0 or >1.0 .");
		}
		const size_t limit =  sampleCountMedian; //   static_cast<size_t>(p * sampleCount);
		_sum = 0;
		for (size_type i = 0; i < bins; i++){
			_sum += (*this)[i];
			if (_sum >= limit){
				return static_cast<T>(i);
			}
		}
		return static_cast<T>(bins);
	}


	/// Computes variance of the values inside the window.
	//  NOTE: could be pipelined.
	template <class T>
	inline
	T getVariance() const {
		int n;
		long sum;
		long sum2;
		T f;
		T sumT;

		sum = 0;
		sum2 = 0;
		for (size_type i = 0; i < bins; i++){
			f = scaleOut<T>(i);
			n = (*this)[i];
			sum  += n * f;
			sum2 += n * (f*f);
		}
		sumT = static_cast<T>(sum)/sampleCountNEW;
		return  static_cast<T>(sum2)/sampleCountNEW - sumT*sumT;
	}


	template <class T>
	inline
	T getStdDeviation() const {
		return  static_cast<T>(sqrt(getVariance<double>()));
	}






	inline
	const std::vector<unsigned long> getVector() const {
		return *this;
	};

	void dump(std::ostream & ostr);

	std::string delimiter;

	inline
	double getValue(){
		return (this->*getValuePtr)();
	}

	void setValueFunc(char c){


		switch (c) {
		case 'a':
			getValuePtr = & Histogram::getMean<double>;
			break;
		case 's':
			getValuePtr = & Histogram::getSum<double>;
			break;
		case 'm':
			getValuePtr = & Histogram::getMedian<double>;
			break;
		case 'd':
			getValuePtr = & Histogram::getStdDeviation<double>;
			break;
		case 'v':
			getValuePtr = & Histogram::getVariance<double>;
			break;
		case 'X':
			getValuePtr = & Histogram::getMax<double>;
			break;
		case 'N':
			getValuePtr = & Histogram::getMin<double>;
			break;
		default:
			throw std::runtime_error(std::string("Histogram::get unimplemented type: ") + c);
			break;
		}


	}

protected:

	void initialize(size_t size);

	double (Histogram::*getValuePtr)() const;


private:
	/// Resolution of the histogram.
	size_t bins;

	/// The expected sample count in the histogram.
	//size_type sampleCount;

	/// The actual sample count in the histogram. FOR WEIGHTED
	size_type sampleCountNEW;

	// Half of sampleCount.
	size_type sampleCountMedian;

	// weight for weighted median;
	float weight;

	/*
	int inMax;
	int inMin;
	int inSpan;
	int outMin;
	int outMax;
	int outSpan;
	*/

};

std::ostream & operator<<(std::ostream &ostr,const Histogram &h);

}

//std::ostream & operator<<(std::ostream &ostr,const drain::Histogram &h);

//template <class T>


#endif /*HISTOGRAM_H_*/

// Drain
