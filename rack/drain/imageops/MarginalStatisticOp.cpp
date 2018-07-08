/*

    Copyright 2001 - 2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
					dst.put(di,dj,k, histogram.get<int>(stat[s]));
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
					dst.put(di,dj,k, histogram.get<int>(stat[s]));
				}
			}
		}

	}

}

}
}

