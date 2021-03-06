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
//#include "Type.h"
#include "TypeUtils.h"

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
	sampleCount = 0;
	weight = 0.5;
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


std::size_t  Histogram::recommendSizeByType(const std::type_info & type, std::size_t value){

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (drain::Type::call<drain::typeIsSmallInt>(type)){
		const size_t s = drain::Type::call<drain::sizeGetter>(type);
		const size_t bits = (s*8);
		mout.debug() << bits << " bits => setting " << (1<<bits) << " bins " << mout.endl;
		value = (1<<bits);
	}
	else {
		mout.note() << "assuming " << value << " bins" << mout.endl;
	}

	//setSize(value);

	return value;
}


void Histogram::clearBins(){
	for (size_type i = 0; i < bins; i++)
		(*this)[i] = 0;
	sampleCount = 0;
}

Histogram::stat_ptr_t Histogram::getStatisticPtr(char c){

	switch (c) {
	case 'a':
		return & Histogram::getMean<double>;
		break;
	case 's':
		return & Histogram::getSum<double>;
		break;
	case 'm':
		return & Histogram::getMedian<double>;
		break;
	case 'd':
		return & Histogram::getStdDeviation<double>;
		break;
	case 'v':
		return & Histogram::getVariance<double>;
		break;
	case 'X':
		return & Histogram::getMax<double>;
		break;
	case 'N':
		return & Histogram::getMin<double>;
		break;
	default:
		throw std::runtime_error(std::string("Histogram::getStatisticPtr unimplemented type: ") + c);
		return & Histogram::getMean<double>;
		break;
	}

}



void Histogram::dump(std::ostream & ostr){
	ostr << "# i\tvalue \tcount\n";
	for (size_type i = 0; i < bins; i++){
		ostr << i << '\t' << scaleOut<double>(i) << '\t' << (*this)[i] << '\n';
	}
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

// Drain
