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

#include <drain/Log.h>
#include <drain/Type.h>
#include "PolarODIM.h"

namespace rack {


int PolarODIM::defaultRange(250000); // metres

void PolarODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	if (initialize & ODIMPathElem::ROOT){
		link("what:object", object = "PVOL");  // // or SCAN...
		link("where:lon", lon = 0.0);
		link("where:lat", lat = 0.0);
		link("where:height", height = 0.0);
		link("how:freeze", freeze = 10.0);

		link("how:ACCnum", ACCnum = 0); // NEW
	}

	if (initialize & ODIMPathElem::DATASET){
		link("where:nbins",  area.width  = 0L); //  nb ins = 0L);
		link("where:nrays",  area.height = 0L); //  nr ays = 0L);
		//link("where:test",  test = 123L); //
		link("where:rscale", resolution.x); // rscale = 0.0);
		link("where:elangle", elangle = 0.0);
		link("where:rstart", rstart = 0.0);
		link("where:a1gate", a1gate = 0L);
		link("where:startaz", startaz = 0.0);
		link("where:stopaz",   stopaz = 0.0);
		link("how:wavelength", wavelength = 0.0);
		link("how:highprf", highprf = 0.0);
		link("how:lowprf", lowprf = 0.0);
		// link("how:NI", NI = 0);
		//std::cerr << drain::Type::getTypeChar((*this)["where:jimpo"].getType()) << '\n';
	}

	if (initialize & ODIMPathElem::DATA){
	}

}

void PolarODIM::updateLenient(const PolarODIM & odim){

	drain::Logger mout(__FILE__, __FUNCTION__);

	ODIM::updateLenient(odim);

	if (rscale == 0.0){
		rscale = odim.rscale;
	}

	/*
	if (rstart == 0){
		rstart = odim.rstart;
	}
	*/

	odim.getNyquist(LOG_INFO);

	if ((lat == 0.0) && (lon == 0.0)){
		lat    = odim.lat;
		lon    = odim.lon;
		height = odim.height;
	}

	// ODIM::updateLenient(odim); noticed (?) 2024/03


}

double PolarODIM::getMaxRange(bool warn) const {

	//

	if (!warn)
		return rstart + rscale*static_cast<double>(area.width);
	else {
		drain::Logger mout("PolarODIM", __FUNCTION__);
		if (area.width == 0){
			mout.warn("nbins==0" );
		}
		if (rscale == 0){
			mout.warn("rscale==0" );
			// mout.warn("rscale==0, returning default range=" , PolarODIM::defaultRange , 'm' );
			// return 250000;
		}
		double r = rstart + rscale*static_cast<double>(area.width);
		if (r == 0.0){
			if (PolarODIM::defaultRange > 0){
				r = PolarODIM::defaultRange;
				mout.note("using defaultRange" , r );
			}
			else {
				r = 250000.0;
				mout.note("using range=" , r );
			}
		}
		return r;
	}

}


double PolarODIM::getNyquist(int errorThreshold) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (quantity.substr(0,4) != "VRAD"){
		mout.log(errorThreshold + 4) << "quantity not VRAD but " << quantity << mout.endl;
		return NI;
	}

	if (NI == 0.0){


		NI = 0.01 * wavelength * lowprf / 4.0;
		if (NI != 0){
			// check dual-prf
			if (highprf > lowprf){
				const double NI2 = 0.01 * wavelength * highprf / 4.0;
				mout.debug("dual-PDF detected, NI=" , NI , ", NI2=" , NI2 );
				// HOLLEMAN & BEEKHUIS 2002 Analysis and Correction of Dual PRF Velocity Data
				NI = NI*NI2 / (NI2-NI);
				mout.log(errorThreshold + 4) << "derived NI=" << NI << " from dual-PRF" << mout.endl;
			}
			else {
				mout.log(errorThreshold + 2) << "no NI in metadata, derived from wavelength*lowprf/4.0 " << mout.endl;
			}
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


// Detect Doppler speed aliasing (wrapping)
signed char PolarODIM::checkAliasing(double v1, double v2, double NI_threshold) const {

	if (isValue(v1) && isValue(v2)){ // cf. srcODIM.deriveDifference()
		v1 = scaleForward(v1);
		v2 = scaleForward(v2);
		if ((v1>+NI_threshold) && (v2<-NI_threshold)){
			return +1;
		}
		else if ((v1<-NI_threshold) && (v2>+NI_threshold)){
			return -1;
		}
	}

	return 0;

}


}  // namespace rack

namespace drain {
	DRAIN_TYPENAME_DEF(rack::PolarODIM);
}

