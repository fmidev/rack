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
#include "Histogram.h"


namespace drain
{

/*
*/
Histogram::Histogram(size_t size){
	initialize(size);
}

Histogram::Histogram(const Histogram & histogram){
	initialize(histogram.getSize());
}

//	virtual ~Histogram(){};
void Histogram::initialize(size_t size){
	delimiter = ", ";
	//setScale(0, size);
	setSize(size);
	//setMedianPosition(0.5);
	sampleCountNEW = 0;
	weight = 0.0;
	sampleCountMedian = 0;
	setValueFunc('a');
}

void Histogram::setSize(size_t s){
	bins = s;
	//resize(bins,0);
	resize(bins);
	//setScale(inMin,inMax,outMin,outMax);
	setScale(0, bins-1);
}


void Histogram::clearBins(){
	for (size_type i = 0; i < bins; i++)
		(*this)[i] = 0;
	sampleCountNEW = 0;
}


/// Max refers to upper limit.
/*
void Histogram::setScale(int inMin, int inMax, int outMin, int outMax){
	// First, store the values for future re-scalings.
	this->inMin = inMin;
	this->inMax = inMax;
	this->outMin = outMin;
	this->outMax = outMax;
	// Then, scaling information.
	bins = size();
	this->inSpan = inMax-inMin;
	this->outSpan = outMax-outMin;
	//cout << *this;
}
*/

void Histogram::dump(std::ostream & ostr){
	for (size_type i = 0; i < bins; i++)
		ostr << i << ':' << (*this)[i] << '\n';
}


std::ostream & operator<<(std::ostream & ostr, const Histogram &h){
	ostr << "histogram(" << h.getSize() << ")\t";
	ostr << '[' << h.getInMin() << ','  << h.getUpperBoundIn() << '[';
	ostr << " => ";
	ostr << '[' << h.getOutMin() << ','  << h.getUpperBoundOut() << '[' << '\n';
	ostr << " sum=" << h.getSum<double>() << ',';
	if (h.getSum<double>()>0){
		ostr << " n=" << h.getSampleCount() << ',';
		ostr << " mean=" << h.getMean<double>() << ',';
		ostr << " min=" << h.getMin<double>() << ',';
		ostr << " max=" << h.getMax<double>() << ',';
		ostr << " stdev=" << h.getStdDeviation<double>() << ',';
		ostr << '\n';
	}
	return ostr;
}

}
