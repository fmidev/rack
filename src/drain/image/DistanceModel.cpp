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
#include <drain/image/ImageFile.h>
#include "DistanceModel.h"

namespace drain {

namespace image {

const float DistanceModel::nan_f = std::numeric_limits<float>::quiet_NaN();

// typedef drain::EnumFlagger<drain::SingleFlagger<PIXEL_ADJACENCY> > PixelAdjacencyFlagger;
// PixelAdjacencyFlagger pixelAdjacency;

template <>
const FlaggerDict EnumDict<DistanceModel::PIXEL_ADJACENCY>::dict = {
	{"4-CONNECTED",   drain::image::DistanceModel::CONN4},
	{"8-CONNECTED",   drain::image::DistanceModel::CONN8},
	{"16-CONNECTED",  drain::image::DistanceModel::KNIGHT}
};

void DistanceModel::update(){

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	pixelAdjacency.set(pixelAdjacencyStr);

	setRadius(horzRadius.forward, vertRadius.forward, horzRadius.backward, vertRadius.backward);

	updateBean();

}

void DistanceModel::updateBean() const {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);


	static
	const std::map<std::string,std::string> compatibilityMap = {
			{"0","4-CONNECTED"},
			{"1","8-CONNECTED"},
			{"2","16-CONNECTED"},
			{"KNIGHT","16-CONNECTED"}
	};

	for (const auto & entry: compatibilityMap){
		if (pixelAdjacencyStr == entry.first){
			mout.deprecating("use '", entry.second, "' instead of '", entry.first, "'");
			pixelAdjacencyStr = entry.second;
			break;
		}
	}

	pixelAdjacencyStr = pixelAdjacency.getDict().getKey(pixelAdjacency.getValue());
}

void DistanceModel::createChain(DistanceNeighbourhood & chain, PIXEL_ADJACENCY topology, bool forward) const {

	chain.clear();

	// pixelAdjacency.set(2);
	// pixelAdjacency.set

	switch (topology) {
	// element creation is dynamically by the model, hence initializer list not applicable
	case KNIGHT:
		chain.push_back(getElement(-1, -2, forward));
		chain.push_back(getElement(+1, -2, forward));
		chain.push_back(getElement(-2, -1, forward));
		chain.push_back(getElement(+2, -1, forward));
		// no break
	case CONN8:
		chain.push_back(getElement(-1, -1, forward));
		chain.push_back(getElement(+1, -1, forward));
		// no break
	case CONN4:
		chain.push_back(getElement(-1,  0, forward));
		chain.push_back(getElement( 0, -1, forward));
		break;
	default:
		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		mout.warn("illegal topology value: ", topology, " (", (unsigned int)topology, ")");
		break;
	}

}

/*
void DistanceModel::createChainOLD(DistanceNeighbourhood & chain, topol_t topology, bool forward) const {

	chain.clear();

	// pixelAdjacency.set(2);
	//pixelAdjacency.set

	switch (topology) {
	// element creation is dynamically by the model, hence initializer list not applicable
	case PIX_ADJACENCY_KNIGHT:
		chain.push_back(getElement(-1, -2, forward));
		chain.push_back(getElement(+1, -2, forward));
		chain.push_back(getElement(-2, -1, forward));
		chain.push_back(getElement(+2, -1, forward));
		// no break
	case PIX_ADJACENCY_8:
		chain.push_back(getElement(-1, -1, forward));
		chain.push_back(getElement(+1, -1, forward));
		// no break
	case PIX_ADJACENCY_4:
		chain.push_back(getElement(-1,  0, forward));
		chain.push_back(getElement( 0, -1, forward));
		break;
	default:
		break;
	}

}
*/


}
}


// Drain
