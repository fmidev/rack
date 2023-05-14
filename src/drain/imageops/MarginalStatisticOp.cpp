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
#include "MarginalStatisticOp.h"


namespace drain
{

namespace image
{


void MarginalStatisticOp::process(const ImageFrame &src, const ImageFrame &weight, Image &dst) const {
	//	int &i, const int &iCount, int &n, const int &nCount){
	drain::Logger mout(getImgLog(),"MarginalStatisticOp::filterImage");

	Histogram histogram(256);

	const bool HORIZONTAL = mode[0] == 'h';

	const bool WEIGHTED = !weight.isEmpty();

	//const std::string &stat = parameters.get("stat", "a");
	const unsigned int statSize = stat.size();

	//const double medianPos = parameters.get("medianPos", 0.50);

	size_t i, j;

	size_t & m = HORIZONTAL ? i : j;
	size_t & n = HORIZONTAL ? j : i;
	const size_t & mLimit = HORIZONTAL ? src.getWidth() : src.getHeight();
	const size_t & nLimit = HORIZONTAL ? src.getHeight(): src.getWidth() ;

	const size_t iChannels = src.getImageChannelCount();
	const size_t aChannels = src.getAlphaChannelCount();
	const size_t channels = iChannels + aChannels;

	if (HORIZONTAL)
		dst.setGeometry(statSize, nLimit, iChannels, aChannels);
	else
		dst.setGeometry(nLimit, statSize, iChannels, aChannels);

	histogram.setSampleCount(mLimit);
	histogram.setMedianPosition(medianPos);

	size_t s;
	size_t & di = HORIZONTAL ? s : i;
	size_t & dj = HORIZONTAL ? j : s;

	if (WEIGHTED){
		if (weight.getGeometry() != src.getGeometry())
			mout.error() << "MarginalStatisticOp::geom mismatch" << mout.endl;

		//Point2D<int> p;
		//CoordinateHandler2D handler(weight.getWidth(), weight.getHeight());

		for (size_t k=0; k<channels; k++){

			/// OUTER LOOP (j or i)
			for (n=0; n<nLimit; n++){
				histogram.clearBins();

				// INNER LOOP (i or j)
				for (m=0; m<mLimit; m++)

					//p.setLocation(i,j); // memory bug
					//std::cerr << "p=" << p << "\t=>";
					//handler.handle(p);  // memory bug
					//std::cerr << p << '\n';

					//histogram.increment(src.get<int>(i,j));
					//histogram.increment(src.get<int>(i,j), 128);
					histogram.increment(src.get<int>(i,j), weight.get<int>(i,0));
					//histogram.increment(src.get<int>(i,j), weight.get<int>(p));

				// STORE RESULT
				for (s=0; s<statSize; s++){
					dst.put(di,dj,k, histogram.getValue(stat[s]));
				}
			}
		}
	}
	else {
		for (size_t k=0; k<channels; k++){

			/// OUTER LOOP (j or i)
			for (n=0; n<nLimit; n++){
				histogram.clearBins();

				// INNER LOOP (i or j)
				for (m=0; m<mLimit; m++)
					histogram.increment(src.get<int>(i,j));

				// STORE RESULT
				for (s=0; s<statSize; s++){
					dst.put(di,dj,k, histogram.getValue(stat[s]));
				}
			}
		}

	}

}

}
}


// Drain
