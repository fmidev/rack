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

void DistanceModelLinear::setDecrement(float horz,float vert, bool diag, bool knight){

	if (horz < 0.0)
		horzDecrement = 1;

	if (vert < 0.0)
		vertDecrement = horzDecrement;

	if (diag){
		diagDecrement  = sqrt(horzDecrement*horzDecrement + vertDecrement*vertDecrement) ;
		KNIGHT=false;
		if (knight){
			KNIGHT = true;
			knightDecrementHorz = sqrt(2.0*horzDecrement*horzDecrement +     vertDecrement*vertDecrement);   //diagDecrement * sqrt(5.0)/sqrt(2.0);
			knightDecrementVert = sqrt(    horzDecrement*horzDecrement + 2.0*vertDecrement*vertDecrement);
		}
	}


}

void DistanceModelLinear::setRadius(float horz, float vert, bool diag, bool knight){

	drain::MonitorSource mout("DistanceModelLinear", __FUNCTION__);

	mout.debug(3) << "radii: " << horz << ", " << vert << ", " << diag << mout.endl;

	if (horz <= 1.0)
		horzDecrement = max;
	else
		horzDecrement = static_cast<int>(max/horz  + 0.5);

	if (vert < 0.0)
		vertDecrement = horzDecrement;
	else if (vert == 0.0)
		vertDecrement = max;
	else // (vert > 0)
		vertDecrement = static_cast<int>(max/vert + 0.5);

	if (diag){
		diagDecrement = static_cast<int>(sqrt(horzDecrement*horzDecrement + vertDecrement*vertDecrement) + 0.5) ;
		KNIGHT = knight;
		if (KNIGHT){
			knightDecrementHorz = sqrt(4.0*horzDecrement*horzDecrement +     vertDecrement*vertDecrement);   //diagDecrement * sqrt(5.0)/sqrt(2.0);
			knightDecrementVert = sqrt(    horzDecrement*horzDecrement + 4.0*vertDecrement*vertDecrement);
		}
	}
	else {
		KNIGHT=false;
		diagDecrement = max;
	}


	//drain::
	mout.debug(3) << "decs: " << (float)horzDecrement << ", " << (float)vertDecrement << ", " << (float)diagDecrement << mout.endl;

}




void DistanceModelExponential::setRadius(float horz, float vert, bool diag, bool knight){

	// NEW : nominators now 1.0 => 2.0 to match better with linear half-widths
	drain::MonitorSource mout("DistanceModelExponential", __FUNCTION__);

	// TODO: interpret handle 0 and -1 better
	if (horz <= 0.0)
		horzDecay = 0;
	//horz = 1;
	else
		//horzDecay = static_cast<int>(1024.0 * pow(0.5,1.0/horz));  // 0.5^(1/horz)
		horzDecay = pow(0.5, 2.0/horz);  // 0.5^(1/horz)

	if (vert < 0)
		vertDecay = horzDecay;
	else if (vert == 0)  // label => spread to infinity ??
		//vertDecay10 = 1024;
		vertDecay = 1.0;
	else
		vertDecay = pow(0.5, 2.0/vert);  // 0.5^(1/horz)
	//vertDecay10 = static_cast<int>(1024.0 * pow(0.5,1.0/vert));  // 0.5^(1/horz)

	if (diag){
		//if ((horzDecay > 0) && (horzDecay > 0))
		const double hLog = log(horzDecay);
		const double vLog = log(vertDecay);
		diagDecay = exp(-sqrt(hLog*hLog + vLog*vLog));
		KNIGHT=knight;
		if (KNIGHT){
			knightDecayHorz = exp(-sqrt(4.0*hLog*hLog +     vLog*vLog));
			knightDecayVert = exp(-sqrt(    hLog*hLog + 4.0*vLog*vLog));
		}
	}
	else {
		KNIGHT=false;
		diagDecay = 1.0;
	}

	mout.debug(3) << "decays: " << horzDecay << ", " << vertDecay << ", " << diagDecay << mout.endl;
}


void DistanceModelExponential::setDecrement(float horz, float vert, bool diag, bool knight){

	drain::MonitorSource mout("DistanceModelExponential", __FUNCTION__);

	if (horz < 0.0){
		mout.error() << "'horz' less than zero." << mout.endl;
		//throw std::runtime_error();
	}
	else if (horz > 1.0) {
		mout.error() << "'horz' greater than 1.0." << mout.endl;
		// throw std::runtime_error("setDecrement: 'horz' greater than 1.0.");
	}
	else
		horzDecay = static_cast<int>(1024.4 * horz);  // 0.5^(1/horz)  ????


	if (vert == -1){
		vertDecay = horzDecay;
	}
	else if (vert < 0.0){
		mout.error() << "'vert' less than zero." << mout.endl;
		// throw std::runtime_error("setDecrement: 'vert' less than undetectValue.");
	}
	else if (vert > 1.0) {
		mout.error() << "'vert' greater than 1.0." << mout.endl;
		// throw std::runtime_error("setDecrement: 'vert' greater than 1.0.");
	}
	else
		vertDecay = vert;


	if (diag){
			//if ((horzDecay > 0) && (horzDecay > 0))
			const double hLog = log(horzDecay);
			const double vLog = log(vertDecay);
			diagDecay = exp(-sqrt(hLog*hLog + vLog*vLog));
			KNIGHT=knight;
			if (KNIGHT){
				knightDecayHorz = exp(-sqrt(2.0*hLog*hLog +     vLog*vLog));
				knightDecayVert = exp(-sqrt(    hLog*hLog + 2.0*vLog*vLog));
			}
		}
		else {
			KNIGHT=false;
			diagDecay = 1.0;
		}
	}




}
}


// Drain
