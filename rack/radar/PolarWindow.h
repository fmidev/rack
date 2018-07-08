/*

    Copyright 2001 - 2014  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */
#ifndef RADAR_POLAR_WINDOW_H
#define RADAR_POLAR_WINDOW_H


#include <sstream>

//#include <drain/util/BeanLike.h>
#include <drain/image/GaussianWindow.h>


#include "data/PolarODIM.h"

// // using namespace std;

namespace rack {

class GaussianStripeVertPolarWeighted : public drain::image::GaussianStripeWeighted<false> { // true=horizontal, false=vertical
  public: // repl 


	GaussianStripeVertPolarWeighted(double radius=1.0, int height = 0) : GaussianStripeWeighted<false>(height, radius), rangeNorm(10), rangeNormEnd(0) { // RANGE(0),
		this->resetAtEdges = true;
	};

	//mutable	int RANGE;


	/*
	virtual
	bool reset(){

		//std::cerr << __FUNCTION__ << this->location << '\n';
		//drain::image::GaussianStripe::reset();
		//this->fill();

		// Consider: {NEAR,LINEAR,FAR}
		if (location.x < rangeNorm){
			RANGE = -1;
		}
		else if (location.x > (rangeNorm*this->jMax)){
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
		rangeNorm = i;
	}
	*/

	inline
	void setRangeNorm(const PolarODIM & odim){
		//rangeNorm = i;
		drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
		rangeNorm = static_cast<double>(odim.nrays) / (2.0*M_PI);
		//rangeNorm = static_cast<double>(this->conf.odimSrc.rscale * this->conf.odimSrc.nrays) / (2.0*M_PI);
		rangeNormEnd = (rangeNorm * this->conf.height);
		//mout.warn() << rangeNorm << '-' << rangeNormEnd << mout.endl;
	}



	/// Range index, at which aspect ratio of along-beam and cross-beam distances is unity
	inline
	int getRangeNorm(){
		return rangeNorm;
	}

protected:

	/// Range index, at which aspect ratio of along-beam and cross-beam distances is unity
	int rangeNorm;
	//int rangeNorm;
	int rangeNormEnd;
};


}  // rack::


#endif /* RADAR_POLAR_WINDOW */
