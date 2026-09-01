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


#include "ImpulseAvgOp.h"

namespace drain
{
namespace image
{


void ImpulseAvg::init(const Channel & src, bool horizontal){

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	const size_t n = horizontal ? src.getWidth() : src.getHeight();
	line.resize(n); // , {{0,0}, {0,0}});
	scaling.assignSequence(src.getScaling());

	mout.debug("Data vector, n=" , n );
	mout.debug("Scaling: " , scaling );

}


void ImpulseAvg::reset(){

	accumulated.forward.set(0.0, 0.0);
	accumulated.backward.set(0.0, 0.0);

	for (container::iterator it = line.begin(); it != line.end(); ++it){
		it->forward.set(0.0, 0.0);
		it->backward.set(0.0, 0.0);
	}

}

/**
 *
 *  \param alpha - proportion of current value (of low quality) used
 *
 */
void ImpulseAvg::update(entry & curr, entry & acc, double decay){

	if (curr.weight < acc.weight){ // or (curr.weight == acc.weight) && (decay == 1.0)
		const double w1 = (1.0-decay)*curr.weight;
		const double w2 = decay      *acc.weight;
		curr.weight = w1 + w2;
		curr.x =(w1*curr.x + w2*acc.x) / (curr.weight);
	}

	acc.set(curr.x, curr.weight);

}



void ImpulseAvg::addLeft(int i, double value, double weight){
	entry & curr = line[i].forward;
	curr.set(value, weight);
	update(curr, this->accumulated.forward, decays.horz.forward );
}

void ImpulseAvg::addRight(int i, double value, double weight){
	entry & curr = line[i].backward;
	curr.set(value, weight);
	update(curr, this->accumulated.backward, decays.horz.backward);
}

void ImpulseAvg::addDown(int i, double value, double weight){
	entry & curr = line[i].forward;
	curr.set(value, weight);
	update(curr, this->accumulated.forward, decays.vert.forward);
}

void ImpulseAvg::addUp(int i, double value, double weight){
	// if (i & 64)
	entry & curr = line[i].backward;
	curr.set(value, weight);
	update(curr, this->accumulated.backward, decays.vert.backward);
}



double ImpulseAvg::getWeight(int i){  // TODO const
	const entryPair & d = line[i];
	return (d.forward.weight + d.backward.weight) / 2.0;
}

double ImpulseAvg::get(int i){ // TODO const
	const entryPair & d = line[i];
	double w = d.forward.weight + d.backward.weight;
	if (w > 0.0)
		return ((d.forward.weight*d.forward.x + d.backward.weight*d.backward.x) / w);
	else
		return 0.0; // or code?
}



} // image::

} // drain::

