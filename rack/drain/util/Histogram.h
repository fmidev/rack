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
#include <typeinfo>
#include <cmath>

#include <vector>
#include <iostream>
#include <stdexcept>
#include <map>

#include "Log.h"

#include "ValueScaling.h"



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

	typedef unsigned long count_t;
	typedef std::vector<count_t> vect_t;

	Histogram(size_t size=256);

	Histogram(const Histogram & histogram);

	virtual ~Histogram(){};

	ValueScaling scaling;

	/// Sets the number of bins; the resolution of the histogram.
	void setSize(size_t s);

	static
	std::size_t recommendSizeByType(const std::type_info & type, std::size_t defaultValue = 256);

	inline
	int getSize() const { return size(); };

	inline
	int autoSize(const std::type_info & type){
		if (empty()){
			resize(recommendSizeByType(type, 256));
		}
		return size();
	};

	/// Does not change the size of the histogram.
	inline
	void clearBins(){
		std::fill(begin(), end(), 0);
	}

	/// Collect distribution.
	/**
	 *  \param src - source data
	 *  \tparam T - something iterable, has begin() and end() for example.
	 */
	template <class T>
	void compute(const T & src, const std::type_info & type = typeid(double));

	/// Does not change the size of the histogram.
	//void clearBins();


	/// Sets the expected sample count. Deprecating.
	void setSampleCount(long int n){
		//sampleCount = n;
		sampleCount = n;
		sampleCountMedian = static_cast<size_t>(weight * static_cast<double>(n));
	}

	size_t getSampleCount() const {
		return sampleCount;
	}


	/// Set range of original (physical) values to be mapped on the limited number of bins. Note: max refers to open upper limit.
	void setRange(double dataMin, double dataMax);

	/// Set range of original (physical) values to be mapped on the limited number of bins. Note: max refers to open upper limit.
	inline
	void setRange(const Range<double> & range){
		setRange(range.min, range.max);
	}

	inline
	void deriveScaling(const ValueScaling & s, const Type & type){
		setRange(s.fwd(0.0), s.fwd(1 << (8*drain::Type::call<drain::sizeGetter>(type))));
	}

	inline
	void setScale(const ValueScaling & scaling){
		drain::Logger mout(__FUNCTION__, __FILE__);
		mout.discouraged("use setRange instead (scaling=", scaling, ")");
		mout.advice("range perhaps: [", this->scaling.getPhysicalRange());
		this->scaling.assign(scaling);
		//int s = 1 << (8*drain::Type::call<drain::sizeGetter>(type));
	}

	/// Set range of original (physical) values to be mapped on the limited number of bins. Note: max refers to open upper limit.
	/*
	inline
	void setScale(const ValueScaling & scaling){
		//scaling.setRange(0.0, bins-1.0, dataMin, dataMax);
	};
	*/

	inline
	int getInMin() const { return 0; };

	/// Returns the upperLimit (exclusive)
	inline
	int getUpperBoundIn() const { return size(); };


	inline
	int getOutMin() const { return scaling.fwd(0); };

	/// Returns the upperLimit (exclusive)
	inline
	int getUpperBoundOut() const { return scaling.fwd(size()); };



	/// Set location of the median, if not in the middle (50%).
	/**
	 *
	 */
	inline
	void setMedianPosition(double pos){
		weight = pos;
		sampleCountMedian = static_cast<size_t>(weight * sampleCount);
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
		++(*this)[scaling.inv(i)];
		++sampleCount;  // TODO: slow?
	}

	template <class T>
	inline
	void increment(T i, int count){
		(*this)[scaling.inv(i)] += count;
		//(*this)[((i-inMin)*bins)/inSpan] += count;
		sampleCount += count;
	}

	template <class T>
	inline
	void incrementRaw(T i){
		++(*this)[i];
		++sampleCount;  // TODO: slow?
	}

	template <class T>
	inline
	void incrementRaw(T i, int count){
		(*this)[i] += count;
		sampleCount += count;
	}

	template <class T>
	inline
	void decrement(T i){
		--(*this)[scaling.inv(i)]; // [((i-inMin)*bins)/inSpan];
		--sampleCount;
	}

	template <class T>
	inline
	void decrement(T i, int count){
		(*this)[scaling.inv(i)] -= count; //[((i-inMin)*bins)/inSpan] -= count;
		sampleCount -= count;
	}

	template <class T>
	inline
	void decrementRaw(T i){
		--(*this)[i];
		--sampleCount;
	}

	template <class T>
	inline
	void decrementRaw(T i, int count){
		(*this)[i] -= count;
		sampleCount -= count;
	}


	template <class T>
	inline
	T scaleOut(size_type i) const {
		//return static_cast<T>(outMin + (i*outSpan)/bins);
		return static_cast<T>(scaling.fwd(i));
	}




	/// Statistics

	/// 
	//  @param p applies to weighted median; for standard median, p = 0.5.
	template <class T>
	inline
	T getMax() const {
		for (size_type i = size()-1; i > 0; --i)
			if ((*this)[i] > 0)
				return scaleOut<T>(i);
		// cerr << "Warning: Histogram empty.\n";
		return  getUpperBoundOut(); //static_cast<T>(outMax);
	}


	template <class T>
	inline
	T getMin() const {
		for (size_type i = 0; i < size(); ++i)
			if ((*this)[i] > 0)
				return scaleOut<T>(i);
		//cerr << "Warning: Histogram empty.\n";
		return  getUpperBoundOut(); //static_cast<T>(outMax);
	}

	/// Sum of the samples
	/**
	 *   TODO: handle somehow no-data and other markers - with a different function?
	 */
	template <class T>
	//inline
	T getSum() const {
		double sum = 0;
		//std::cout << __FUNCTION__ << ':' << sum << std::endl;
		for (size_type i = 0; i < size(); i++){
			sum += ((*this)[i] * scaleOut<T>(i)); // count * nominal (should be scaled to middle)
			//if ((*this)[i])
			//	std::cout << i << '\t' << (*this)[i] << '\t' << scaleOut<T>(i) << '\t' << sum << '\n';
		}
		return sum;
	}


	/// Unscaled mean
	template <class T>
	inline
	T getMean() const {
		//return scaleOut(getSum()/sampleCount);
		if (sampleCount > 0)
		  return getSum<T>()/sampleCount;
		else
		  return 0;
	}


	template <class T>
	inline
	T getMedian() const {
		size_type sum;
		const size_type limit = static_cast<size_t>(weight*sampleCount);

		sum = 0;
		for (size_type i = 0; i < size(); ++i){
			sum += (*this)[i];
			if (sum >= limit){
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
		for (size_type i = 0; i < size(); i++){
			_sum += (*this)[i];
			if (_sum >= limit){
				return static_cast<T>(i);
			}
		}
		return static_cast<T>(size());
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
		for (size_type i = 0; i < size(); i++){
			f = scaleOut<T>(i);
			n = (*this)[i];
			sum  += n * f;
			sum2 += n * (f*f);
		}
		sumT = static_cast<T>(sum)/sampleCount;
		return  static_cast<T>(sum2)/sampleCount - sumT*sumT;
	}


	template <class T>
	inline
	T getStdDeviation() const {
		return  static_cast<T>(sqrt(getVariance<double>()));
	}






	inline
	const vect_t getVector() const {
		return *this;
	};

	void dump(std::ostream & ostr = std::cout);

	std::string delimiter;

	inline
	double getValue(){
		return (this->*statisticPtr)();
	}

	/// Return requested statistic
	/**
	 *
	 *
	 */
	inline
	double getValue(char c){
		return (this->*getStatisticPtr(c))();
	}


	inline
	void setValueFunc(char c){
		statisticPtr = getStatisticPtr(c);
	}


protected:

	void initialize();

	typedef double (Histogram::*stat_ptr_t)() const;

	double (Histogram::*statisticPtr)() const;

	stat_ptr_t getStatisticPtr(char c);


	//double (Histogram::*)getPtr const;

private:
	/// Resolution of the histogram.
	// size_t bins;

	/// The expected sample count in the histogram.
	//size_type sampleCount;

	/// The actual sample count in the histogram. FOR WEIGHTED
	size_type sampleCount;

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


template <class T>
void Histogram::compute(const T & dst, const std::type_info & type){

	drain::Logger mout(__FUNCTION__, __FILE__);

	const std::size_t s = autoSize(type);

	mout.note(s, " bins");

	initialize();

	if (s == 0){
		mout.error("0 bins, something went wrong");
		return;
	}

	if (s > 0x10000){
		mout.error(s, " bins exceeds Histogram size limit: ", 0x10000);
		return;
	}


	const bool IS_SMALL_INT = (type == typeid(unsigned char)) || (type == typeid(unsigned short int));

	int histBits = 0;
	for (int i=0; (s>>i)>0; ++i){
		histBits=i;
	}

	if (IS_SMALL_INT && (s == (1<<histBits))){
		int dataBits = 8*drain::Type::call<drain::sizeGetter>(type);
		int bitShift = dataBits - histBits;
		mout.note("Fast mode (bitShift=", bitShift, ": data ", dataBits, "b int, histogram ", histBits, "b <=", s, " bins)");
		for (typename T::const_iterator it = dst.begin(); it != dst.end(); ++it){
			this->incrementRaw(static_cast<unsigned short int>(*it) >> bitShift);
		}
	}
	else {
		mout.warn("Size ", s, " not 2^N (slow mode)");
		mout.advice("Consider histogram size of 2^N, e.g.", (1<<histBits));
		mout.unimplemented("(slow mode)");
		mout.error("(slow mode)");
		// ValueScaling only here!
	}

	/*
	if ((s == 256) && (type == typeid(unsigned char))){
		mout.note() << "direct 256-mode: u char (fast)" << mout.endl;
		for (typename T::const_iterator it = dst.begin(); it != dst.end(); ++it){
			this->incrementRaw(static_cast<unsigned short int>(*it));
		}
	}
	else if ((s == 256) && (type == typeid(unsigned short int))){
		mout.note() << "direct 256-mode: u short (fast)" << mout.endl;
		for (typename T::const_iterator it = dst.begin(); it != dst.end(); ++it){
			this->incrementRaw(static_cast<unsigned short int>(*it) >> 8);
		}
	}
	else {
		mout.note("scaled mode (slow)");

		if (dst.getScaling().isPhysical()){
			this->setRange(dst.getScaling().getMinPhys(), dst.getScaling().getMaxPhys());
		}
		//this->setRange(dst.getScaling().getMinPhys(), dst.getScaling().getMaxPhys());
		mout.note("range:   ", scaling.physRange);
		mout.note("scaling: ", scaling);
		for (typename T::const_iterator it = dst.begin(); it != dst.end(); ++it){
			this->increment(static_cast<unsigned int>(*it));
		}
	}
	*/

	mout.note("finito");

}


std::ostream & operator<<(std::ostream &ostr,const Histogram &h);

}

//std::ostream & operator<<(std::ostream &ostr,const drain::Histogram &h);

//template <class T>


#endif /*HISTOGRAM_H_*/

// Drain
