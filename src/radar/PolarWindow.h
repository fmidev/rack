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
#ifndef RADAR_POLAR_WINDOW_H
#define RADAR_POLAR_WINDOW_H


#include <sstream>

//#include "drain/util/BeanLike.h"
#include "drain/image/GaussianWindow.h"


#include "data/PolarODIM.h"

// // using namespace std;

namespace rack {

class GaussianStripeVertPolarWeighted : public drain::image::GaussianStripeWeighted<false> { // true=horizontal, false=vertical

public:

	GaussianStripeVertPolarWeighted(double radius=1.0, int height = 0) : drain::image::GaussianStripeWeighted<false>(height, radius), normRange(10,0) {
		this->resetAtEdges = true;
	};

	/*
	GaussianStripeVertPolarWeighted(double radius=1.0, int height = 0) : drain::image::GaussianStripeWeighted<false>(height, radius), normRange.min(10), rangeNormEnd(0) { // RANGE(0),
		this->resetAtEdges = true;
	};
	*/

	//mutable	int RANGE;


	/*
	virtual
	bool reset(){

		//std::cerr << __FUNCTION__ << this->location << '\n';
		//drain::image::GaussianStripe::reset();
		//this->fill();

		// Consider: {NEAR,LINEAR,FAR}
		if (location.x < normRange.min){
			RANGE = -1;
		}
		else if (location.x > (normRange.min*this->jMax)){
			RANGE = +1;
		}
		else {
			RANGE = 0;
		}
		return true;
	}
	*/


	void update();

	/// Range index, at which aspect ratio of along-beam and cross-beam distances is unity
	/*
	inline
	void setRangeNorm(int i){
		normRange.min = i;
	}
	*/

	inline
	void setRangeNorm(const PolarODIM & odim){
		//normRange.min = i;
		drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
		normRange.min = static_cast<double>(odim.area.height) / (2.0*M_PI);
		//normRange.min = static_cast<double>(this->conf.odimSrc.rscale * this->conf.odimSrc.geometry.height) / (2.0*M_PI);
		normRange.max = (normRange.min * this->conf.frame.height);
		//mout.warn(rangeNorm , '-' , rangeNormEnd );
	}



	/// Range index, at which aspect ratio of along-beam and cross-beam distances is unity
	inline
	int getRangeNorm(){
		return normRange.min;
	}

protected:

	drain::Range<int> normRange;
	/// Range index, at which aspect ratio of along-beam and cross-beam distances is unity
	//int rangeNorm;
	//int rangeNorm;
	//int rangeNormEnd;
};


}  // rack::


#endif /* RADAR_POLAR_WINDOW */

// Rack
