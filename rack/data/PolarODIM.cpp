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

#include <drain/util/Log.h>

#include "PolarODIM.h"

namespace rack {



void PolarODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	if (initialize & ROOT){
		reference("what:object", object = "PVOL");  // // or SCAN...
		reference("where:lon", lon = 0.0);
		reference("where:lat", lat = 0.0);
		reference("where:height", height = 0.0);
		reference("how:freeze", freeze = 10.0);
	}

	if (initialize & DATASET){
		reference("where:nbins",  nbins = 0L);
		reference("where:nrays",  nrays = 0L);
		reference("where:rscale", rscale = 0.0);
		reference("where:elangle", elangle = 0.0);
		reference("where:rstart", rstart = 0.0);
		reference("where:a1gate", a1gate = 0L);
		reference("where:startaz", startaz = 0.0);
		reference("where:stopaz",   stopaz = 0.0);
		reference("how:wavelength", wavelength = 0.0);
		reference("how:highprf", highprf = 0.0);
		reference("how:lowprf", lowprf = 0.0);
		// reference("how:NI", NI = 0);
	}

	if (initialize & DATA){
	}

}

void PolarODIM::update(const PolarODIM & odim){

	//if (NI == 0.0)
	odim.getNyquist(LOG_INFO);

	ODIM::update(odim);


}


double PolarODIM::getNyquist(int errorThreshold) const {

	drain::Logger mout("PolarODIM", __FUNCTION__);

	if (quantity.substr(0,4) != "VRAD"){
		//if (errorThreshold >= LOG_INFO)
		mout.log(errorThreshold + 4) << "quantity not VRAD but " << quantity << mout.endl;
		return NI;
	}

	if (NI == 0.0){

		NI = 0.01 * wavelength * lowprf / 4.0;
		if (NI != 0){
			mout.log(errorThreshold + 2) << "no NI in metadata, derived from wavelength*lowprf/4.0 " << mout.endl;
		}
		else {

			const std::type_info & t = drain::Type::getTypeInfo(type);

			if (drain::Type::call<drain::typeIsSmallInt>(t)){

				const double vMin = getMin(); // drain::Type::call<drain::typeMin, double>(t);
				const double vMax = getMax(); // drain::Type::call<drain::typeMax, double>(t);

				mout.log(errorThreshold + 1);
				mout << "no NI, wavelength or lowprf in metadata of elangle(" << elangle << "), guessed from type min/max: ";
				mout << "[" << vMin << ',' << vMax << "]"  << mout.endl;
				NI = vMax;

			}
			else {
				mout.log(errorThreshold) << " could not derive Nyquist speed (NI)" << mout.endl;
			}
		}

	}

	return NI;
}

/// Given two Doppler speeds (m/s), computes their difference (m/s).
/*
 *   - Assumes that the difference is within Nyquist domain [-NI,+NI].
 */
bool PolarODIM::deriveDifference(double v1, double v2, double & vDiff) const {

	// v - (2.0*vNyq)*floor((vNyq + v)/(2.0*vNyq));

	if (isValue(v1) && isValue(v2)){

		/// Raw value (m/s)
		vDiff = scaleForward(v2) - scaleForward(v1);

		if (vDiff < -NI)
			vDiff += (2.0*NI);
		else if (vDiff > NI)
			vDiff -= (2.0*NI);

		return true;
	}
	else
		return false;

}




}  // namespace rack



// Rack
