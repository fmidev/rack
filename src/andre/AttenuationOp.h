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

#ifndef Attenuation_OP_H_
#define Attenuation_OP_H_

#include "DetectorOp.h"


namespace rack {

/// Computes attenuation caused by \em precipitation.
/*!
 *
 *  By Ulrich Blahak (and Isztar Zawadski, Heikki Pohjola, Jarmo Koistinen),
 *  for liquid precipitation (including bright band):
 *  \f[
 *      k(Z) = 1.12\cdot10^{-4} Z_e^{0.62}
 *  \f]
 *  and for snow:
 *  \f[
 *      k(Z) = 1.1\cdot10^{-7} Z_e + 2.1\cdot10^{-5} Z_e^{0.5}
 *  \f]
 *
 * Hence, the general expression for precipitation attenuation is
 * \f[
 *      k(Z) = c Z_e^p + c_2 Z_e^{p_2}.
 * \f]
 *
 */
class AttenuationOp: public DetectorOp {

public:

	/**
	 * \param prob - maximum expected reflectance of biometeors
	 * \param maxAltitude - maximum expected altitude of biometeors
	 * \param reflDev - fuzzy width of  \c reflMax
	 * \param devAltitude - fuzzy width of \c maxAltitude
	 */
	AttenuationOp() :
		DetectorOp(__FUNCTION__, "Computes attenuation and converts it to probability", "distortion.attenuation"){ //ECHO_CLASS_PRECIP){

		parameters.link("reflHalfWidth", this->reflHalfWidth=3.0, "dBZ limit of 50% quality");

		// todo: "rain", "snow"
		parameters.link("c", this->c=1.12E-7, "coeff");
		parameters.link("p", this->p=0.62, "coeff");

		parameters.link("c2", this->c2=0, "coeff");
		parameters.link("p2", this->p2=0, "coeff");


		UNIVERSAL = true;
		//dataSelector.setQuantityRegExp("^DBZH$");
		dataSelector.setQuantities("DBZH:DBZ");
		// dataSelector.setMaxCount(1); NEW 2025
		REQUIRE_STANDARD_DATA = false;


	};

	/*
	virtual inline
	void set Parameters(const std::string & params, char assignmentSymbol='=', char separatorSymbol=0){

		if (params == "rain"){
			c = 1.12E-7;
			p = 0.62;
			c2 = 0;
			p2 = 0;
		}
		else if (params == "snow"){
			c = 1.1E-7;
			p = 1;
			c2 = 2.1E-5;
			p2 = 0.5;
		}
		else {
			BeanLike::setParameters(params, assignmentSymbol, separatorSymbol);
		}

	};
	*/

	double reflHalfWidth;
	double p;
	double c;
	double p2;
	double c2;

protected:

	virtual
	void runDetector(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const;

};




}

#endif /* BIOMET_OP_H_ */

// Rack
