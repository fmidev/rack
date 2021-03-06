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
#include "File.h"
#include "DistanceModel.h"

namespace drain {

namespace image {

const float DistanceModel::nan_f = std::numeric_limits<float>::quiet_NaN();

void DistanceModel::createChain(DistanceNeighbourhood & chain, topol_t topology, bool forward) const {

	chain.clear();

	switch (topology) {
	case PIX_ADJACENCY_KNIGHT:
		chain.push_back(getElement(-1, -2, forward));
		chain.push_back(getElement(+1, -2, forward));
		chain.push_back(getElement(-2, -1, forward));
		chain.push_back(getElement(+2, -1, forward));
		// no break
	case PIX_ADJACENCY_8:
		// 8-adjacency
		chain.push_back(getElement(-1, -1, forward));
		chain.push_back(getElement(+1, -1, forward));
		// no break
	case PIX_ADJACENCY_4:
		// 4-adjacency
		chain.push_back(getElement(-1,  0, forward));
		chain.push_back(getElement( 0, -1, forward));
		break;
	default:
		break;
	}

}



}
}


// Drain
