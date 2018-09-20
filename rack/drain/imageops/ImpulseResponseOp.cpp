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

#include <sstream>
#include <ostream>
#include "image/Coordinates.h"
#include "image/FilePng.h"

#include "ImageOp.h"
#include "ImpulseResponseOp.h"

namespace drain
{
namespace image
{


void ImpulseAvg::reset(){
	unDecay = 1.0-this->decay;
	prev1 = 0.0;
	prev2 = 0.0;
	for (std::vector<entry>::iterator it = data.begin(); it != data.end(); ++it){
		it->value1  = 0.0;
		it->weight1 = 0.0;
		it->value2  = 0.0;
		it->weight2 = 0.0;
	}
}


void ImpulseAvg::add1(int i, double value, double weight){
	entry & d = data[i];
	d.value1 = prev1 =  unDecay*value + this->decay*prev1;
	d.weight1 = weight;
}

void ImpulseAvg::add2(int i, double value, double weight){
	entry & d = data[i];
	d.value2  = prev2 =  unDecay*value + this->decay*prev2;
	d.weight2 = weight;
}

double ImpulseAvg::getWeight(int i){
	entry & d = data[i];
	return d.weight1 + d.weight2;
}

double ImpulseAvg::get(int i){
	entry & d = data[i];
	double w = d.weight1 + d.weight2;
	if (w > 0.0)
		return (d.weight1*d.value1 + d.weight2*d.value2) / w;
	else
		return 0.0; // or code?
}



} // drain::image::

} // drain::

