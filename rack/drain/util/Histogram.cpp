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
#include "Histogram.h"

namespace drain
{

/*
*/
Histogram::Histogram(size_type size) : _sampleCount(0),  _sampleCountMedian(0) {
	delimiter = ", ";
	setSize(size);
	setMedianPosition(0.5);
	setScale(0,256);
	_sampleCountNEW = 0;
	//getsy['m'] = &getMedian;
	setValueFunc('a');
}

//	virtual ~Histogram(){};


void Histogram::setSize(const size_type &s){
	_bins = s;
	resize(_bins,0);
	setScale(_inMin,_inMax,_outMin,_outMax);
}


void Histogram::clearBins(){
	for (size_type i = 0; i < _bins; i++)
		(*this)[i] = 0;
	_sampleCountNEW = 0;
}


/*
void Histogram::setSampleCount(long int n){
	_sampleCount = n;
	_sampleCountMedian = static_cast<size_t>(weight * _sampleCount);
}
*/


/// Max refers to upper limit.
void Histogram::setScale(int inMin, int inMax, int outMin, int outMax){
	// First, store the values for future re-scalings.
	_inMin = inMin;
	_inMax = inMax;
	_outMin = outMin;
	_outMax = outMax;
	// Then, scaling information.
	_bins = size();
	_inSpan = inMax-inMin;
	_outSpan = outMax-outMin;
	//cout << *this;
}


void Histogram::dump(std::ostream & ostr){
	for (size_type i = 0; i < _bins; i++)
		ostr << i << ':' << (*this)[i] << '\n';
}


std::ostream & operator<<(std::ostream & ostr, const Histogram &h){
	ostr << "histogram(" << h.getSize() << ")\t";
	ostr << '[' << h.getInMin() << ','  << h.getInMax() << '[';
	ostr << " => ";
	ostr << '[' << h.getOutMin() << ','  << h.getOutMax() << '[' << '\n';
	ostr << " sum=" << h.getSum<double>() << ',';
	if (h.getSum<double>()>0){
		ostr << " mean=" << h.getMean<double>() << ',';
		ostr << " min=" << h.getMin<double>() << ',';
		ostr << " max=" << h.getMax<double>() << ',';
		ostr << " stdev=" << h.getStdDeviation<double>() << ',';
		ostr << '\n';
	}
	return ostr;
}

}

// Drain
