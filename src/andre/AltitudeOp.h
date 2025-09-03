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

#ifndef AltitudeOP_H_
#define AltitudeOP_H_

#include "DetectorOp.h"


namespace rack {

/// Computes attenuation caused by \em precipitation.
/*!

 *
 */
class AltitudeOp: public DetectorOp {

public:

	/**
	 * \param prob - maximum expected reflectance of biometeors
	 * \param maxAltitude - maximum expected altitude of biometeors
	 * \param reflDev - fuzzy width of  \c reflMax
	 * \param devAltitude - fuzzy width of \c maxAltitude
	 */
	AltitudeOp() :
		DetectorOp(__FUNCTION__, "Weights altitude", "nonmet.biol") { //"distortion.altitude"){ //ECHO_CLASS_PRECIP){

		parameters.link("functor", this->functor = functor, " Functor");
		parameters.link("aboveSeaLevel", this->aboveSeaLevel, "Reference is sea level, not radar site");
		parameters.link("bias", this->bias, "offset added to altitude");

	};

	std::string functor;
	bool aboveSeaLevel = true;
	double bias = 0.0;

protected:

	virtual
	void runDetector(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const override;

}; // Rack

}

#endif /* AltitudeOP_H_ */

