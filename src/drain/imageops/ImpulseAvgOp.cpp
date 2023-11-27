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
#include "drain/image/Coordinates.h"
#include "drain/image/FilePng.h"

#include "ImageOp.h"
#include "ImpulseAvgOp.h"

namespace drain
{
namespace image
{


void ImpulseAvg::init(const Channel & src, bool horizontal){
	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	const size_t n = horizontal ? src.getWidth() : src.getHeight();
	data.resize(n);
	scaling.assign(src.getScaling());

	mout.debug("Data vector, n=" , n );
	mout.debug("Scaling: " , scaling );


}


void ImpulseAvg::reset(){

	latest.first.set(0.0, 0.0);
	latest.second.set(0.0, 0.0);

	for (container::iterator it = data.begin(); it != data.end(); ++it){
		it->first.set(0.0, 0.0);
		it->second.set(0.0, 0.0);
	}

}


void ImpulseAvg::addLeft(int i, double value, double weight){
	e.set(scaling.fwd(value), weight);
	mix(latest.first, e, decays.horz.forward);
	data[i].first = latest.first;
}

void ImpulseAvg::addRight(int i, double value, double weight){
	e.set(scaling.fwd(value), weight);
	mix(latest.second, e, decays.horz.backward);
	data[i].second = latest.second;
}

void ImpulseAvg::addDown(int i, double value, double weight){
	e.set(scaling.fwd(value), weight);
	mix(latest.first, e, decays.vert.forward);
	data[i].first = latest.first;
}

void ImpulseAvg::addUp(int i, double value, double weight){
	e.set(scaling.fwd(value), weight);
	mix(latest.second, e, decays.vert.backward);
	data[i].second = latest.second;
}


double ImpulseAvg::getWeight(int i){  // TODO const
	const entryPair & d = data[i];
	return (d.first.weight + d.second.weight) / 2.0;
}

double ImpulseAvg::get(int i){ // TODO const
	const entryPair & d = data[i];
	double w = d.first.weight + d.second.weight;
	if (w > 0.0)
		return ((d.first.weight*d.first.x + d.second.weight*d.second.x) / w);
	else
		return 0.0; // or code?
}



} // drain::image::

} // drain::

